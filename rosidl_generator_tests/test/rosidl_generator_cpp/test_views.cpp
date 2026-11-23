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

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "rosidl_generator_tests/msg/basic_types.hpp"
#include "rosidl_generator_tests/msg/bounded_sequences.hpp"
#include "rosidl_generator_tests/msg/multi_nested.hpp"
#include "rosidl_generator_tests/msg/nested.hpp"
#include "rosidl_generator_tests/msg/strings.hpp"
#include "rosidl_generator_tests/msg/unbounded_sequences.hpp"
#include "rosidl_generator_tests/msg/view_constraint_cases.hpp"
#include "rosidl_generator_tests/msg/views/basic_types.hpp"
#include "rosidl_generator_tests/msg/views/basic_types__region.hpp"
#include "rosidl_generator_tests/msg/views/multi_nested.hpp"
#include "rosidl_generator_tests/msg/views/multi_nested__region.hpp"
#include "rosidl_generator_tests/msg/views/nested.hpp"
#include "rosidl_generator_tests/msg/views/nested__region.hpp"
#include "rosidl_generator_tests/msg/views/strings.hpp"
#include "rosidl_generator_tests/msg/views/unbounded_sequences.hpp"
#include "rosidl_generator_tests/msg/views/unbounded_sequences__region.hpp"
#include "rosidl_generator_tests/msg/views/view_constraint_cases.hpp"
#include "rosidl_generator_tests/msg/views/view_constraint_cases__region.hpp"
#include "rosidl_generator_tests/srv/views/basic_types.hpp"
#include "rosidl_generator_tests/srv/views/basic_types__region.hpp"

namespace msg = rosidl_generator_tests::msg;
namespace views = rosidl_generator_tests::msg::views;

TEST(test_views, service_request_and_response_views) {
  namespace service_views = rosidl_generator_tests::srv::views;
  rosidl_generator_tests::srv::BasicTypes_Request request;
  service_views::BasicTypes_Request borrowed(request);
  borrowed.int32_value(42);
  borrowed.string_value("request");
  EXPECT_EQ(request.int32_value, 42);
  EXPECT_EQ(request.string_value, "request");

  service_views::RegionBasicTypes_Response response;
  response.string_value().assign("response");
  response.int32_value(7);
  service_views::RegionBasicTypes_Response::Constraints constraints;
  constraints.string_value.size = 4;
  EXPECT_FALSE(constraints.CheckCompatibleView(response));
  constraints.string_value.size = 8;
  EXPECT_TRUE(constraints.CheckCompatibleView(response));
  EXPECT_EQ(response.int32_value(), 7);
  static_assert(std::is_const_v<std::remove_reference_t<
      decltype(std::declval<service_views::ConstRegionBasicTypes_Response &>().string_value())>>);
}

TEST(test_views, region_views_own_and_move_nested_fields) {
  views::RegionNested owned;
  EXPECT_TRUE(owned.owns_storage());
  owned.basic_types_value().int32_value(73);
  EXPECT_EQ(73, owned.basic_types_value().int32_value());

  views::RegionNested moved(std::move(owned));
  EXPECT_EQ(73, moved.basic_types_value().int32_value());
  EXPECT_THROW(owned.basic_types_value(), std::logic_error);
  static_assert(std::is_const_v<std::remove_reference_t<
      decltype(std::declval<views::ConstRegionNested &>().basic_types_value())>>);
  static_assert(std::is_same_v<
      decltype(std::declval<views::ConstRegionNested &>().basic_types_value().int32_value()),
      const int32_t &>);

  views::RegionMultiNested nested_containers;
  EXPECT_TRUE(nested_containers.owns_storage());

  views::RegionUnboundedSequences defaulted;
  msg::UnboundedSequences plain;
  ASSERT_EQ(defaulted.bool_values_default().size(), plain.bool_values_default.size());
  EXPECT_EQ(defaulted.bool_values_default()[1], plain.bool_values_default[1]);
  ASSERT_EQ(defaulted.string_values_default().size(), plain.string_values_default.size());
  EXPECT_EQ(defaulted.string_values_default()[1].view(), plain.string_values_default[1]);
}

TEST(test_views, region_views_copy_plain_messages) {
  msg::MultiNested plain;
  plain.bounded_sequence_of_bounded_sequences.resize(1);
  plain.bounded_sequence_of_bounded_sequences[0].string_values = {"first", "second"};
  views::RegionMultiNested view;
  view.assign(plain);
  EXPECT_EQ(view.bounded_sequence_of_bounded_sequences()[0].string_values()[1].view(), "second");
  view.bounded_sequence_of_bounded_sequences()[0].string_values()[1].assign("changed");
  auto copy = view.to_plain();
  EXPECT_EQ(copy.bounded_sequence_of_bounded_sequences[0].string_values[1], "changed");
  EXPECT_EQ(plain.bounded_sequence_of_bounded_sequences[0].string_values[1], "second");

  msg::UnboundedSequences sequences;
  sequences.bool_values = {true, false, true};
  sequences.string_values = {"hello", "world"};
  views::RegionUnboundedSequences sequence_view;
  sequence_view.assign(sequences);
  EXPECT_EQ(sequence_view.to_plain().bool_values, sequences.bool_values);
  EXPECT_EQ(sequence_view.to_plain().string_values, sequences.string_values);

  views::ConstRegionNested const_view;
  EXPECT_EQ(const_view.to_plain().basic_types_value.int32_value, 0);
  views::RegionMultiNested moved(std::move(view));
  EXPECT_THROW(view.to_plain(), std::logic_error);
  EXPECT_THROW(view.assign(plain), std::logic_error);
}

TEST(test_views, region_views_borrow_nested_scalar_storage) {
  msg::BasicTypes plain;
  views::NestedRegionStorage storage;
  storage.block = {&plain, sizeof(plain)};
  auto & fields = storage.members.basic_types_value.members;
  const auto bind = [](auto & value) {
      return rosidl_runtime_cpp::Memory<std::remove_reference_t<decltype(value)>>(&value);
    };
  fields.bool_value = bind(plain.bool_value);
  fields.byte_value = bind(plain.byte_value);
  fields.char_value = bind(plain.char_value);
  fields.float32_value = bind(plain.float32_value);
  fields.float64_value = bind(plain.float64_value);
  fields.int8_value = bind(plain.int8_value);
  fields.uint8_value = bind(plain.uint8_value);
  fields.int16_value = bind(plain.int16_value);
  fields.uint16_value = bind(plain.uint16_value);
  fields.int32_value = bind(plain.int32_value);
  fields.uint32_value = bind(plain.uint32_value);
  fields.int64_value = bind(plain.int64_value);
  fields.uint64_value = bind(plain.uint64_value);

  views::RegionNested view(storage);
  EXPECT_FALSE(view.owns_storage());
  EXPECT_EQ(view.backing_storage().data(), &plain);
  view.basic_types_value().int32_value(24);
  EXPECT_EQ(plain.int32_value, 24);
  EXPECT_EQ(view.backing_storage().size(), sizeof(plain));
  views::ConstRegionNested read_only(storage);
  EXPECT_EQ(read_only.basic_types_value().int32_value(), 24);
  EXPECT_EQ(read_only.to_plain().basic_types_value.int32_value, 24);
}

TEST(test_views, region_views_borrow_bounded_string_storage) {
  char value[4] = "one";
  char array0[4] = "a";
  char array1[4] = "b";
  char sequence0[4] = "c";
  char sequence1[4] = "d";
  views::ViewConstraintCasesRegionStorage storage;
  storage.members.bounded_value = {value, sizeof(value)};
  storage.members.bounded_array = {{{array0, sizeof(array0)}, {array1, sizeof(array1)}}};
  storage.members.bounded_sequence.elements = {{sequence0, sizeof(sequence0)}};
  storage.members.bounded_sequence.size = 1;
  storage.members.unbounded_sequence.elements = {{sequence1, sizeof(sequence1)}};
  storage.members.unbounded_sequence.size = 1;

  views::RegionViewConstraintCases borrowed(storage);
  EXPECT_FALSE(borrowed.owns_storage());
  borrowed.bounded_value().assign("two");
  borrowed.bounded_array()[1].assign("xyz");
  borrowed.bounded_sequence()[0].assign("hi");
  EXPECT_STREQ(value, "two");
  EXPECT_STREQ(array1, "xyz");
  EXPECT_STREQ(sequence0, "hi");
  EXPECT_THROW(borrowed.bounded_value().assign("long"), std::length_error);
  EXPECT_THROW(borrowed.bounded_sequence().resize(3), std::length_error);

  views::ConstRegionViewConstraintCases const_view(storage);
  static_assert(std::is_const_v<std::remove_reference_t<
      decltype(const_view.bounded_value())>>);
  EXPECT_EQ(const_view.bounded_value().view(), "two");

  storage.members.bounded_sequence.size = 2;
  EXPECT_THROW((views::RegionViewConstraintCases{storage}), std::length_error);
  storage.members.bounded_sequence.elements.clear();
  storage.members.bounded_sequence.size = 0;
  views::RegionViewConstraintCases empty_borrow(storage);
  EXPECT_THROW(empty_borrow.bounded_sequence().resize(1), std::length_error);
  char unterminated[4] = {'a', 'b', 'c', 'd'};
  storage.members.bounded_value = {unterminated, sizeof(unterminated)};
  EXPECT_THROW((views::RegionViewConstraintCases{storage}), std::invalid_argument);
}

TEST(test_views, region_views_validate_nested_constraints) {
  views::RegionMultiNested message;
  auto & item = message.bounded_sequence_of_bounded_sequences().emplace_back();
  item.string_values().emplace_back().assign("abcd");

  views::RegionMultiNested::Constraints limits;
  limits.bounded_sequence_of_bounded_sequences.size = 2;
  limits.bounded_sequence_of_bounded_sequences.element.string_values.size = 2;
  limits.bounded_sequence_of_bounded_sequences.element.string_values.element.size = 3;
  std::string path;
  const auto report = [&path](std::string_view field, int) {path = field;};
  EXPECT_FALSE(limits.CheckCompatibleView(message, report));
  EXPECT_EQ("bounded_sequence_of_bounded_sequences[0].string_values[0]", path);
  limits.bounded_sequence_of_bounded_sequences.element.string_values.element.size = 4;
  EXPECT_TRUE(limits.CheckCompatibleView(message));
}

TEST(test_views, bounded_constraints_reject_limits_above_idl_bounds) {
  views::RegionViewConstraintCases::Constraints limits;
  const views::RegionViewConstraintCases::Constraints baseline;
  std::string path;
  const auto report = [&path](std::string_view field, int) {path = field;};

  limits.bounded_value.size = 4;
  EXPECT_FALSE(limits.CheckCompatible(baseline, report));
  EXPECT_EQ("bounded_value", path);
  limits.bounded_value.size = 0;
  limits.bounded_array.size = 4;
  EXPECT_FALSE(limits.CheckCompatible(baseline, report));
  EXPECT_EQ("bounded_array", path);
  limits.bounded_array.size = 0;
  limits.bounded_sequence.size = 3;
  EXPECT_FALSE(limits.CheckCompatible(baseline, report));
  EXPECT_EQ("bounded_sequence", path);
  limits.bounded_sequence.size = 0;
  limits.unbounded_sequence.element.size = 4;
  EXPECT_FALSE(limits.CheckCompatible(baseline, report));
  EXPECT_EQ("unbounded_sequence", path);
  limits.unbounded_sequence.element.size = 3;
  EXPECT_TRUE(limits.CheckCompatible(baseline));
}

TEST(test_views, owning_and_borrowed_plain_message_adapters) {
  views::BasicTypes owned;
  EXPECT_TRUE(owned.owns_message());
  owned.int32_value(17);
  EXPECT_EQ(17, owned.int32_value());

  msg::BasicTypes plain;
  views::BasicTypes borrowed(plain);
  EXPECT_FALSE(borrowed.owns_message());
  borrowed.int32_value(42);
  EXPECT_EQ(42, plain.int32_value);

  const msg::BasicTypes const_plain{};
  views::ConstBasicTypes read_only(const_plain);
  EXPECT_FALSE(read_only.owns_message());
  static_assert(std::is_same_v<
      decltype(std::declval<views::ConstBasicTypes &>().int32_value()), const int32_t &>);
  static_assert(!std::is_assignable_v<
      decltype((std::declval<views::ConstBasicTypes &>().int32_value())), int32_t>);

  views::BasicTypes copied(const_plain);
  EXPECT_TRUE(copied.owns_message());
}

TEST(test_views, string_and_sequence_constraints_reject_looser_bounds) {
  rosidl_runtime_cpp::StringConstraint string_baseline{8};
  EXPECT_TRUE(rosidl_runtime_cpp::StringConstraint{5}.CheckCompatible(string_baseline));
  EXPECT_FALSE(rosidl_runtime_cpp::StringConstraint{9}.CheckCompatible(string_baseline));

  rosidl_runtime_cpp::NestedSequenceConstraint<rosidl_runtime_cpp::StringConstraint>
  sequence_baseline{4, {8}};
  std::string reported_path;
  auto report = [&reported_path](std::string_view path, int) {
      reported_path = path;
    };
  EXPECT_FALSE(
    (rosidl_runtime_cpp::NestedSequenceConstraint<rosidl_runtime_cpp::StringConstraint>{
    4, {9}}).CheckCompatible(sequence_baseline, report, "labels"));
  EXPECT_EQ("labels", reported_path);

  views::UnboundedSequencesView<>::Constraints sequence_constraints;
  views::UnboundedSequencesView<>::Constraints baseline;
  sequence_constraints.int32_values.size = 3;
  baseline.int32_values.size = 2;
  EXPECT_FALSE(sequence_constraints.CheckCompatible(baseline));

  views::StringsView<>::Constraints string_constraints;
  views::StringsView<>::Constraints string_limit;
  string_constraints.string_value.size = 4;
  string_limit.string_value.size = 6;
  EXPECT_TRUE(string_constraints.CheckCompatible(string_limit));

  msg::Strings strings;
  strings.string_value = "too long";
  string_constraints.string_value.size = 4;
  EXPECT_FALSE(string_constraints.CheckCompatible(strings));

  msg::UnboundedSequences sequences;
  sequences.string_values.push_back("longer than allowed");
  views::UnboundedSequencesView<>::Constraints sequence_limits;
  sequence_limits.string_values.element.size = 8;
  EXPECT_FALSE(sequence_limits.CheckCompatible(sequences));

  views::UnboundedSequences sequence_view;
  sequence_view.uint8_values().push_back(11);
  EXPECT_EQ(11, sequence_view.uint8_values().at(0));
}

TEST(test_views, nested_accessors_and_constraints_recurse) {
  views::Nested nested;
  nested.basic_types_value().int32_value(73);
  EXPECT_EQ(73, nested.basic_types_value().int32_value());

  views::NestedView<>::Constraints constraints;
  views::NestedView<>::Constraints baseline;
  EXPECT_TRUE(constraints.CheckCompatible(baseline));
  EXPECT_EQ(constraints, baseline);

  views::MultiNestedView<>::Constraints sequence_constraints;
  views::MultiNestedView<>::Constraints sequence_baseline;
  sequence_constraints.unbounded_sequence_of_unbounded_sequences.size = 4;
  sequence_baseline.unbounded_sequence_of_unbounded_sequences.size = 3;
  EXPECT_FALSE(sequence_constraints.CheckCompatible(sequence_baseline));

  sequence_constraints.unbounded_sequence_of_unbounded_sequences.element.int32_values.size = 4;
  sequence_baseline.unbounded_sequence_of_unbounded_sequences.element.int32_values.size = 3;
  EXPECT_FALSE(sequence_constraints.CheckCompatible(sequence_baseline));
}

TEST(test_views, nested_sequence_and_array_elements_are_views) {
  msg::UnboundedSequences element;
  views::MultiNested multi;
  multi.unbounded_sequence_of_unbounded_sequences(
    std::vector<msg::UnboundedSequences>{element});

  auto sequence = multi.unbounded_sequence_of_unbounded_sequences();
  EXPECT_EQ(1u, sequence.size());
  for (auto nested : sequence) {
    nested.int32_values().push_back(81);
  }
  EXPECT_EQ(
    81, multi.unbounded_sequence_of_unbounded_sequences()[0].int32_values().at(0));
  EXPECT_THROW(sequence.at(1), std::out_of_range);

  const auto & const_multi = multi;
  auto const_sequence = const_multi.unbounded_sequence_of_unbounded_sequences();
  static_assert(std::is_same_v<
      decltype(const_sequence[0].int32_values()), const std::vector<int32_t> &>);

  views::MultiNested array;
  array.array_of_unbounded_sequences()[0].int32_values().push_back(27);
  EXPECT_EQ(27, array.array_of_unbounded_sequences()[0].int32_values().at(0));
}

TEST(test_views, bounded_strings_are_validated_in_plain_messages) {
  views::ViewConstraintCases::Constraints limits;
  msg::ViewConstraintCases message;
  std::string path;
  const auto report = [&path](std::string_view field, int) {path = field;};

  message.bounded_value = "abc";
  message.bounded_array[1] = "abc";
  message.bounded_sequence.push_back("abc");
  message.unbounded_sequence.push_back("abc");
  EXPECT_TRUE(limits.CheckCompatible(message, report));

  message.bounded_value = "abcd";
  EXPECT_FALSE(limits.CheckCompatible(message, report));
  EXPECT_EQ("bounded_value", path);
  message.bounded_value = "abc";

  message.bounded_array[1] = "abcd";
  EXPECT_FALSE(limits.CheckCompatible(message, report));
  EXPECT_EQ("bounded_array[1]", path);
  message.bounded_array[1] = "abc";

  message.bounded_sequence[0] = "abcd";
  EXPECT_FALSE(limits.CheckCompatible(message, report));
  EXPECT_EQ("bounded_sequence[0]", path);
  message.bounded_sequence[0] = "abc";

  message.unbounded_sequence[0] = "abcd";
  EXPECT_FALSE(limits.CheckCompatible(message, report));
  EXPECT_EQ("unbounded_sequence[0]", path);

  message.unbounded_sequence[0] = "abc";
  limits.bounded_value.size = 2;
  EXPECT_FALSE(limits.CheckCompatible(message, report));
  EXPECT_EQ("bounded_value", path);
  limits.bounded_value.size = 0;
  limits.bounded_array.size = 2;
  EXPECT_FALSE(limits.CheckCompatible(message, report));
  EXPECT_EQ("bounded_array[1]", path);
  limits.bounded_array.size = 0;
  limits.bounded_sequence.element.size = 2;
  EXPECT_FALSE(limits.CheckCompatible(message, report));
  EXPECT_EQ("bounded_sequence[0]", path);
}

TEST(test_views, nested_constraints_preserve_zero_and_reject_looser_limits) {
  views::MultiNested::Constraints baseline;
  baseline.bounded_sequence_of_bounded_sequences.size = 2;
  baseline.bounded_sequence_of_bounded_sequences.element.string_values.size = 2;
  baseline.bounded_sequence_of_bounded_sequences.element.string_values.element.size = 3;
  views::MultiNested::Constraints requested;
  EXPECT_TRUE(requested.CheckCompatible(baseline));

  msg::MultiNested message;
  msg::BoundedSequences item;
  item.string_values.push_back("abcd");
  message.bounded_sequence_of_bounded_sequences.push_back(item);
  EXPECT_TRUE(requested.CheckCompatible(message));

  std::string path;
  const auto report = [&path](std::string_view field, int) {path = field;};
  EXPECT_FALSE(baseline.CheckCompatible(message, report));
  EXPECT_EQ("bounded_sequence_of_bounded_sequences[0].string_values[0]", path);

  requested.bounded_sequence_of_bounded_sequences.element.string_values.element.size = 4;
  EXPECT_FALSE(requested.CheckCompatible(baseline, report));
  EXPECT_EQ("bounded_sequence_of_bounded_sequences.string_values", path);
  EXPECT_THROW(message.bounded_sequence_of_bounded_sequences.resize(4), std::length_error);
}

TEST(test_views, moved_from_plain_view_rejects_access) {
  views::Strings original;
  auto moved = std::move(original);
  EXPECT_THROW(original.string_value(), std::logic_error);
  EXPECT_NO_THROW(moved.string_value("still valid"));
}
