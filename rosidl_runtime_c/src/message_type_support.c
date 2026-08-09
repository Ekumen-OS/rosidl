// Copyright 2016 Open Source Robotics Foundation, Inc.
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

#include "rosidl_runtime_c/message_type_support_struct.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

rosidl_message_type_support_t rosidl_get_zero_initialized_message_type_support_handle(void)
{
  static rosidl_message_type_support_t null_message_type_support = {
    .typesupport_identifier = NULL,
    .data = NULL,
    .func = NULL,
    .get_type_hash_func = NULL,
    .get_type_description_func = NULL,
    .get_type_description_sources_func = NULL
  };
  return null_message_type_support;
}

bool
rosidl_runtime_c_typesupport_identifier_matches(
  const char * identifier, const char * pattern)
{
  assert(identifier);
  assert(pattern);

  // A trailing '*' makes the pattern a prefix match (used by middleware to
  // select any typesupport of a family, e.g. "rosidl_typesupport_xcdr*").
  // Without it, the match is exact — identical to strcmp.
  const size_t pattern_length = strlen(pattern);
  if (pattern_length > 0 && pattern[pattern_length - 1] == '*') {
    return strncmp(identifier, pattern, pattern_length - 1) == 0;
  }
  return strcmp(identifier, pattern) == 0;
}

const rosidl_message_type_support_t * get_message_typesupport_handle(
  const rosidl_message_type_support_t * handle, const char * identifier)
{
  assert(handle);
  assert(handle->func);
  rosidl_message_typesupport_handle_function func =
    (rosidl_message_typesupport_handle_function)(handle->func);
  return func(handle, identifier);
}

const rosidl_message_type_support_t * get_message_typesupport_handle_function(
  const rosidl_message_type_support_t * handle, const char * identifier)
{
  assert(handle);
  assert(handle->typesupport_identifier);
  assert(identifier);
  if (rosidl_runtime_c_typesupport_identifier_matches(
      handle->typesupport_identifier, identifier))
  {
    return handle;
  }
  return 0;
}

rosidl_message_type_constraints_t rosidl_get_zero_initialized_message_type_constraints(void)
{
  static rosidl_message_type_constraints_t null_constraints = {
    .type_specific = NULL,
    .max_string_length = 0u,
    .max_total_size = 0u,
    .strict = false
  };
  return null_constraints;
}
