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

#include <cstdint>

#include "gtest/gtest.h"

extern "C"
{
#include "rcutils/allocator.h"
#include "rosidl_runtime_c/experimental/sequence.h"

ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE(
  rosidl_runtime_c__experimental__BoundedInt32Sequence, int32_t, 3U);
}

TEST(TestExperimentalSequence, managed_resize_and_push_back)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rosidl_runtime_c__experimental__Int32Sequence sequence{};

  ASSERT_TRUE(rosidl_runtime_c__experimental__Int32Sequence__init(&sequence, &allocator));
  ASSERT_TRUE(rosidl_runtime_c__experimental__Int32Sequence__resize(&sequence, 2U));
  sequence.value[0] = 3;
  sequence.value[1] = 5;
  ASSERT_TRUE(rosidl_runtime_c__experimental__Int32Sequence__push_back(&sequence, 8));

  EXPECT_EQ(3U, sequence.size);
  EXPECT_GE(sequence.capacity, 3U);
  EXPECT_EQ(8, sequence.value[2]);

  rosidl_runtime_c__experimental__Int32Sequence__fini(&sequence);
}

TEST(TestExperimentalSequence, fixed_region_capacity)
{
  int32_t backing[3] = {0, 0, 0};
  rosidl_memory_region_t region{};
  region.location.address = backing;
  region.location.attributes = 0;
  region.size = sizeof(backing);

  rosidl_runtime_c__experimental__Int32Sequence sequence{};
  ASSERT_TRUE(rosidl_runtime_c__experimental__Int32Sequence__init_from_region(&sequence, region));
  EXPECT_EQ(3U, sequence.capacity);
  EXPECT_TRUE(rosidl_runtime_c__experimental__Int32Sequence__resize(&sequence, 3U));
  EXPECT_FALSE(rosidl_runtime_c__experimental__Int32Sequence__push_back(&sequence, 1));
}

TEST(TestExperimentalSequence, bounded_sequence_upper_bound_is_enforced)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rosidl_runtime_c__experimental__BoundedInt32Sequence sequence{};

  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedInt32Sequence__init(&sequence, &allocator));
  EXPECT_TRUE(rosidl_runtime_c__experimental__BoundedInt32Sequence__push_back(&sequence, 1));
  EXPECT_TRUE(rosidl_runtime_c__experimental__BoundedInt32Sequence__push_back(&sequence, 2));
  EXPECT_TRUE(rosidl_runtime_c__experimental__BoundedInt32Sequence__push_back(&sequence, 3));
  EXPECT_FALSE(rosidl_runtime_c__experimental__BoundedInt32Sequence__push_back(&sequence, 4));
  EXPECT_FALSE(rosidl_runtime_c__experimental__BoundedInt32Sequence__reserve(&sequence, 4U));

  rosidl_runtime_c__experimental__BoundedInt32Sequence__fini(&sequence);
}

TEST(TestExperimentalSequence, bounded_sequence_external_region_is_clamped)
{
  int32_t backing[8] = {0};
  rosidl_memory_region_t region{};
  region.location.address = backing;
  region.location.attributes = 0;
  region.size = sizeof(backing);

  rosidl_runtime_c__experimental__BoundedInt32Sequence sequence{};
  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedInt32Sequence__init_from_region(&sequence,
    region));
  EXPECT_EQ(3U, sequence.capacity);
  EXPECT_TRUE(rosidl_runtime_c__experimental__BoundedInt32Sequence__resize(&sequence, 3U));
  EXPECT_FALSE(rosidl_runtime_c__experimental__BoundedInt32Sequence__resize(&sequence, 4U));
}

TEST(TestExperimentalSequence, sequence_copy_and_equality)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rosidl_runtime_c__experimental__BoundedInt32Sequence lhs{};
  rosidl_runtime_c__experimental__BoundedInt32Sequence rhs{};

  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedInt32Sequence__init(&lhs, &allocator));
  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedInt32Sequence__init(&rhs, &allocator));

  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedInt32Sequence__push_back(&lhs, 10));
  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedInt32Sequence__push_back(&lhs, 20));

  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedInt32Sequence__copy(&lhs, &rhs));
  EXPECT_TRUE(rosidl_runtime_c__experimental__BoundedInt32Sequence__are_equal(&lhs, &rhs));

  rhs.value[1] = 99;
  EXPECT_FALSE(rosidl_runtime_c__experimental__BoundedInt32Sequence__are_equal(&lhs, &rhs));

  rosidl_runtime_c__experimental__BoundedInt32Sequence__fini(&lhs);
  rosidl_runtime_c__experimental__BoundedInt32Sequence__fini(&rhs);
}
