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

#include <cstring>

#include "gtest/gtest.h"

extern "C"
{
#include "rcutils/allocator.h"
#include "rosidl_runtime_c/experimental/string.h"

ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING(
  rosidl_runtime_c__experimental__BoundedString5, char, 5U);
}

TEST(TestExperimentalString, assign_and_append)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rosidl_runtime_c__experimental__String string{};

  ASSERT_TRUE(rosidl_runtime_c__experimental__String__init(&string, &allocator));
  ASSERT_TRUE(rosidl_runtime_c__experimental__String__assignn(&string, "hello", 5U));
  ASSERT_TRUE(rosidl_runtime_c__experimental__String__appendn(&string, " world", 6U));

  EXPECT_EQ(11U, string.size);
  EXPECT_STREQ("hello world", string.value);

  rosidl_runtime_c__experimental__String__fini(&string);
}

TEST(TestExperimentalString, external_region_is_fixed_capacity)
{
  char storage[8] = {};
  rosidl_memory_region_t region{};
  region.location.address = storage;
  region.location.attributes = 0;
  region.size = sizeof(storage);

  rosidl_runtime_c__experimental__String string{};
  ASSERT_TRUE(rosidl_runtime_c__experimental__String__init_from_region(&string, region));
  ASSERT_TRUE(rosidl_runtime_c__experimental__String__assignn(&string, "abc", 3U));
  EXPECT_FALSE(rosidl_runtime_c__experimental__String__assignn(&string, "0123456789", 10U));
  EXPECT_EQ(0, std::strncmp("abc", string.value, 3));
}

TEST(TestExperimentalString, bounded_string_upper_bound_is_enforced)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rosidl_runtime_c__experimental__BoundedString5 string{};

  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedString5__init(&string, &allocator));
  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedString5__assignn(&string, "hello", 5U));
  EXPECT_FALSE(rosidl_runtime_c__experimental__BoundedString5__appendn(&string, "!", 1U));
  EXPECT_FALSE(rosidl_runtime_c__experimental__BoundedString5__reserve(&string, 6U));

  rosidl_runtime_c__experimental__BoundedString5__fini(&string);
}

TEST(TestExperimentalString, bounded_string_external_region_is_clamped)
{
  char storage[32] = {};
  rosidl_memory_region_t region{};
  region.location.address = storage;
  region.location.attributes = 0;
  region.size = sizeof(storage);

  rosidl_runtime_c__experimental__BoundedString5 string{};
  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedString5__init_from_region(&string, region));
  EXPECT_EQ(5U, string._impl.capacity);
  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedString5__assignn(&string, "abcde", 5U));
  EXPECT_FALSE(rosidl_runtime_c__experimental__BoundedString5__assignn(&string, "abcdef", 6U));
}

TEST(TestExperimentalString, string_assign_resize_copy_and_equality)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rosidl_runtime_c__experimental__BoundedString5 lhs{};
  rosidl_runtime_c__experimental__BoundedString5 rhs{};

  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedString5__init(&lhs, &allocator));
  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedString5__init(&rhs, &allocator));

  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedString5__assign(&lhs, "hi"));
  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedString5__resize(&lhs, 4U));
  EXPECT_EQ(4U, lhs.size);
  EXPECT_EQ('\0', lhs.value[2]);

  ASSERT_TRUE(rosidl_runtime_c__experimental__BoundedString5__copy(&lhs, &rhs));
  EXPECT_TRUE(rosidl_runtime_c__experimental__BoundedString5__are_equal(&lhs, &rhs));

  rhs.value[0] = 'H';
  EXPECT_FALSE(rosidl_runtime_c__experimental__BoundedString5__are_equal(&lhs, &rhs));

  rosidl_runtime_c__experimental__BoundedString5__fini(&lhs);
  rosidl_runtime_c__experimental__BoundedString5__fini(&rhs);
}
