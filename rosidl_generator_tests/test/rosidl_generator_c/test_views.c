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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rcutils/error_handling.h"
#include "rosidl_generator_tests/msg/views/arrays.h"
#include "rosidl_generator_tests/msg/views/basic_types.h"
#include "rosidl_generator_tests/msg/views/nested.h"
#include "rosidl_generator_tests/msg/views/unbounded_sequences.h"
#include "rosidl_generator_tests/msg/views/w_strings.h"
#include "rosidl_generator_tests/srv/views/basic_types.h"
#include "rosidl_runtime_c/primitives_sequence_functions.h"
#include "rosidl_runtime_c/string_functions.h"
#include "rosidl_runtime_c/u16string_functions.h"

#define EXPECT_TRUE(expression) \
  do { \
    if (!(expression)) { \
      fprintf(stderr, "%s:%d: %s failed\n", __FILE__, __LINE__, #expression); \
      return 1; \
    } \
  } while (0)

#define EXPECT_EQ(actual, expected) \
  do { \
    if ((actual) != (expected)) { \
      fprintf(stderr, "%s:%d: %s != %s\n", __FILE__, __LINE__, #actual, #expected); \
      return 1; \
    } \
  } while (0)

int main(void)
{
  rosidl_generator_tests__msg__BasicTypes__View owned = {0};
  EXPECT_TRUE(rosidl_generator_tests__msg__BasicTypes__View__init_owned(&owned));
  EXPECT_TRUE(owned.owns_message);
  *rosidl_generator_tests__msg__BasicTypes__View__get__int32_value(&owned) = 42;
  EXPECT_EQ(owned.message->int32_value, 42);
  rosidl_generator_tests__msg__BasicTypes * released = NULL;
  bool was_owned = false;
  EXPECT_TRUE(rosidl_generator_tests__msg__BasicTypes__View__release(&owned, &released,
    &was_owned));
  EXPECT_TRUE(was_owned && released->int32_value == 42);
  EXPECT_TRUE(!rosidl_generator_tests__msg__BasicTypes__View__get__int32_value(&owned));
  rcutils_reset_error();
  EXPECT_TRUE(!rosidl_generator_tests__msg__BasicTypes__View__fini(&owned));
  rcutils_reset_error();
  rosidl_generator_tests__msg__BasicTypes__destroy(released);

  rosidl_generator_tests__msg__Arrays * plain = rosidl_generator_tests__msg__Arrays__create();
  EXPECT_TRUE(plain);
  rosidl_generator_tests__msg__Arrays__View borrowed = {0};
  EXPECT_TRUE(rosidl_generator_tests__msg__Arrays__View__init_borrowed(&borrowed, plain));
  rosidl_generator_tests__msg__Arrays__View__get__int32_values(&borrowed)[1] = 9;
  EXPECT_EQ(plain->int32_values[1], 9);
  EXPECT_TRUE(rosidl_generator_tests__msg__Arrays__View__fini(&borrowed));
  EXPECT_EQ(plain->int32_values[1], 9);
  EXPECT_TRUE(rosidl_generator_tests__msg__Arrays__View__init_borrowed(&borrowed, plain));
  rosidl_generator_tests__msg__Arrays * borrowed_message = NULL;
  was_owned = true;
  EXPECT_TRUE(rosidl_generator_tests__msg__Arrays__View__release(
      &borrowed, &borrowed_message, &was_owned));
  EXPECT_TRUE(borrowed_message == plain && !was_owned);

  rosidl_generator_tests__msg__Arrays__ConstView read_only = {0};
  EXPECT_TRUE(rosidl_generator_tests__msg__Arrays__ConstView__init_borrowed(&read_only, plain));
  EXPECT_EQ(rosidl_generator_tests__msg__Arrays__ConstView__get__int32_values(&read_only)[1], 9);
  const rosidl_generator_tests__msg__Arrays * original = NULL;
  EXPECT_TRUE(rosidl_generator_tests__msg__Arrays__ConstView__release(&read_only, &original));
  EXPECT_EQ(original, plain);
  rosidl_generator_tests__msg__Arrays__destroy(plain);

  rosidl_generator_tests__srv__BasicTypes_Request__View request = {0};
  EXPECT_TRUE(rosidl_generator_tests__srv__BasicTypes_Request__View__init_owned(&request));
  EXPECT_TRUE(rosidl_runtime_c__String__assign(
      rosidl_generator_tests__srv__BasicTypes_Request__View__get__string_value(&request), "hello"));
  EXPECT_EQ(strcmp(request.message->string_value.data, "hello"), 0);
  EXPECT_TRUE(rosidl_generator_tests__srv__BasicTypes_Request__View__fini(&request));

  rosidl_generator_tests__msg__Nested__View nested = {0};
  EXPECT_TRUE(rosidl_generator_tests__msg__Nested__View__init_owned(&nested));
  rosidl_generator_tests__msg__Nested__View__get__basic_types_value(&nested)->int32_value = 17;
  EXPECT_EQ(nested.message->basic_types_value.int32_value, 17);
  EXPECT_TRUE(rosidl_generator_tests__msg__Nested__View__fini(&nested));

  rosidl_generator_tests__msg__UnboundedSequences__View sequence = {0};
  EXPECT_TRUE(rosidl_generator_tests__msg__UnboundedSequences__View__init_owned(&sequence));
  rosidl_runtime_c__int32__Sequence * values =
    rosidl_generator_tests__msg__UnboundedSequences__View__get__int32_values(&sequence);
  rosidl_runtime_c__int32__Sequence__fini(values);
  EXPECT_TRUE(rosidl_runtime_c__int32__Sequence__init(values, 2));
  values->data[1] = 23;
  EXPECT_EQ(sequence.message->int32_values.data[1], 23);
  EXPECT_TRUE(rosidl_generator_tests__msg__UnboundedSequences__View__fini(&sequence));

  rosidl_generator_tests__msg__WStrings__View wide = {0};
  EXPECT_TRUE(rosidl_generator_tests__msg__WStrings__View__init_owned(&wide));
  const uint16_t wide_value[] = {'w', 'i', 'd', 'e', 0};
  EXPECT_TRUE(rosidl_runtime_c__U16String__assign(
      rosidl_generator_tests__msg__WStrings__View__get__wstring_value(&wide), wide_value));
  EXPECT_EQ(wide.message->wstring_value.size, 4);
  EXPECT_TRUE(rosidl_generator_tests__msg__WStrings__View__fini(&wide));
  return 0;
}
