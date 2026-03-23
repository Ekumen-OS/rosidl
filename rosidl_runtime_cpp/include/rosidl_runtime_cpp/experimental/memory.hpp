// Copyright 2026 Ekumen, Inc.
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

#ifndef ROSIDL_RUNTIME_CPP__EXPERIMENTAL__MEMORY_HPP_
#define ROSIDL_RUNTIME_CPP__EXPERIMENTAL__MEMORY_HPP_

#include "rosidl_runtime_c/experimental/memory.h"

namespace rosidl_runtime_cpp
{

/// @file
/// @brief Experimental C++ aliases for ROSIDL memory descriptors.

/// @brief Memory descriptor alias.
///
/// This aliases `rosidl_memory_t` from the C runtime API.
using Memory = rosidl_memory_t;

/// @brief Memory region descriptor alias.
///
/// This aliases `rosidl_memory_region_t` from the C runtime API.
using MemoryRegion = rosidl_memory_region_t;

}  // namespace rosidl_runtime_cpp

#endif  // ROSIDL_RUNTIME_CPP__EXPERIMENTAL__MEMORY_HPP_
