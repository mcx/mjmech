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

#pragma once

#include "base/visitor.h"

#include "point3d.h"
#include "static_signal.h"

struct ImuData {
  uint32_t timestamp = {};
  int32_t error = 0;
  uint16_t rate_hz = 0;
  Point3D gyro_dps;
  Point3D accel_g;

  template <typename Archive>
  void Serialize(Archive* a) {
    a->Visit(MJ_NVP(timestamp));
    a->Visit(MJ_NVP(error));
    a->Visit(MJ_NVP(rate_hz));
    a->Visit(MJ_NVP(gyro_dps));
    a->Visit(MJ_NVP(accel_g));
  }
};

typedef StaticSignal<void (const ImuData*)> ImuDataSignal;
