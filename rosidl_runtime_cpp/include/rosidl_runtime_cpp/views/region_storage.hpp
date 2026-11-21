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

#ifndef ROSIDL_RUNTIME_CPP__VIEWS__REGION_STORAGE_HPP_
#define ROSIDL_RUNTIME_CPP__VIEWS__REGION_STORAGE_HPP_

#include <cstddef>
#include <optional>
#include <vector>

#include "rosidl_runtime_cpp/memory.hpp"

namespace rosidl_runtime_cpp::views
{

template<typename T>
struct StringStorage
{
  StringStorage() = default;
  StringStorage(MemoryRegion<T> value)  // NOLINT(runtime/explicit)
  : region(value) {}
  StringStorage(T * data, std::size_t bytes)
  : region(data, bytes) {}

  MemoryRegion<T> region{};
  std::optional<std::size_t> size;
};

template<typename T>
struct PrimitiveSequenceStorage
{
  MemoryRegion<T> region{};
  std::size_t size{0};
};

template<typename T>
struct ElementSequenceStorage
{
  std::vector<T> elements;
  std::size_t size{0};
};

namespace detail
{

struct RegionViewAccess
{
  template<typename View, typename Message>
  static void assign(View & view, const Message & message)
  {
    view.assign_plain(message);
  }
};

}  // namespace detail
}  // namespace rosidl_runtime_cpp::views

#endif  // ROSIDL_RUNTIME_CPP__VIEWS__REGION_STORAGE_HPP_
