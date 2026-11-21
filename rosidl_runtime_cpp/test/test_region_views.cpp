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

#include <gtest/gtest.h>

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "rosidl_runtime_cpp/views/array.hpp"
#include "rosidl_runtime_cpp/views/scalar.hpp"
#include "rosidl_runtime_cpp/views/sequence.hpp"
#include "rosidl_runtime_cpp/views/string.hpp"

TEST(region_views, scalar_and_array_alias_external_storage)
{
  int scalar_storage = 3;
  rosidl_memory_t scalar_memory{};
  scalar_memory.address = &scalar_storage;
  rosidl_runtime_cpp::views::Scalar<int> scalar(scalar_memory);
  scalar = 11;
  EXPECT_EQ(scalar_storage, 11);
  const rosidl_runtime_cpp::views::Scalar<int> const_scalar(scalar_memory);
  EXPECT_EQ(const_scalar.get(), 11);

  int array_storage[3] = {1, 2, 3};
  rosidl_memory_region_t region{
    {array_storage, 0},
    sizeof(array_storage)};
  rosidl_runtime_cpp::views::Array<int, 3> array{
    rosidl_runtime_cpp::MemoryRegion<int>(region)};
  array[1] = 20;
  EXPECT_EQ(array_storage[1], 20);
  const rosidl_runtime_cpp::views::Array<int, 3> const_array{
    rosidl_runtime_cpp::MemoryRegion<int>(region)};
  EXPECT_EQ(const_array[1], 20);

  rosidl_memory_region_t undersized{{nullptr, 0}, sizeof(int)};
  bool array_rejected = false;
  try {
    rosidl_runtime_cpp::views::Array<int, 3> bad_array{
      rosidl_runtime_cpp::MemoryRegion<int>(undersized)};
    (void)bad_array;
  } catch (const std::invalid_argument &) {
    array_rejected = true;
  }
  EXPECT_TRUE(array_rejected);
}

TEST(region_views, string_fixed_storage_and_bounds)
{
  char storage[4] = {};
  rosidl_memory_region_t region{{storage, 0}, sizeof(storage)};
  rosidl_runtime_cpp::views::String string{
    rosidl_runtime_cpp::MemoryRegion<char>(region)};
  string.assign("abc");
  EXPECT_STREQ(string.c_str(), "abc");

  char bounded_storage[4] = {};
  rosidl_memory_region_t bounded_region{{bounded_storage, 0}, sizeof(bounded_storage)};
  rosidl_runtime_cpp::views::BoundedString<3> bounded{
    rosidl_runtime_cpp::MemoryRegion<char>(bounded_region)};
  bounded.assign("abc");
  EXPECT_THROW(bounded.push_back('d'), std::length_error);
}

TEST(region_views, sequence_fixed_storage_and_bounds)
{
  struct NestedElement
  {
    int value{0};
  };

  alignas(NestedElement) std::byte nested_storage[
    2 * sizeof(NestedElement)] = {};
  rosidl_memory_region_t nested_region{
    {nested_storage, 0}, sizeof(nested_storage)};
  rosidl_runtime_cpp::views::Sequence<NestedElement> nested_sequence{
    rosidl_runtime_cpp::MemoryRegion<NestedElement>(nested_region)};
  nested_sequence.emplace_back(NestedElement{42});
  nested_sequence.emplace_back(NestedElement{99});
  EXPECT_EQ(nested_sequence[0].value, 42);
  EXPECT_EQ(nested_sequence[1].value, 99);
  nested_sequence[0].value = 7;
  EXPECT_EQ(
    reinterpret_cast<NestedElement *>(nested_storage)[0].value,
    7);

  int storage[3] = {};
  rosidl_memory_region_t region{{storage, 0}, sizeof(storage)};
  rosidl_runtime_cpp::views::Sequence<int> sequence{
    rosidl_runtime_cpp::MemoryRegion<int>(region)};
  sequence.assign(3, 7);
  EXPECT_EQ(storage[2], 7);
  EXPECT_THROW(sequence.push_back(8), std::length_error);

  rosidl_memory_region_t bad{{nullptr, 0}, 0};
  bool sequence_rejected = false;
  try {
    rosidl_runtime_cpp::views::Sequence<int> bad_sequence{
      rosidl_runtime_cpp::MemoryRegion<int>(bad)};
    (void)bad_sequence;
  } catch (const std::invalid_argument &) {
    sequence_rejected = true;
  }
  EXPECT_TRUE(sequence_rejected);
}
