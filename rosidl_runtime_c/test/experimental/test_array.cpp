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
#include "rosidl_runtime_c/experimental/array.h"
}

using Int32 = std::int32_t;
typedef rosidl_runtime_c__experimental__Int32Array Int32Array;

#define Int32Array__init rosidl_runtime_c__experimental__Int32Array__init
#define Int32Array__init_from_region rosidl_runtime_c__experimental__Int32Array__init_from_region
#define Int32Array__fini rosidl_runtime_c__experimental__Int32Array__fini

ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY(rosidl_runtime_c__experimental__FourIntsArray, Int32, 4U);

#undef Int32Array__fini
#undef Int32Array__init_from_region
#undef Int32Array__init

TEST(TestExperimentalArray, local_storage_access)
{
  rosidl_runtime_c__experimental__FourIntsArray array{};
  ASSERT_TRUE(rosidl_runtime_c__experimental__FourIntsArray__init(&array));
  ASSERT_NE(nullptr, array.value);

  for (size_t i = 0; i < 4U; ++i) {
    array.value->data[i] = static_cast<int32_t>(i + 1U);
  }

  EXPECT_EQ(4, array.value->data[3U]);
}

TEST(TestExperimentalArray, external_storage_access)
{
  int32_t storage[4] = {0, 0, 0, 0};
  rosidl_memory_region_t region{};
  region.location.address = storage;
  region.location.attributes = 0;
  region.size = sizeof(storage);

  rosidl_runtime_c__experimental__FourIntsArray array{};
  ASSERT_TRUE(rosidl_runtime_c__experimental__FourIntsArray__init_from_region(&array, region));
  ASSERT_NE(nullptr, array.value);
  array.value->data[0U] = 11;
  array.value->data[1U] = 22;
  EXPECT_EQ(11, storage[0]);
  EXPECT_EQ(22, storage[1]);
}
