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
#include <memory_resource>
#include <stdexcept>
#include <vector>

#include "rosidl_runtime_cpp/experimental/sequence.hpp"

namespace
{

struct Widget
{
  Widget() = default;
  explicit Widget(int value_in)
  : value(value_in)
  {}

  int value{0};
};

}  // namespace

TEST(rosidl_runtime_cpp_experimental_sequence, pmr_growth_and_vector_conversion)
{
  std::byte buffer[256];
  std::pmr::monotonic_buffer_resource pool(buffer, sizeof(buffer));

  rosidl_runtime_cpp::BoundedSequence<int, 5> sequence(&pool);
  sequence.push_back(1);
  sequence.push_back(2);
  sequence.emplace_back(3);
  sequence.push_back(4);

  EXPECT_FALSE(sequence.using_fixed_storage());
  EXPECT_EQ(sequence.size(), 4u);
  EXPECT_EQ(sequence.front(), 1);
  EXPECT_EQ(sequence.back(), 4);

  std::vector<int> vector = sequence;
  EXPECT_EQ(vector, (std::vector<int>{1, 2, 3, 4}));
}

TEST(rosidl_runtime_cpp_experimental_sequence, upper_bound_is_enforced)
{
  std::byte buffer[256];
  std::pmr::monotonic_buffer_resource pool(buffer, sizeof(buffer));

  rosidl_runtime_cpp::BoundedSequence<int, 3> sequence(&pool);
  sequence.resize(3, 1);
  EXPECT_THROW(sequence.push_back(4), std::length_error);
  EXPECT_THROW(sequence.reserve(4), std::length_error);
}

TEST(rosidl_runtime_cpp_experimental_sequence, fixed_scalar_storage_capacity_is_enforced)
{
  int storage[3] = {0, 0, 0};
  rosidl_memory_region_t region;
  region.location.address = storage;
  region.location.attributes = 0;
  region.size = sizeof(storage);

  rosidl_runtime_cpp::Sequence<int> sequence(region);
  EXPECT_TRUE(sequence.using_fixed_storage());

  sequence.assign(3, 7);
  EXPECT_EQ(storage[0], 7);
  EXPECT_EQ(storage[2], 7);
  EXPECT_THROW(sequence.push_back(8), std::length_error);
}

TEST(rosidl_runtime_cpp_experimental_sequence, fixed_storage_capacity_uses_element_size)
{
  std::byte storage[3 * sizeof(int) + 1] = {};

  rosidl_memory_region_t region;
  region.location.address = storage;
  region.location.attributes = 0;
  region.size = sizeof(storage);

  rosidl_runtime_cpp::Sequence<int> sequence(region);
  sequence.assign(3, 9);
  EXPECT_EQ(sequence.size(), 3u);
  EXPECT_THROW(sequence.push_back(10), std::length_error);
}

TEST(rosidl_runtime_cpp_experimental_sequence, non_scalar_fixed_storage)
{
  alignas(Widget) std::byte storage[2 * sizeof(Widget)] = {};

  rosidl_memory_region_t region;
  region.location.address = storage;
  region.location.attributes = 0;
  region.size = sizeof(storage);

  rosidl_runtime_cpp::Sequence<Widget> sequence(region);
  sequence.emplace_back(10);
  sequence.emplace_back(20);

  EXPECT_EQ(sequence.size(), 2u);
  EXPECT_EQ(sequence[0].value, 10);
  EXPECT_EQ(sequence[1].value, 20);
}

TEST(rosidl_runtime_cpp_experimental_sequence, insert_count_value)
{
  rosidl_runtime_cpp::Sequence<int> sequence;
  sequence = {1, 2, 3};

  sequence.insert(0, 2, 9);
  EXPECT_EQ(std::vector<int>(sequence), (std::vector<int>{9, 9, 1, 2, 3}));

  sequence.insert(sequence.size(), 1, 7);
  EXPECT_EQ(std::vector<int>(sequence), (std::vector<int>{9, 9, 1, 2, 3, 7}));

  sequence.insert(2, 1, 5);
  EXPECT_EQ(std::vector<int>(sequence), (std::vector<int>{9, 9, 5, 1, 2, 3, 7}));

  // Zero count is a no-op.
  sequence.insert(0, 0, 42);
  EXPECT_EQ(sequence.size(), 7u);

  EXPECT_THROW(sequence.insert(sequence.size() + 1, 1, 0), std::out_of_range);
}

TEST(rosidl_runtime_cpp_experimental_sequence, insert_range)
{
  rosidl_runtime_cpp::Sequence<int> sequence;
  sequence = {1, 2, 3};

  std::vector<int> middle{8, 9};
  sequence.insert(1, middle.begin(), middle.end());
  EXPECT_EQ(std::vector<int>(sequence), (std::vector<int>{1, 8, 9, 2, 3}));

  std::vector<int> front{0};
  sequence.insert(0, front.begin(), front.end());
  EXPECT_EQ(std::vector<int>(sequence), (std::vector<int>{0, 1, 8, 9, 2, 3}));

  std::vector<int> back{4, 5};
  sequence.insert(sequence.size(), back.begin(), back.end());
  EXPECT_EQ(std::vector<int>(sequence), (std::vector<int>{0, 1, 8, 9, 2, 3, 4, 5}));

  // Raw pointer range (zero-copy source path used by the bindings).
  int raw[2] = {6, 7};
  sequence.insert(sequence.size(), raw, raw + 2);
  EXPECT_EQ(std::vector<int>(sequence), (std::vector<int>{0, 1, 8, 9, 2, 3, 4, 5, 6, 7}));

  // Self-aliasing range insert keeps the source intact.
  sequence.insert(2, sequence.begin(), sequence.begin() + 2);
  EXPECT_EQ(std::vector<int>(sequence), (std::vector<int>{0, 1, 0, 1, 8, 9, 2, 3, 4, 5, 6, 7}));

  EXPECT_THROW(sequence.insert(sequence.size() + 1, raw, raw + 1), std::out_of_range);
}

TEST(rosidl_runtime_cpp_experimental_sequence, erase)
{
  rosidl_runtime_cpp::Sequence<int> sequence;
  sequence = {1, 2, 3, 4, 5};

  sequence.erase(0);
  EXPECT_EQ(std::vector<int>(sequence), (std::vector<int>{2, 3, 4, 5}));

  sequence.erase(sequence.size() - 1);
  EXPECT_EQ(std::vector<int>(sequence), (std::vector<int>{2, 3, 4}));

  sequence.erase(1, 2);
  EXPECT_EQ(std::vector<int>(sequence), (std::vector<int>{2}));

  // Count is clamped to the tail.
  sequence = {1, 2, 3};
  sequence.erase(1, 10);
  EXPECT_EQ(std::vector<int>(sequence), (std::vector<int>{1}));

  EXPECT_THROW(sequence.erase(sequence.size()), std::out_of_range);
}

TEST(rosidl_runtime_cpp_experimental_sequence, insert_erase_non_trivial)
{
  rosidl_runtime_cpp::Sequence<Widget> sequence;
  sequence.emplace_back(1);
  sequence.emplace_back(2);
  sequence.emplace_back(3);

  sequence.insert(1, 2, Widget(9));
  EXPECT_EQ(sequence.size(), 5u);
  EXPECT_EQ(sequence[0].value, 1);
  EXPECT_EQ(sequence[1].value, 9);
  EXPECT_EQ(sequence[2].value, 9);
  EXPECT_EQ(sequence[3].value, 2);
  EXPECT_EQ(sequence[4].value, 3);

  sequence.erase(1, 2);
  EXPECT_EQ(sequence.size(), 3u);
  EXPECT_EQ(sequence[0].value, 1);
  EXPECT_EQ(sequence[1].value, 2);
  EXPECT_EQ(sequence[2].value, 3);

  // Append with a non-trivial type: the new slots are uninitialized and must
  // be constructed, not copy-assigned.
  sequence.insert(sequence.size(), 2, Widget(9));
  EXPECT_EQ(sequence.size(), 5u);
  EXPECT_EQ(sequence[3].value, 9);
  EXPECT_EQ(sequence[4].value, 9);

  std::vector<Widget> tail{Widget(7), Widget(8)};
  sequence.insert(sequence.size(), tail.begin(), tail.end());
  EXPECT_EQ(sequence.size(), 7u);
  EXPECT_EQ(sequence[5].value, 7);
  EXPECT_EQ(sequence[6].value, 8);
}

TEST(rosidl_runtime_cpp_experimental_sequence, insert_enforces_upper_bound)
{
  rosidl_runtime_cpp::BoundedSequence<int, 3> sequence;
  sequence = {1, 2, 3};
  EXPECT_THROW(sequence.insert(1, 1, 9), std::length_error);
  EXPECT_THROW(sequence.insert(1, 2, 9), std::length_error);
}

TEST(rosidl_runtime_cpp_experimental_sequence, lexicographic_ordering)
{
  rosidl_runtime_cpp::Sequence<int> a;
  a = {1, 2, 3};
  rosidl_runtime_cpp::Sequence<int> b;
  b = {1, 2, 4};
  rosidl_runtime_cpp::Sequence<int> c;
  c = {1, 2, 3};
  EXPECT_TRUE(a < b);
  EXPECT_TRUE(a <= c);
  EXPECT_TRUE(b > a);
  EXPECT_TRUE(c >= a);
  rosidl_runtime_cpp::Sequence<int> longer;
  longer = {1, 2, 3, 0};
  EXPECT_TRUE(a < longer);
}
