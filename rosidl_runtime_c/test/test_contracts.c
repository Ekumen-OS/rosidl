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

#include "rosidl_runtime_c/constraints.h"
#include "rosidl_runtime_c/memory.h"
#include "rosidl_runtime_c/message_context.h"

int main(void)
{
  unsigned char data[8] = {0};
  rosidl_memory_region_t region = {
    .location = {data, 0},
    .size = sizeof(data),
  };
  rosidl_runtime_c__message_context_t context = {
    .constraints = {
      .type_specific = NULL,
      .max_string_length = 16,
      .max_total_size = 64,
      .strict = true,
    },
  };
  rosidl_runtime_c__string_sequence_constraint_t strings = {
    .size = 4,
    .element = {8},
  };

  return region.location.address == data &&
         region.size == sizeof(data) &&
         context.constraints.max_string_length == 16 &&
         context.constraints.max_total_size == 64 &&
         context.constraints.strict &&
         strings.size == 4 &&
         strings.element.size == 8 ? 0 : 1;
}
