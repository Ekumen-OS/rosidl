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
#include <type_traits>
#include <vector>

#include "rosidl_runtime_c/message_context.h"
#include "rosidl_runtime_cpp/constraints.hpp"
#include "rosidl_runtime_cpp/memory.hpp"
#include "rosidl_runtime_cpp/views/view_handle.hpp"
#include "rosidl_runtime_cpp/views/message_view.hpp"

namespace
{

struct TestMessage
{
  int32_t value{0};
};

struct TrackedView
{
  explicit TrackedView(int * count)
  : destructions(count) {}
  int * destructions;
  ~TrackedView() {++*destructions;}
};

struct BorrowedValueView
{
  explicit BorrowedValueView(int & value)
  : value(value) {}
  int & value;
};

template<bool Mutable>
struct ExposedMessageView : rosidl_runtime_cpp::views::MessageView<TestMessage, Mutable>
{
  using Base = rosidl_runtime_cpp::views::MessageView<TestMessage, Mutable>;
  using Base::Base;
  using Base::message;
};

}  // namespace

TEST(test_views, message_view_distinguishes_ownership_and_borrowing) {
  ExposedMessageView<true> owned;
  EXPECT_TRUE(owned.owns_message());
  owned.message().value = 12;
  EXPECT_EQ(12, owned.message().value);

  TestMessage message;
  ExposedMessageView<true> borrowed(message);
  EXPECT_FALSE(borrowed.owns_message());
  borrowed.message().value = 24;
  EXPECT_EQ(24, message.value);

  const TestMessage const_message{};
  ExposedMessageView<false> read_only(const_message);
  EXPECT_FALSE(read_only.owns_message());
  static_assert(!std::is_assignable_v<decltype((read_only.message().value)), int32_t>);

  auto moved = std::move(owned);
  EXPECT_TRUE(moved.owns_message());
  EXPECT_THROW(owned.message(), std::logic_error);

  const TestMessage source{36};
  ExposedMessageView<true> copied(source);
  EXPECT_TRUE(copied.owns_message());
  EXPECT_EQ(36, copied.message().value);
  copied.message().value = 48;
  EXPECT_EQ(36, source.value);

  ExposedMessageView<false> const_temporary(std::move(source));
  EXPECT_TRUE(const_temporary.owns_message());
  EXPECT_EQ(36, const_temporary.message().value);
}

TEST(test_views, message_sequence_view_borrows_elements) {
  std::vector<TestMessage> messages(2);
  using MutableSequenceView = rosidl_runtime_cpp::views::MessageSequenceView<
    std::vector<TestMessage>, ExposedMessageView<true>, true>;
  using ConstSequenceView = rosidl_runtime_cpp::views::MessageSequenceView<
    std::vector<TestMessage>, ExposedMessageView<false>, false>;

  MutableSequenceView sequence(&messages);
  EXPECT_EQ(2u, sequence.size());
  EXPECT_THROW(MutableSequenceView(nullptr), std::invalid_argument);
  for (auto message : sequence) {
    message.message().value = 19;
  }
  EXPECT_EQ(19, messages[0].value);
  EXPECT_EQ(19, messages[1].value);
  EXPECT_THROW(sequence.at(2), std::out_of_range);

  const auto & const_messages = messages;
  ConstSequenceView const_sequence(&const_messages);
  static_assert(!std::is_assignable_v<
      decltype((const_sequence[0].message().value)), int32_t>);
}

TEST(test_views, memory_regions_preserve_address_extent_and_attributes) {
  uint32_t values[3]{};
  rosidl_runtime_cpp::MemoryRegion<uint32_t> region(values, sizeof(values), 7);
  EXPECT_EQ(values, region.data());
  EXPECT_EQ(sizeof(values), region.size());
  EXPECT_EQ(3u, region.capacity());
  EXPECT_EQ(7, region.attributes());
}

TEST(test_views, borrowed_storage_handle_has_one_terminal_operation) {
  int destructions = 0;
  int storage[2] = {1, 2};
  using Handle = rosidl_runtime_cpp::views::ViewHandle<TrackedView>;
  Handle::Regions regions = {
    {{&storage[0], 1}, sizeof(storage[0])},
    {{&storage[1], 2}, sizeof(storage[1])},
  };
  Handle handle(std::make_unique<TrackedView>(&destructions), regions);
  EXPECT_EQ(&storage[0], handle.regions()[0].location.address);
  EXPECT_EQ(&storage[1], handle.regions()[1].location.address);
  EXPECT_EQ(&destructions, handle.view().destructions);
  EXPECT_EQ(0, destructions);

  auto released = handle.release();
  EXPECT_EQ(1, destructions);
  ASSERT_EQ(2u, released.size());
  EXPECT_EQ(&storage[0], released[0].location.address);
  EXPECT_EQ(2, released[1].location.attributes);
  EXPECT_EQ(2, storage[1]);
  EXPECT_THROW(handle.view(), std::logic_error);
  EXPECT_THROW(handle.regions(), std::logic_error);
  EXPECT_THROW(handle.release(), std::logic_error);
  EXPECT_THROW(handle.destroy(), std::logic_error);

  Handle other(std::make_unique<TrackedView>(&destructions), regions);
  other.destroy();
  EXPECT_EQ(2, destructions);
  EXPECT_EQ(1, storage[0]);
  EXPECT_THROW(other.release(), std::logic_error);
}

TEST(test_views, borrowed_storage_handle_exposes_live_region) {
  int storage = 1;
  using Handle = rosidl_runtime_cpp::views::ViewHandle<BorrowedValueView>;
  Handle handle(
    std::make_unique<BorrowedValueView>(storage), {{{&storage, 0}, sizeof(storage)}});
  handle.view().value = 7;
  EXPECT_EQ(&storage, handle.regions()[0].location.address);
  EXPECT_EQ(7, storage);
  auto regions = handle.release();
  EXPECT_EQ(&storage, regions[0].location.address);
  EXPECT_EQ(7, storage);
}

TEST(test_views, borrowed_storage_handle_cleans_up_and_moves) {
  int destructions = 0;
  int storage = 1;
  using Handle = rosidl_runtime_cpp::views::ViewHandle<TrackedView>;
  Handle::Regions regions = {{{&storage, 0}, sizeof(storage)}};
  {
    Handle source(std::make_unique<TrackedView>(&destructions), regions);
    Handle target(std::make_unique<TrackedView>(&destructions), regions);
    target = std::move(source);
    EXPECT_EQ(1, destructions);
    EXPECT_THROW(source.regions(), std::logic_error);
    EXPECT_EQ(&storage, target.regions()[0].location.address);
  }
  EXPECT_EQ(2, destructions);
  EXPECT_EQ(1, storage);
  EXPECT_THROW(Handle(nullptr, regions), std::invalid_argument);
  EXPECT_THROW(
    Handle(std::make_unique<TrackedView>(&destructions), {}),
    std::invalid_argument);
  EXPECT_THROW(
    Handle(std::make_unique<TrackedView>(&destructions), {{{nullptr, 0}, 1}}),
    std::invalid_argument);
}

TEST(test_views, context_and_nested_constraints_are_usable) {
  rosidl_runtime_c__message_context_t context{};
  context.constraints.max_string_length = 32;
  context.constraints.strict = true;
  EXPECT_EQ(32u, context.constraints.max_string_length);
  EXPECT_TRUE(context.constraints.strict);

  using StringSequence =
    rosidl_runtime_cpp::NestedSequenceConstraint<rosidl_runtime_cpp::StringConstraint>;
  StringSequence baseline{3, {12}};
  StringSequence tighter{2, {8}};
  EXPECT_TRUE(tighter.CheckCompatible(baseline));

  StringSequence looser{2, {16}};
  std::string_view reported_path;
  EXPECT_FALSE(looser.CheckCompatible(
      baseline,
      [&reported_path](std::string_view path, int) {reported_path = path;},
      "labels"));
  EXPECT_EQ("labels", reported_path);
}
