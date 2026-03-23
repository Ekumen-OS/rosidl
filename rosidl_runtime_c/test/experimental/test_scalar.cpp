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
#include "rosidl_runtime_c/experimental/scalar.h"
}

TEST(TestExperimentalScalar, local_storage_value_accessor)
{
  rosidl_runtime_c__experimental__Int32 scalar{};
  ASSERT_TRUE(rosidl_runtime_c__experimental__Int32__init(&scalar));
  ASSERT_NE(nullptr, scalar.value);
  scalar.value->data = 42;
  EXPECT_EQ(42, scalar.value->data);
  rosidl_runtime_c__experimental__Int32__fini(&scalar);
}

TEST(TestExperimentalScalar, external_storage_value_accessor)
{
  int32_t external_value = 7;
  rosidl_memory_t memory{};
  memory.address = &external_value;
  memory.attributes = 0;

  rosidl_runtime_c__experimental__Int32 scalar{};
  ASSERT_TRUE(rosidl_runtime_c__experimental__Int32__init_from_memory(&scalar, memory));
  scalar.value->data = 13;
  EXPECT_EQ(13, external_value);
}
