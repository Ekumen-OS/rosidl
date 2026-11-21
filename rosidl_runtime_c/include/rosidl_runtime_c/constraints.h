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

#ifndef ROSIDL_RUNTIME_C__CONSTRAINTS_H_
#define ROSIDL_RUNTIME_C__CONSTRAINTS_H_

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// Maximum character length for an otherwise unbounded string.
/// A value of zero imposes no additional limit.
typedef struct rosidl_runtime_c__string_constraint_s
{
  size_t size;
} rosidl_runtime_c__string_constraint_t;

/// Maximum element count for an otherwise unbounded sequence.
/// A value of zero imposes no additional limit.
typedef struct rosidl_runtime_c__sequence_constraint_s
{
  size_t size;
} rosidl_runtime_c__sequence_constraint_t;

/// Maximum element count and per-element string limit for string sequences.
typedef struct rosidl_runtime_c__string_sequence_constraint_s
{
  size_t size;
  rosidl_runtime_c__string_constraint_t element;
} rosidl_runtime_c__string_sequence_constraint_t;

/// Maximum element count and per-element wide-string limit for wide-string sequences.
typedef struct rosidl_runtime_c__wstring_sequence_constraint_s
{
  size_t size;
  rosidl_runtime_c__string_constraint_t element;
} rosidl_runtime_c__wstring_sequence_constraint_t;

/// Type-specific and blanket limits used by view-aware APIs.
///
/// The object referenced by type_specific is generated, message-specific data.
/// It is not an ABI for type support operations.
typedef struct rosidl_runtime_c__message_type_constraints_s
{
  const void * type_specific;
  size_t max_string_length;
  size_t max_total_size;
  bool strict;
} rosidl_runtime_c__message_type_constraints_t;

#ifdef __cplusplus
}
#endif

#endif  // ROSIDL_RUNTIME_C__CONSTRAINTS_H_
