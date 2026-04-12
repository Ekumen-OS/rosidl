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

#ifndef ROSIDL_RUNTIME_C__EXPERIMENTAL__CONSTRAINTS_H_
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__CONSTRAINTS_H_

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @file
/// @brief Runtime constraint types for experimental C message fields.

/// @brief Constraints for a string (narrow or wide) member.
///
/// Carries the maximum character length for an unbounded string.
/// Using a struct rather than a bare `size_t` keeps the access syntax
/// consistent with other constraint types: `constraints.my_string.size`.
typedef struct rosidl_runtime_c__experimental__StringConstraint_s
{
  /// Maximum character length (0 = no limit imposed at this layer).
  size_t size;
} rosidl_runtime_c__experimental__StringConstraint;

static inline bool
rosidl_runtime_c__experimental__StringConstraint__are_equal(
  const rosidl_runtime_c__experimental__StringConstraint * lhs,
  const rosidl_runtime_c__experimental__StringConstraint * rhs)
{
  return lhs->size == rhs->size;
}

#ifdef __cplusplus
}
#endif

#endif  // ROSIDL_RUNTIME_C__EXPERIMENTAL__CONSTRAINTS_H_
