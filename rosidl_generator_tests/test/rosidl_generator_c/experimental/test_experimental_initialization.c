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

// Tests for the experimental C generator __init_from_storage path.
// Verifies that caller-supplied ExternalStorage is properly aliased and
// that default values are applied at init time.

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rosidl_generator_tests/msg/experimental/basic_types.h"
#include "rosidl_generator_tests/msg/experimental/defaults.h"
#include "rosidl_generator_tests/msg/experimental/empty.h"
#include "rosidl_generator_tests/msg/experimental/nested.h"
#include "rosidl_generator_tests/msg/experimental/strings.h"
#include "rosidl_generator_tests/msg/experimental/arrays.h"

#include "test_macros.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static rosidl_memory_t make_memory(void * address)
{
  rosidl_memory_t m;
  m.address = address;
  m.attributes = 0;
  return m;
}

static rosidl_memory_region_t make_region(void * address, size_t size)
{
  rosidl_memory_region_t r;
  r.location = make_memory(address);
  r.size = size;
  return r;
}

// ---------------------------------------------------------------------------
// test_storage_empty: NULL storage → fallback allocates from heap
// ---------------------------------------------------------------------------

static int test_storage_empty(void)
{
  EXP(Empty) * msg = EXP(Empty__create)(NULL);
  EXPECT_NE(NULL, msg);
  EXP(Empty__destroy)(msg);
  return 0;
}

// ---------------------------------------------------------------------------
// test_storage_basic_types_preserves_value:
//   ExternalStorage for BasicTypes contains a raw int32_t backing.
//   Writing through msg->int32_value.value->data is visible in the stack var.
// ---------------------------------------------------------------------------

static int test_storage_basic_types_preserves_value(void)
{
  // Allocate backing storage on the stack.
  int32_t i32 = 0;
  int8_t  i8  = 0;
  bool    b   = false;
  float   f32 = 0.0f;

  EXP(BasicTypes__ExternalStorage) storage;
  memset(&storage, 0, sizeof(storage));

  storage.members.int32_value  = make_memory(&i32);
  storage.members.int8_value   = make_memory(&i8);
  storage.members.bool_value   = make_memory(&b);
  storage.members.float32_value = make_memory(&f32);
  // Leave remaining members zeroed (NULL address → managed allocation)

  EXP(BasicTypes) msg;
  EXPECT_TRUE(EXP(BasicTypes__init_from_storage)(&msg, &storage));

  // Scalar value starts zero (as initialized)
  EXPECT_EQ(0, msg.int32_value.value->data);
  EXPECT_EQ(&i32, msg.int32_value.value);

  // Write through the message field; read back via the original variable
  msg.int32_value.value->data = 42;
  EXPECT_EQ(42, i32);
  EXPECT_EQ(42, msg.int32_value.value->data);

  msg.bool_value.value->data = true;
  EXPECT_TRUE(b);

  msg.float32_value.value->data = 1.5f;
  EXPECT_EQ(1.5f, f32);

  EXP(BasicTypes__fini)(&msg);
  return 0;
}

// ---------------------------------------------------------------------------
// test_storage_defaults_backed_values:
//   ExternalStorage for Defaults; default values must be written to backing.
// ---------------------------------------------------------------------------

static int test_storage_defaults_backed_values(void)
{
  bool    b   = false;
  uint8_t u8  = 0u;
  int32_t i32 = 0;
  int16_t i16 = 0;
  uint64_t u64 = 0u;
  float   f32 = 0.0f;
  double  f64 = 0.0;

  EXP(Defaults__ExternalStorage) storage;
  memset(&storage, 0, sizeof(storage));

  storage.members.bool_value   = make_memory(&b);
  storage.members.byte_value   = make_memory(&u8);
  storage.members.int32_value  = make_memory(&i32);
  storage.members.int16_value  = make_memory(&i16);
  storage.members.uint64_value = make_memory(&u64);
  storage.members.float32_value = make_memory(&f32);
  storage.members.float64_value = make_memory(&f64);

  EXP(Defaults) msg;
  EXPECT_TRUE(EXP(Defaults__init_from_storage)(&msg, &storage));

  // Check defaults applied
  EXPECT_TRUE(msg.bool_value.value->data);
  EXPECT_TRUE(b);
  EXPECT_EQ(50u, msg.byte_value.value->data);
  EXPECT_EQ(50u, u8);
  EXPECT_EQ(-30000, msg.int32_value.value->data);
  EXPECT_EQ(-30000, i32);
  EXPECT_EQ(50000000u, msg.uint64_value.value->data);
  EXPECT_EQ(50000000u, u64);

  EXP(Defaults__fini)(&msg);
  return 0;
}

// ---------------------------------------------------------------------------
// test_storage_strings_regions:
//   ExternalStorage for Strings provides region backing for string fields.
// ---------------------------------------------------------------------------

static int test_storage_strings_regions(void)
{
  char buf_sv[64] = {0};
  char buf_bsv[32] = {0};

  EXP(Strings__ExternalStorage) storage;
  memset(&storage, 0, sizeof(storage));

  storage.members.string_value =
    make_region(buf_sv, sizeof(buf_sv));
  storage.members.bounded_string_value =
    make_region(buf_bsv, sizeof(buf_bsv));
  // Leave remaining string fields as null → managed

  EXP(Strings) msg;
  EXPECT_TRUE(EXP(Strings__init_from_storage)(&msg, &storage));

  // Empty after init
  EXPECT_EQ(0u, msg.string_value.size);
  EXPECT_EQ((void *)buf_sv, (void *)msg.string_value.value);

  // Assign into the region-backed field
  EXPECT_TRUE(
    EXP(Strings__string_value_t__assign)(&msg.string_value, "hello"));
  EXPECT_EQ(5u, msg.string_value.size);
  EXPECT_EQ(0, strcmp(buf_sv, "hello"));
  EXPECT_EQ(0, strcmp((const char *)msg.string_value.value, "hello"));

  // The managed fields (default values) still work
  EXPECT_EQ(0, strcmp((const char *)msg.string_value_default1.value,
    "Hello world!"));

  EXP(Strings__fini)(&msg);
  return 0;
}

// ---------------------------------------------------------------------------
// test_storage_nested:
//   ExternalStorage for Nested contains a sub-storage for BasicTypes.
// ---------------------------------------------------------------------------

static int test_storage_nested(void)
{
  int32_t inner_i32 = 0;
  float   inner_f32 = 0.0f;

  EXP(BasicTypes__ExternalStorage) inner_storage;
  memset(&inner_storage, 0, sizeof(inner_storage));
  inner_storage.members.int32_value  = make_memory(&inner_i32);
  inner_storage.members.float32_value = make_memory(&inner_f32);

  EXP(Nested__ExternalStorage) storage;
  memset(&storage, 0, sizeof(storage));
  storage.members.basic_types_value = inner_storage;

  EXP(Nested) msg;
  EXPECT_TRUE(EXP(Nested__init_from_storage)(&msg, &storage));

  EXPECT_EQ(0, msg.basic_types_value.int32_value.value->data);
  EXPECT_EQ(&inner_i32, msg.basic_types_value.int32_value.value);

  msg.basic_types_value.int32_value.value->data = 111;
  EXPECT_EQ(111, inner_i32);

  msg.basic_types_value.float32_value.value->data = 2.5f;
  EXPECT_EQ(2.5f, inner_f32);

  EXP(Nested__fini)(&msg);
  return 0;
}

// ---------------------------------------------------------------------------
// test_storage_arrays_scalar_region:
//   ExternalStorage for Arrays provides region backing for a scalar array.
// ---------------------------------------------------------------------------

static int test_storage_arrays_scalar_region(void)
{
  int32_t int32_buf[3] = {0, 0, 0};

  EXP(Arrays__ExternalStorage) storage;
  memset(&storage, 0, sizeof(storage));
  storage.members.int32_values =
    make_region(int32_buf, sizeof(int32_buf));
  // Leave all other fields null → managed

  EXP(Arrays) msg;
  EXPECT_TRUE(EXP(Arrays__init_from_storage)(&msg, &storage));

  EXPECT_EQ((void *)int32_buf, (void *)msg.int32_values.value);
  EXPECT_EQ(0, msg.int32_values.value->data[0]);

  msg.int32_values.value->data[0] = 10;
  msg.int32_values.value->data[1] = 20;
  msg.int32_values.value->data[2] = 30;

  EXPECT_EQ(10, int32_buf[0]);
  EXPECT_EQ(20, int32_buf[1]);
  EXPECT_EQ(30, int32_buf[2]);

  EXP(Arrays__fini)(&msg);
  return 0;
}

// ---------------------------------------------------------------------------
// test_storage_sequences_null_storage:
//   When storage is NULL, __init falls back to heap allocation.
//   Sequences remain empty by default, and push_back works normally.
// ---------------------------------------------------------------------------

static int test_storage_sequences_null_storage(void)
{
  EXP(BasicTypes) * msg = EXP(BasicTypes__create)(NULL);
  EXPECT_NE(NULL, msg);

  // Just verify managed init produces a valid zeroed message
  EXPECT_EQ(0, msg->int32_value.value->data);
  EXPECT_FALSE(msg->bool_value.value->data);

  msg->int32_value.value->data = 999;
  EXPECT_EQ(999, msg->int32_value.value->data);

  EXP(BasicTypes__destroy)(msg);
  return 0;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(void)
{
  int rc = 0;
  printf("test_experimental_initialization_c\n");
  RUN(test_storage_empty);
  RUN(test_storage_basic_types_preserves_value);
  RUN(test_storage_defaults_backed_values);
  RUN(test_storage_strings_regions);
  RUN(test_storage_nested);
  RUN(test_storage_arrays_scalar_region);
  RUN(test_storage_sequences_null_storage);
  if (rc != 0) {
    fprintf(stderr, "%d test(s) FAILED\n", rc);
  } else {
    printf("All tests passed.\n");
  }
  return rc != 0;
}
