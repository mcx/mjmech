// Copyright 2016-2019 Josh Pieper, jjp@pobox.com.  All rights reserved.
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

#pragma once

#include "mjlib/io/virtual_deadline_timer.h"

namespace mjmech {
namespace base {

inline boost::posix_time::ptime Now(boost::asio::io_service& service) {
  return boost::asio::use_service<
    mjlib::io::VirtualDeadlineTimerServiceHolder>(service).now();
}

}
}
