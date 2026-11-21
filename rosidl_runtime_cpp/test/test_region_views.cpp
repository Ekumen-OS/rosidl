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
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <tuple>
#include <utility>
#include <vector>

#include "rosidl_buffer/buffer.hpp"
#include "rosidl_runtime_cpp/views/array.hpp"
#include "rosidl_runtime_cpp/views/scalar.hpp"
#include "rosidl_runtime_cpp/views/sequence.hpp"
#include "rosidl_runtime_cpp/views/string.hpp"
#include "rosidl_runtime_cpp/views/traits.hpp"

namespace
{

int invalid_array_lifetimes = 0;
int live_array_elements = 0;
int live_default_array_elements = 0;

struct ThrowingDefaultArrayElement
{
  ThrowingDefaultArrayElement()
  {
    if (live_default_array_elements == 1) {
      throw std::runtime_error("construction failed");
    }
    ++live_default_array_elements;
  }
  ~ThrowingDefaultArrayElement() {--live_default_array_elements;}
};

struct ArrayElement
{
  explicit ArrayElement(int value)
  : value(value), valid(true)
  {
    if (value == 2) {
      throw std::runtime_error("construction failed");
    }
    ++live_array_elements;
  }
  ArrayElement(ArrayElement && other) noexcept
  : value(other.value), valid(true)
  {
    if (!other.valid) {
      ++invalid_array_lifetimes;
    }
    ++live_array_elements;
  }
  ~ArrayElement()
  {
    if (!valid) {
      ++invalid_array_lifetimes;
      return;
    }
    --live_array_elements;
    valid = false;
  }
  int value;
  bool valid;
};

}  // namespace

TEST(region_views, piecewise_array_constructs_only_live_elements)
{
  invalid_array_lifetimes = 0;
  live_array_elements = 0;
  {
    rosidl_runtime_cpp::views::Array<ArrayElement, 2> elements(
      std::piecewise_construct, std::make_tuple(0), std::make_tuple(1));
    EXPECT_EQ(elements[1].value, 1);
    EXPECT_EQ(live_array_elements, 2);
  }
  EXPECT_EQ(live_array_elements, 0);
  EXPECT_EQ(invalid_array_lifetimes, 0);

  EXPECT_THROW(
    (rosidl_runtime_cpp::views::Array<ArrayElement, 2>(
      std::piecewise_construct, std::make_tuple(1), std::make_tuple(2))),
    std::runtime_error);
  EXPECT_EQ(live_array_elements, 0);
  EXPECT_EQ(invalid_array_lifetimes, 0);
}

TEST(region_views, array_failed_default_construction_destroys_constructed_elements)
{
  live_default_array_elements = 0;
  EXPECT_THROW(
    (rosidl_runtime_cpp::views::Array<ThrowingDefaultArrayElement, 2>{}),
    std::runtime_error);
  EXPECT_EQ(live_default_array_elements, 0);
}

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
  rosidl_runtime_cpp::views::Scalar<int> scalar_copy(scalar);
  scalar_copy = 12;
  EXPECT_EQ(scalar_storage, 11);
  rosidl_memory_t missing_scalar{};
  EXPECT_THROW(
    (rosidl_runtime_cpp::views::Scalar<int>{missing_scalar}), std::invalid_argument);

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
  auto array_copy = array;
  array_copy[1] = 21;
  EXPECT_EQ(array_storage[1], 20);

  rosidl_memory_region_t undersized{{array_storage, 0}, sizeof(int)};
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

TEST(region_views, nested_sequence_preserves_borrowed_element_storage)
{
  char text[4] = "one";
  {
    std::vector<rosidl_runtime_cpp::MemoryRegion<char>> regions{{text, sizeof(text)}};
    rosidl_runtime_cpp::views::Sequence<rosidl_runtime_cpp::views::String> sequence{
      std::move(regions), 1};
    sequence[0].assign("two");
  }
  EXPECT_STREQ(text, "two");
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

TEST(region_views, string_rejects_unterminated_region)
{
  char storage[] = {'a', 'b', 'c', 'd', '\0'};
  rosidl_runtime_cpp::MemoryRegion<char> region(storage, 4);
  EXPECT_THROW((void)rosidl_runtime_cpp::views::String{region}, std::invalid_argument);

  rosidl_runtime_cpp::MemoryRegion<char> terminated(storage, 5);
  rosidl_runtime_cpp::views::String valid{terminated};
  EXPECT_EQ(valid.size(), 4u);
  EXPECT_THROW(
    (rosidl_runtime_cpp::views::BoundedString<3>{terminated}), std::length_error);
}

TEST(region_views, constraints_expose_rolling_descriptor)
{
  struct View
  {
    struct Constraints
    {
      std::size_t limit{3};
    };
  };
  rosidl_runtime_cpp::views::MessageConstraints<View> constraints;
  const auto descriptor = constraints.to_rosidl_message_type_constraints();
  EXPECT_EQ(descriptor.type_specific, &constraints.type_specific);
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

TEST(region_views, sequence_borrows_buffer_cpu_storage)
{
  rosidl::Buffer<uint8_t> buffer(3, 5);
  {
    auto storage = buffer.span();
    rosidl_runtime_cpp::MemoryRegion<uint8_t> region(storage.data(), storage.size_bytes());
    rosidl_runtime_cpp::views::Sequence<uint8_t> view{region, storage.size()};
    view[1] = 9;
    EXPECT_EQ(buffer[1], 9);
    EXPECT_THROW(view.push_back(1), std::length_error);
  }
  EXPECT_EQ(buffer[1], 9);
}

TEST(region_views, sequence_rejects_invalid_prepopulated_size)
{
  int storage[2] = {};
  rosidl_runtime_cpp::MemoryRegion<int> region(storage, sizeof(storage));
  rosidl_runtime_cpp::views::Sequence<int> valid{region, 2};
  EXPECT_EQ(valid.size(), 2u);
  EXPECT_THROW(
    (rosidl_runtime_cpp::views::Sequence<int>{region, 3}), std::length_error);
  EXPECT_THROW(
    (rosidl_runtime_cpp::views::BasicSequence<int, 1>{region, 2}), std::length_error);

  struct NestedElement
  {
    struct ExternalStorage {int value;};
    NestedElement() = default;
    explicit NestedElement(const ExternalStorage & storage)
    : value(storage.value) {}
    int value{0};
  };
  std::vector<NestedElement::ExternalStorage> regions{{1}};
  rosidl_runtime_cpp::views::Sequence<NestedElement> nested{regions, 1};
  EXPECT_EQ(nested[0].value, 1);
  EXPECT_THROW(
    (rosidl_runtime_cpp::views::Sequence<NestedElement>{regions, 2}), std::length_error);
  regions.push_back({2});
  EXPECT_THROW(
    (rosidl_runtime_cpp::views::BasicSequence<NestedElement, 1>{regions, 2}),
    std::length_error);
  rosidl_runtime_cpp::views::BasicSequence<NestedElement, 1> bounded{regions};
  EXPECT_EQ(bounded.max_size(), 1u);
  EXPECT_THROW(bounded.reserve(2), std::length_error);
  EXPECT_FALSE((std::is_nothrow_move_assignable_v<
      rosidl_runtime_cpp::views::Sequence<NestedElement>>));

  rosidl_runtime_cpp::MemoryRegion<NestedElement> missing;
  EXPECT_THROW(
    (rosidl_runtime_cpp::views::Sequence<NestedElement>{missing, regions}),
    std::invalid_argument);

  alignas(NestedElement) std::byte target_storage[sizeof(NestedElement)];
  rosidl_runtime_cpp::MemoryRegion<NestedElement> target_region(
    target_storage, sizeof(target_storage));
  rosidl_runtime_cpp::views::Sequence<NestedElement> target{target_region, regions};
  rosidl_runtime_cpp::views::Sequence<NestedElement> source{regions, 2};
  EXPECT_THROW(target = std::move(source), std::length_error);
}
