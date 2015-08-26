// Copyright 2015 Josh Pieper, jjp@pobox.com.  All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// TODO jpieper:
//  * Support calling Open multiple times.

#include "telemetry_log.h"

#include <fcntl.h>
#include <signal.h>
#include <time.h>

#include <cstdio>
#include <list>
#include <mutex>
#include <thread>

#include "circular_buffer.h"
#include "move_wrapper.h"

namespace legtool {

namespace {
const int64_t kBlockSize = (1 << 20);
const double kFlushTimeout_s = 1.0;
const size_t kBufferStartPadding = 16;
const int kTelemetryLogSignal = SIGRTMIN + 10;

struct Pipe {
  int read = 0;
  int write = 0;
};

int Errwrap(int value) {
  if (value < 0) {
    throw boost::system::system_error(
        boost::system::error_code(
            errno, boost::system::generic_category()));
  }
  return value;
}

struct SchemaRecord {
  std::string name;
  uint32_t identifier = 0;
  uint32_t block_schema_flags = 0;
  std::string schema;

  SchemaRecord(const std::string& name,
               uint32_t identifier,
               uint32_t block_schema_flags,
               const std::string& schema)
      : name(name),
        identifier(identifier),
        block_schema_flags(block_schema_flags),
        schema(schema) {}
  SchemaRecord() {}
};
}

static void timer_handler(int signum, siginfo_t* si, void *);

class TelemetryLog::Impl {
 public:
  Impl(TelemetryLog* parent) : parent_(parent) {}
  void Write(std::unique_ptr<OStream> buffer);
  void PostOpen();

  TelemetryLog* const parent_;
  bool realtime_ = false;
  std::unique_ptr<ThreadWriter> writer_;

  std::map<std::string, uint32_t> identifier_map_;
  std::map<uint32_t, std::string> reverse_identifier_map_;

  uint32_t next_id_ = 1;

  std::mutex buffers_mutex_;
  std::vector<std::unique_ptr<OStream> > buffers_;

  std::map<uint32_t, SchemaRecord> schema_;
};

class TelemetryLog::ThreadWriter : boost::noncopyable {
 public:
  ThreadWriter(Impl* impl, const std::string& name, bool realtime)
      : ThreadWriter(impl, OpenName(name), realtime) {}

  ThreadWriter(Impl* impl, int fd, bool realtime)
      : ThreadWriter(impl, OpenFd(fd), realtime) {}

  ThreadWriter(Impl* impl, FILE* fd, bool realtime)
      : impl_(impl),
        parent_id_(std::this_thread::get_id()),
        realtime_(realtime),
        fd_(fd),
        pipe_(MakePipe()),
        thread_(std::bind(&ThreadWriter::Run, this)) {
    BOOST_ASSERT(fd >= 0);
  }

  ~ThreadWriter() {
    BOOST_ASSERT(std::this_thread::get_id() == parent_id_);
    {
      std::lock_guard<std::mutex> lock(command_mutex_);
      done_ = true;
    }
    SignalThread();

    thread_.join();
    WriteAll();

    ::fclose(fd_);
    ::close(pipe_.read);
    ::close(pipe_.write);
  }

  void Flush() {
    BOOST_ASSERT(std::this_thread::get_id() == parent_id_);
    {
      std::lock_guard<std::mutex> lock(command_mutex_);
      flush_ = true;
    }
    SignalThread();
  }

  void Write(std::unique_ptr<OStream> buffer) {
    BOOST_ASSERT(std::this_thread::get_id() == parent_id_);
    {
      std::lock_guard<std::mutex> lock(data_mutex_);
      data_.push_back(std::move(buffer));
    }
    SignalThread();
  }

  void DoTimer() {
    // This will be invoked from a signal handler, so it could be in a
    // random thread.
    timer_fired_ = true;
    SignalThread();
  }

 private:
  static FILE* OpenName(const std::string& name) {
    FILE* result = ::fopen(name.c_str(), "wb");
    if (result == nullptr) {
      throw boost::system::system_error(
          boost::system::error_code(
              errno, boost::system::generic_category()));
    }
    return result;
  }

  static Pipe MakePipe() {
    int fds[2] = {};
    Errwrap(::pipe(fds));

    // We make the writing side non-blocking.
    ::fcntl(fds[1], F_SETFL, O_NONBLOCK);

    Pipe result;
    result.read = fds[0];
    result.write = fds[1];
    return result;
  }

  static FILE* OpenFd(int fd) {
    FILE* result = ::fdopen(fd, "wb");
    if (result == nullptr) {
      throw boost::system::system_error(
          boost::system::error_code(
              errno, boost::system::generic_category()));
    }
    return result;
  }

  void SignalThread() {
    // This can be called from the parent thread, or a signal handler
    // (which could be in a random thread.

    char c = 0;
    while (true) {
      int err = ::write(pipe_.write, &c, 1);
      if (err == 1) { return; }
      if (errno == EAGAIN ||
          errno == EWOULDBLOCK) {
        // Yikes, I guess we are backed up.  Just return, because
        // apparently the receiver is plenty well signaled.
        return;
      }
      if (errno != EINTR) {
        // Hmmm, we might be inside of a POSIX signal handler.  Just
        // abort.
        ::raise(SIGABRT);
      }
    }
  }

  void Run() {
    BOOST_ASSERT(std::this_thread::get_id() == thread_.get_id());

    ::setvbuf(fd_, buf_, _IOFBF, sizeof(buf_));

    if (realtime_) {
      StartTimer();
    }

    size_t result = ::fwrite(TelemetryFormat::kHeader,
             ::strlen(TelemetryFormat::kHeader), 1, fd_);
    if (result == 0) {
      Errwrap(-1);
    }

    while (true) {
      char c = {};
      int err = ::read(pipe_.read, &c, 1);
      if (err < 0 && errno != EINTR) {
        Errwrap(-1);
      }

      {
        std::lock_guard<std::mutex> lock(command_mutex_);
        if (done_) { return; }
        if (flush_) { HandleFlush(); }
      }

      if (timer_fired_) {
        HandleTimer();
      }

      RunWork();
    }
  }

  void RunWork() {
    BOOST_ASSERT(std::this_thread::get_id() == thread_.get_id());

    while (true) {
      {
        std::lock_guard<std::mutex> lock(data_mutex_);
        if (data_.empty()) { return; }
      }

      // No one can remove things from data but us, so we don't need
      // to check again.
      StartWrite();
    }
  }

  void StartTimer() {
    BOOST_ASSERT(std::this_thread::get_id() == thread_.get_id());

    struct sigaction act;
    std::memset(&act, 0, sizeof(act));
    act.sa_sigaction = timer_handler;
    act.sa_flags = SA_SIGINFO;
    Errwrap(sigaction(kTelemetryLogSignal, &act, nullptr));

    struct sigevent sevp;
    std::memset(&sevp, 0, sizeof(sevp));
    sevp.sigev_notify = SIGEV_SIGNAL;
    sevp.sigev_value.sival_ptr = this;
    sevp.sigev_signo = kTelemetryLogSignal;

    Errwrap(::timer_create(CLOCK_MONOTONIC, &sevp, &timer_id_));

    struct itimerspec value;
    std::memset(&value, 0, sizeof(value));
    value.it_interval.tv_sec = static_cast<int>(kFlushTimeout_s);
    value.it_interval.tv_nsec =
        static_cast<long>((kFlushTimeout_s - value.it_interval.tv_sec) * 1e9);
    value.it_value = value.it_interval;
    Errwrap(::timer_settime(timer_id_, 0, &value, nullptr));
  }

  void HandleTimer() {
    BOOST_ASSERT(std::this_thread::get_id() == thread_.get_id());
    ::fflush(fd_);
  }

  void StartWrite() {
    BOOST_ASSERT(std::this_thread::get_id() == thread_.get_id());

    WriteFront();

    // At block boundaries, let the OS know that we don't plan on
    // using this data anytime soon so as to not fill up the page
    // cache.
    if (realtime_) {
      if ((child_offset_ - last_fadvise_) > kBlockSize) {
        ::fflush(fd_);
        int64_t next_fadvise = last_fadvise_;
        while ((next_fadvise + kBlockSize) < child_offset_) {
          next_fadvise += kBlockSize;
        }
        ::posix_fadvise(fileno(fd_),
                        last_fadvise_,
                        next_fadvise - last_fadvise_,
                        POSIX_FADV_DONTNEED);
        last_fadvise_ = next_fadvise;
      }
    }
  }

  void WriteAll() {
    BOOST_ASSERT(std::this_thread::get_id() == parent_id_);
    while (!data_.empty()) {
      WriteFront();
    }
  }

  void HandleFlush() {
    BOOST_ASSERT(std::this_thread::get_id() == thread_.get_id());
    ::fflush(fd_);
  }

  void WriteFront() {
    // NOTE: This can only be called by the parent thread during the
    // final write-out, after the child thread has stopped.

    std::unique_ptr<OStream> buffer;
    {
      std::lock_guard<std::mutex> lock(data_mutex_);
      buffer = std::move(data_.front());
      data_.pop_front();
    }

    const OStream& stream = *buffer;

    const char* ptr = &(*stream.data())[stream.start()];
    size_t size = stream.data()->size() - stream.start();
    size_t result = ::fwrite(ptr, size, 1, fd_);
    if (result == 0) {
      Errwrap(-1);
    }

    child_offset_ += size;
    {
      std::lock_guard<std::mutex> guard(impl_->buffers_mutex_);
      impl_->buffers_.push_back(std::move(buffer));
    }
  }

  // Parent items.
  Impl* const impl_;
  std::thread::id parent_id_;
  const bool realtime_;
  timer_t timer_id_ = {};

  // Initialized from parent, then only accessed from child.
  FILE* fd_;
  const Pipe pipe_;

  // Only accessed from child thread.
  int64_t child_offset_ = 0;
  int64_t last_fadvise_ = 0;
  char buf_[65536] = {};

  // All threads.
  std::thread thread_;

  std::mutex data_mutex_;
  circular_buffer<std::unique_ptr<OStream> > data_;

  std::mutex command_mutex_;
  bool flush_ = false;
  bool done_ = false;

  bool timer_fired_ = false;
};

void TelemetryLog::Impl::Write(std::unique_ptr<OStream> buffer) {
  if (writer_) {
    writer_->Write(std::move(buffer));
  } else {
    std::lock_guard<std::mutex> lock(buffers_mutex_);
    buffers_.push_back(std::move(buffer));
  }
}

void TelemetryLog::Impl::PostOpen() {
  for (const auto& pair: schema_) {
    parent_->WriteSchema(pair.second.identifier,
                         pair.second.block_schema_flags,
                         pair.second.name,
                         pair.second.schema);
  }
}

void timer_handler(int signum, siginfo_t* si, void *) {
  auto writer =
      static_cast<TelemetryLog::ThreadWriter*>(si->si_value.sival_ptr);
  writer->DoTimer();
}

TelemetryLog::TelemetryLog() : impl_(new Impl(this)) {}
TelemetryLog::~TelemetryLog() {}

void TelemetryLog::SetRealtime(bool value) { impl_->realtime_ = value; }

void TelemetryLog::Open(const std::string& filename) {
  BOOST_ASSERT(!impl_->writer_);
  impl_->writer_.reset(
      new ThreadWriter(impl_.get(), filename, impl_->realtime_));
  impl_->PostOpen();
}

void TelemetryLog::Open(int fd) {
  BOOST_ASSERT(!impl_->writer_);
  impl_->writer_.reset(
      new ThreadWriter(impl_.get(), fd, impl_->realtime_));
  impl_->PostOpen();
}

bool TelemetryLog::IsOpen() const {
  return !!impl_->writer_;
}

void TelemetryLog::Close() {
  impl_->writer_.reset();
}

void TelemetryLog::Flush() {
  if (impl_->writer_) {
    impl_->writer_->Flush();
  }
}

uint32_t TelemetryLog::AllocateIdentifier(const std::string& record_name) {
  auto it = impl_->identifier_map_.find(record_name);
  if (it != impl_->identifier_map_.end()) { return it->second; }

  // Find one that isn't used yet.
  while (impl_->reverse_identifier_map_.count(impl_->next_id_)) {
    impl_->next_id_++;
  }

  uint32_t result = impl_->next_id_;
  impl_->next_id_ ++;

  impl_->identifier_map_[record_name] = result;
  impl_->reverse_identifier_map_[result] = record_name;

  return result;
}

void TelemetryLog::WriteSchema(uint32_t identifier,
                               uint32_t block_schema_flags,
                               const std::string& record_name,
                               const std::string& schema) {
  auto it = impl_->identifier_map_.find(record_name);
  if (it != impl_->identifier_map_.end()) {
    if (identifier != it->second) {
      throw std::runtime_error(
          (boost::format(
              "Attempt to write schema for '%s' with identifier %d "
              "but already allocated as %d") %
           record_name % identifier % it->second).str());
    }
  } else {
    auto rit = impl_->reverse_identifier_map_.find(identifier);
    if (rit != impl_->reverse_identifier_map_.end()) {
      throw std::runtime_error(
          (boost::format(
              "Attempt to write schema for '%s' but identifier %d "
              "already used for '%s'") %
           record_name % identifier % it->second).str());
    } else {
      // Guess we might as well mark this identifier as being used.
      impl_->identifier_map_[record_name] = identifier;
      impl_->reverse_identifier_map_[identifier] = record_name;
    }
  }

  impl_->schema_[identifier] = SchemaRecord(
      record_name, identifier, block_schema_flags, schema);

  FastOStringStream ostr_schema;
  TelemetryWriteStream<FastOStringStream> stream_schema(ostr_schema);
  stream_schema.Write(block_schema_flags);
  stream_schema.Write(record_name);
  stream_schema.RawWrite(schema.data(), schema.size());

  auto buffer = GetBuffer();

  TelemetryWriteStream<OStream> stream(*buffer);
  stream.Write(static_cast<uint16_t>(TelemetryFormat::BlockType::kBlockSchema));
  stream.Write(static_cast<uint32_t>(ostr_schema.str().size()));
  stream.RawWrite(ostr_schema.str().data(), ostr_schema.str().size());

  impl_->Write(std::move(buffer));
}

void TelemetryLog::WriteData(uint32_t identifier,
                             uint32_t block_data_flags,
                             const std::string& serialized_data) {
  auto buffer = GetBuffer();

  TelemetryWriteStream<OStream> stream(*buffer);
  stream.Write(static_cast<uint16_t>(TelemetryFormat::BlockType::kBlockData));
  stream.Write(static_cast<uint32_t>(serialized_data.size() + 4));
  stream.Write(block_data_flags);
  stream.RawWrite(serialized_data.data(), serialized_data.size());

  impl_->Write(std::move(buffer));
}

void TelemetryLog::WriteBlock(TelemetryFormat::BlockType block_type,
                              const std::string& data) {
  auto buffer = GetBuffer();

  TelemetryWriteStream<OStream> stream(*buffer);
  stream.Write(static_cast<uint16_t>(block_type));
  stream.Write(static_cast<uint32_t>(data.size()));
  stream.RawWrite(data.data(), data.size());

  impl_->Write(std::move(buffer));
}

std::unique_ptr<TelemetryLog::OStream> TelemetryLog::GetBuffer() {
  std::lock_guard<std::mutex> guard(impl_->buffers_mutex_);

  std::unique_ptr<OStream> result;
  if (impl_->buffers_.empty()) {
    result = std::unique_ptr<OStream>(new OStream());
  } else {
    result = std::move(impl_->buffers_.back());
    impl_->buffers_.pop_back();
  }
  result->data()->resize(kBufferStartPadding);
  result->set_start(kBufferStartPadding);
  return result;
}

namespace {
class FakeStream : boost::noncopyable {
 public:
  FakeStream(char* data) : data_(data) {}

  void write(const char* data, size_t size) {
    std::memcpy(&data_[offset_], data, size);
    offset_ += size;
  }

 private:
  char* const data_;
  size_t offset_ = 0;
};
}

void TelemetryLog::WriteData(uint32_t identifier,
                             uint32_t block_data_flags,
                             std::unique_ptr<OStream> buffer) {
  const size_t kBlockHeaderSize = 2 + 4 + 4;
  BOOST_ASSERT(buffer->start() >= kBlockHeaderSize);

  size_t data_size = buffer->data()->size() - buffer->start();

  FakeStream stream(
      &(*buffer->data())[0] + buffer->start() - kBlockHeaderSize);
  TelemetryWriteStream<FakeStream> writer(stream);
  writer.Write(static_cast<uint16_t>(TelemetryFormat::BlockType::kBlockData));
  writer.Write(static_cast<uint32_t>(data_size + 4));
  writer.Write(block_data_flags);

  buffer->set_start(buffer->start() - kBlockHeaderSize);

  impl_->Write(std::move(buffer));
}

void TelemetryLog::WriteBlock(TelemetryFormat::BlockType block_type,
                              std::unique_ptr<OStream> buffer) {
  const size_t kBlockHeaderSize = 2 + 4;
  BOOST_ASSERT(buffer->start() >= kBlockHeaderSize);

  size_t data_size = buffer->data()->size() - buffer->start();

  FakeStream stream(
      &(*buffer->data())[0] + buffer->start() - kBlockHeaderSize);
  TelemetryWriteStream<FakeStream> writer(stream);
  writer.Write(static_cast<uint16_t>(block_type));
  writer.Write(static_cast<uint32_t>(data_size));
  buffer->set_start(buffer->start() - kBlockHeaderSize);

  impl_->Write(std::move(buffer));
}

}
