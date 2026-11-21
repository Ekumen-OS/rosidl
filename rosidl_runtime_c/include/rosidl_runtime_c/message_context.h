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

#ifndef ROSIDL_RUNTIME_C__MESSAGE_CONTEXT_H_
#define ROSIDL_RUNTIME_C__MESSAGE_CONTEXT_H_

#include "rosidl_runtime_c/constraints.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// Context for view-aware message operations.
///
/// The context is intentionally limited to the constraints currently defined
/// by the REP draft. Implementations must not assume a standardized type
/// support operation table or storage-backend representation.
typedef struct rosidl_runtime_c__message_context_s
{
  rosidl_runtime_c__message_type_constraints_t constraints;
} rosidl_runtime_c__message_context_t;

#ifdef __cplusplus
}
#endif

#endif  // ROSIDL_RUNTIME_C__MESSAGE_CONTEXT_H_
