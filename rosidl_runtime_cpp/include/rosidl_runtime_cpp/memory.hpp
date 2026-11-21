// Copyright 2026 Open Source Robotics Foundation, Inc.
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

#ifndef ROSIDL_RUNTIME_CPP__MEMORY_HPP_
#define ROSIDL_RUNTIME_CPP__MEMORY_HPP_

#include <cstddef>

#include "rosidl_runtime_c/memory.h"

namespace rosidl_runtime_cpp
{

template<typename T>
class MemoryRegion
{
public:
  using value_type = T;
  using pointer = T *;
  using const_pointer = const T *;
  using size_type = std::size_t;

  MemoryRegion() noexcept = default;

  MemoryRegion(rosidl_memory_region_t region) noexcept  // NOLINT(runtime/explicit)
  : region_(region)
  {}

  MemoryRegion(void * address, size_type size, int attributes = 0) noexcept
  : region_{{address, attributes}, size}
  {}

  pointer data() noexcept {return static_cast<pointer>(region_.location.address);}
  const_pointer data() const noexcept
  {
    return static_cast<const_pointer>(region_.location.address);
  }
  size_type size() const noexcept {return region_.size;}
  size_type capacity() const noexcept {return size() / sizeof(T);}
  int attributes() const noexcept {return region_.location.attributes;}
  const rosidl_memory_region_t & c_region() const noexcept {return region_;}

  explicit operator bool() const noexcept {return region_.location.address != nullptr;}

private:
  rosidl_memory_region_t region_{};
};

template<>
class MemoryRegion<void>
{
public:
  using size_type = std::size_t;

  MemoryRegion() noexcept = default;

  MemoryRegion(rosidl_memory_region_t region) noexcept  // NOLINT(runtime/explicit)
  : region_(region)
  {}

  MemoryRegion(void * address, size_type size, int attributes = 0) noexcept
  : region_{{address, attributes}, size}
  {}

  void * data() noexcept {return region_.location.address;}
  const void * data() const noexcept {return region_.location.address;}
  size_type size() const noexcept {return region_.size;}
  int attributes() const noexcept {return region_.location.attributes;}
  const rosidl_memory_region_t & c_region() const noexcept {return region_;}

  template<typename T>
  T * as() noexcept {return static_cast<T *>(data());}

  template<typename T>
  const T * as() const noexcept {return static_cast<const T *>(data());}

  explicit operator bool() const noexcept {return region_.location.address != nullptr;}

private:
  rosidl_memory_region_t region_{};
};

}  // namespace rosidl_runtime_cpp

#endif  // ROSIDL_RUNTIME_CPP__MEMORY_HPP_
