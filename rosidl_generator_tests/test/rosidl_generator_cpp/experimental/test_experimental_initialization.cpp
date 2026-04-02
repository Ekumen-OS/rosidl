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

#include <array>
#include <cstddef>
#include <cstring>
#include <memory_resource>

#include "rosidl_generator_tests/msg/experimental/arrays.hpp"
#include "rosidl_generator_tests/msg/experimental/basic_idl.hpp"
#include "rosidl_generator_tests/msg/experimental/basic_types.hpp"
#include "rosidl_generator_tests/msg/experimental/bounded_plain_sequences.hpp"
#include "rosidl_generator_tests/msg/experimental/bounded_sequences.hpp"
#include "rosidl_generator_tests/msg/experimental/constants.hpp"
#include "rosidl_generator_tests/msg/experimental/defaults.hpp"
#include "rosidl_generator_tests/msg/experimental/empty.hpp"
#include "rosidl_generator_tests/msg/experimental/multi_nested.hpp"
#include "rosidl_generator_tests/msg/experimental/nested.hpp"
#include "rosidl_generator_tests/msg/experimental/small_constant.hpp"
#include "rosidl_generator_tests/msg/experimental/strings.hpp"
#include "rosidl_generator_tests/msg/experimental/unbounded_sequences.hpp"
#include "rosidl_generator_tests/msg/experimental/w_strings.hpp"

namespace experimental = rosidl_generator_tests::msg::experimental;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

struct Arena
{
  alignas(std::max_align_t) std::array<std::byte, 65536> buf{};
  std::pmr::monotonic_buffer_resource res{buf.data(), buf.size()};
  std::pmr::memory_resource * get() {return &res;}
};

// ---------------------------------------------------------------------------
// Default construction — value initialisation for every message type
// ---------------------------------------------------------------------------

TEST(test_experimental_initialization_default, empty) {
  experimental::Empty a, b;
  EXPECT_EQ(a, b);
}

TEST(test_experimental_initialization_default, basic_types_all_zero) {
  experimental::BasicTypes msg;
  EXPECT_FALSE(msg.bool_value);
  EXPECT_EQ(0, msg.int32_value);
  EXPECT_EQ(0.0, msg.float64_value);
  EXPECT_EQ(0u, msg.uint64_value);
}

TEST(test_experimental_initialization_default, defaults_non_zero) {
  experimental::Defaults msg;
  EXPECT_TRUE(msg.bool_value);
  EXPECT_EQ(50, msg.byte_value);
  EXPECT_EQ(-30000, msg.int32_value);
}

TEST(test_experimental_initialization_default, constants_empty_struct) {
  experimental::Constants a, b;
  EXPECT_EQ(a, b);
}

TEST(test_experimental_initialization_default, strings_fields_empty) {
  experimental::Strings msg;
  EXPECT_TRUE(msg.string_value.empty());
  EXPECT_TRUE(msg.bounded_string_value.empty());
}

TEST(test_experimental_initialization_default, wstrings_fields_empty) {
  experimental::WStrings msg;
  EXPECT_TRUE(msg.wstring_value.empty());
  EXPECT_EQ(0u, msg.unbounded_sequence_of_wstrings.size());
}

TEST(test_experimental_initialization_default, nested_sub_message_zero) {
  experimental::Nested msg;
  EXPECT_EQ(0, msg.basic_types_value.int32_value);
}

TEST(test_experimental_initialization_default, arrays_scalar_zero_string_empty) {
  experimental::Arrays msg;
  EXPECT_EQ(0, msg.int32_values[0]);
  EXPECT_TRUE(msg.string_values[0].empty());
}

TEST(test_experimental_initialization_default, bounded_sequences_all_empty) {
  experimental::BoundedSequences msg;
  EXPECT_EQ(0u, msg.int32_values.size());
  EXPECT_EQ(0u, msg.string_values.size());
  EXPECT_EQ(0u, msg.basic_types_values.size());
}

TEST(test_experimental_initialization_default, bounded_plain_sequences_all_empty) {
  experimental::BoundedPlainSequences msg;
  EXPECT_EQ(0u, msg.int32_values.size());
  EXPECT_EQ(0u, msg.basic_types_values.size());
}

TEST(test_experimental_initialization_default, unbounded_sequences_all_empty) {
  experimental::UnboundedSequences msg;
  EXPECT_EQ(0u, msg.int32_values.size());
  EXPECT_EQ(0u, msg.string_values.size());
  EXPECT_EQ(0u, msg.basic_types_values.size());
}

TEST(test_experimental_initialization_default, multi_nested_sequences_empty) {
  experimental::MultiNested msg;
  EXPECT_EQ(0u, msg.bounded_sequence_of_arrays.size());
  EXPECT_EQ(0u, msg.unbounded_sequence_of_bounded_sequences.size());
}

// ---------------------------------------------------------------------------
// PMR constructor — resource propagation
// ---------------------------------------------------------------------------

TEST(test_experimental_initialization_pmr, basic_types_no_pmr_members_compiles) {
  // BasicTypes has no PMR members; constructor accepts mem_res without error.
  Arena arena;
  experimental::BasicTypes msg{arena.get()};
  EXPECT_FALSE(msg.bool_value);
  EXPECT_EQ(0, msg.int32_value);
}

TEST(test_experimental_initialization_pmr, nullptr_falls_back_to_default_resource) {
  experimental::BasicTypes msg{static_cast<std::pmr::memory_resource *>(nullptr)};
  EXPECT_EQ(0, msg.int32_value);
}

TEST(test_experimental_initialization_pmr, strings_pmr_string_is_usable) {
  Arena arena;
  experimental::Strings msg{arena.get()};
  // After PMR construction the string must be assignable and readable.
  msg.string_value = "pmr_test";
  EXPECT_EQ("pmr_test", msg.string_value);
  msg.bounded_string_value = "bounded_pmr";
  EXPECT_EQ("bounded_pmr", msg.bounded_string_value);
}

TEST(test_experimental_initialization_pmr, wstrings_pmr_wstring_is_usable) {
  Arena arena;
  experimental::WStrings msg{arena.get()};
  msg.wstring_value = u"\u30cf\u30ed\u30fc";
  EXPECT_EQ(u"\u30cf\u30ed\u30fc", msg.wstring_value);
  msg.unbounded_sequence_of_wstrings.push_back(rosidl_runtime_cpp::WString{u"a"});
  EXPECT_EQ(1u, msg.unbounded_sequence_of_wstrings.size());
}

TEST(test_experimental_initialization_pmr, nested_pmr_propagates_to_sub_message) {
  // Nested has no PMR members itself, but its sub-message might; verify it compiles.
  Arena arena;
  experimental::Nested msg{arena.get()};
  EXPECT_EQ(0, msg.basic_types_value.int32_value);
}

TEST(test_experimental_initialization_pmr, arrays_pmr_string_array_is_usable) {
  Arena arena;
  experimental::Arrays msg{arena.get()};
  msg.string_values[0] = "first";
  msg.string_values[1] = "second";
  EXPECT_EQ("first", msg.string_values[0]);
  EXPECT_EQ("second", msg.string_values[1]);
}

TEST(test_experimental_initialization_pmr, bounded_sequences_pmr_push_back_works) {
  Arena arena;
  experimental::BoundedSequences msg{arena.get()};
  msg.string_values.push_back(rosidl_runtime_cpp::String{"pmr"});
  ASSERT_EQ(1u, msg.string_values.size());
  EXPECT_EQ("pmr", msg.string_values[0]);
}

TEST(test_experimental_initialization_pmr, unbounded_sequences_pmr_push_back_works) {
  Arena arena;
  experimental::UnboundedSequences msg{arena.get()};
  msg.string_values.push_back(rosidl_runtime_cpp::String{"pmr_unbounded"});
  ASSERT_EQ(1u, msg.string_values.size());
  EXPECT_EQ("pmr_unbounded", msg.string_values[0]);
}

TEST(test_experimental_initialization_pmr, multi_nested_pmr_compiles_and_is_usable) {
  Arena arena;
  experimental::MultiNested msg{arena.get()};
  msg.array_of_arrays[0].string_values[0] = "deep";
  EXPECT_EQ("deep", msg.array_of_arrays[0].string_values[0]);
}

// ---------------------------------------------------------------------------
// Storage struct — default construction and equality
// ---------------------------------------------------------------------------

TEST(test_experimental_initialization_storage, basic_types_storage_equality) {
  experimental::BasicTypes::ExternalStorage s1, s2;
  EXPECT_EQ(s1, s2);
  EXPECT_FALSE(s1 != s2);
}

TEST(test_experimental_initialization_storage, defaults_storage_equality) {
  experimental::Defaults::ExternalStorage s1, s2;
  EXPECT_EQ(s1, s2);
}

TEST(test_experimental_initialization_storage, empty_storage_equality) {
  experimental::Empty::ExternalStorage s1, s2;
  EXPECT_EQ(s1, s2);
}

TEST(test_experimental_initialization_storage, strings_storage_equality) {
  experimental::Strings::ExternalStorage s1, s2;
  EXPECT_EQ(s1, s2);
}

TEST(test_experimental_initialization_storage, arrays_storage_equality) {
  experimental::Arrays::ExternalStorage s1, s2;
  EXPECT_EQ(s1, s2);
}

TEST(test_experimental_initialization_storage, nested_storage_equality) {
  experimental::Nested::ExternalStorage s1, s2;
  EXPECT_EQ(s1, s2);
}

TEST(test_experimental_initialization_storage, bounded_sequences_storage_equality) {
  experimental::BoundedSequences::ExternalStorage s1, s2;
  EXPECT_EQ(s1, s2);
}

TEST(test_experimental_initialization_storage, unbounded_sequences_storage_equality) {
  experimental::UnboundedSequences::ExternalStorage s1, s2;
  EXPECT_EQ(s1, s2);
}

TEST(test_experimental_initialization_storage, multi_nested_storage_equality) {
  experimental::MultiNested::ExternalStorage s1, s2;
  EXPECT_EQ(s1, s2);
}

// ---------------------------------------------------------------------------
// Storage constructor — initialise message from external memory descriptors
//
// For scalar fields the Storage holds a rosidl_runtime_cpp::Memory descriptor
// (address + attributes).  A default-constructed descriptor has address==nullptr;
// we must NOT dereference the resulting Scalar but the constructor must run.
// For real use a caller would set descriptor.address to a valid allocation.
// ---------------------------------------------------------------------------

TEST(test_experimental_initialization_storage_ctor, empty_storage_ctor_compiles) {
  experimental::Empty::ExternalStorage s;
  experimental::Empty msg{s};
  (void)msg;
}

TEST(test_experimental_initialization_storage_ctor, constants_storage_ctor_compiles) {
  experimental::Constants::ExternalStorage s;
  experimental::Constants msg{s};
  (void)msg;
}

TEST(test_experimental_initialization_storage_ctor, basic_types_storage_ctor_compiles) {
  // Null address — do not dereference, just verify construction doesn't crash.
  experimental::BasicTypes::ExternalStorage s;
  experimental::BasicTypes msg{s};
  (void)msg;
}

TEST(test_experimental_initialization_storage_ctor, basic_types_storage_ctor_with_real_memory) {
  // Provide a real backing buffer so we can safely read back through .
  alignas(int32_t) std::byte int32_buf[sizeof(int32_t)];
  std::memset(int32_buf, 0, sizeof(int32_buf));
  *reinterpret_cast<int32_t *>(int32_buf) = 1234;

  experimental::BasicTypes::ExternalStorage s;
  s.int32_value.assign(int32_buf);
  experimental::BasicTypes msg{s};
  EXPECT_EQ(1234, msg.int32_value);
}

TEST(test_experimental_initialization_storage_ctor, strings_storage_ctor_compiles) {
  experimental::Strings::ExternalStorage s;
  experimental::Strings msg{s};
  (void)msg;
}

TEST(test_experimental_initialization_storage_ctor, nested_storage_ctor_compiles) {
  experimental::Nested::ExternalStorage s;
  experimental::Nested msg{s};
  (void)msg;
}

TEST(test_experimental_initialization_storage_ctor, arrays_storage_ctor_compiles) {
  experimental::Arrays::ExternalStorage s;
  experimental::Arrays msg{s};
  (void)msg;
}

TEST(test_experimental_initialization_storage_ctor, bounded_sequences_storage_ctor_compiles) {
  experimental::BoundedSequences::ExternalStorage s;
  experimental::BoundedSequences msg{s};
  (void)msg;
}

TEST(test_experimental_initialization_storage_ctor, unbounded_sequences_storage_ctor_compiles) {
  experimental::UnboundedSequences::ExternalStorage s;
  experimental::UnboundedSequences msg{s};
  (void)msg;
}

TEST(test_experimental_initialization_storage_ctor, multi_nested_storage_ctor_compiles) {
  experimental::MultiNested::ExternalStorage s;
  experimental::MultiNested msg{s};
  (void)msg;
}
