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

#ifndef ROSIDL_RUNTIME_C__EXPERIMENTAL__MEMORY_H_
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__MEMORY_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct rosidl_memory_s
{
  void * address;
  int attributes;
} rosidl_memory_t;

typedef struct rosidl_memory_region_s
{
  rosidl_memory_t location;
  size_t size;
} rosidl_memory_region_t;

#ifdef __cplusplus
}
#endif

#endif  // ROSIDL_RUNTIME_C__EXPERIMENTAL__MEMORY_H_
