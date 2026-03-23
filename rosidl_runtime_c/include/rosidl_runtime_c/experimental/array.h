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

#ifndef ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_H_
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <uchar.h>

#include "rosidl_runtime_c/experimental/memory.h"
#include "rosidl_runtime_c/experimental/storage.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// @file
/// @brief Experimental C11 fixed-size array wrapper macros.

/// @brief Declare a size-1 array model type and function signatures.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(STRUCT_NAME, VALUE_TYPE) \
  typedef struct STRUCT_NAME ## _s \
  { \
    struct { \
      VALUE_TYPE data[1U]; \
    } * value; \
    struct \
    { \
      rosidl_runtime_c__experimental__storage_kind_t kind; \
      union \
      { \
        rosidl_memory_region_t region; \
        struct \
        { \
          VALUE_TYPE data[1U]; \
        } local; \
      } storage; \
    } _impl; \
  } STRUCT_NAME; \
  bool STRUCT_NAME ## __init(STRUCT_NAME * array, size_t size); \
  bool STRUCT_NAME ## __init_from_region(STRUCT_NAME * array, rosidl_memory_region_t region, \
    size_t size); \
  void STRUCT_NAME ## __fini(STRUCT_NAME * array, size_t size);

/// @brief Define a size-1 array model implementation.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DEFINE(STRUCT_NAME, VALUE_TYPE) \
  bool STRUCT_NAME ## __init(STRUCT_NAME * array, size_t size) \
  { \
    (void)size; \
    if (array == NULL) { \
      return false; \
    } \
    array->_impl.kind = ROSIDL_RUNTIME_C__EXPERIMENTAL__STORAGE_KIND__LOCAL; \
    array->value = (void *)&array->_impl.storage.local; \
    return true; \
  } \
  bool STRUCT_NAME ## __init_from_region(STRUCT_NAME * array, rosidl_memory_region_t region, \
    size_t size) \
  { \
    if (array == NULL || region.location.address == NULL) { \
      return false; \
    } \
    if (size > (SIZE_MAX / sizeof(VALUE_TYPE))) { \
      return false; \
    } \
    if (region.size < (size * sizeof(VALUE_TYPE))) { \
      return false; \
    } \
    array->_impl.kind = ROSIDL_RUNTIME_C__EXPERIMENTAL__STORAGE_KIND__EXTERNAL; \
    array->_impl.storage.region = region; \
    array->value = (void *)region.location.address; \
    return true; \
  } \
  void STRUCT_NAME ## __fini(STRUCT_NAME * array, size_t size) \
  { \
    (void)size; \
    if (array == NULL) { \
      return; \
    } \
    if (array->_impl.kind == ROSIDL_RUNTIME_C__EXPERIMENTAL__STORAGE_KIND__LOCAL) { \
      array->value = (void *)&array->_impl.storage.local; \
      return; \
    } \
    array->value = NULL; \
  }

/// @brief Route array operation for primitive models, else fallback to VALUE_TYPE operation.
#ifndef __cplusplus
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_DISPATCH(VALUE_TYPE, OPERATION, ARRAY_PTR, ...) \
  _Generic(((VALUE_TYPE *)0), \
    float *: rosidl_runtime_c__experimental__FloatArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__FloatArray *)(ARRAY_PTR), __VA_ARGS__), \
    double *: rosidl_runtime_c__experimental__DoubleArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__DoubleArray *)(ARRAY_PTR), __VA_ARGS__), \
    long double *: rosidl_runtime_c__experimental__LongDoubleArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__LongDoubleArray *)(ARRAY_PTR), __VA_ARGS__), \
    char *: rosidl_runtime_c__experimental__CharArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__CharArray *)(ARRAY_PTR), __VA_ARGS__), \
    char16_t * : rosidl_runtime_c__experimental__WCharArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__WCharArray *)(ARRAY_PTR), __VA_ARGS__), \
    bool *: rosidl_runtime_c__experimental__BooleanArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__BooleanArray *)(ARRAY_PTR), __VA_ARGS__), \
    uint8_t * : rosidl_runtime_c__experimental__UInt8Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__UInt8Array *)(ARRAY_PTR), __VA_ARGS__), \
    int8_t * : rosidl_runtime_c__experimental__Int8Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__Int8Array *)(ARRAY_PTR), __VA_ARGS__), \
    uint16_t * : rosidl_runtime_c__experimental__UInt16Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__UInt16Array *)(ARRAY_PTR), __VA_ARGS__), \
    int16_t * : rosidl_runtime_c__experimental__Int16Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__Int16Array *)(ARRAY_PTR), __VA_ARGS__), \
    uint32_t * : rosidl_runtime_c__experimental__UInt32Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__UInt32Array *)(ARRAY_PTR), __VA_ARGS__), \
    int32_t * : rosidl_runtime_c__experimental__Int32Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__Int32Array *)(ARRAY_PTR), __VA_ARGS__), \
    uint64_t * : rosidl_runtime_c__experimental__UInt64Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__UInt64Array *)(ARRAY_PTR), __VA_ARGS__), \
    int64_t * : rosidl_runtime_c__experimental__Int64Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__Int64Array *)(ARRAY_PTR), __VA_ARGS__), \
      default: VALUE_TYPE ## Array__ ## OPERATION((VALUE_TYPE ## Array *)(ARRAY_PTR), __VA_ARGS__) \
  )
#else
/* *INDENT-OFF* */
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_DISPATCH(VALUE_TYPE, OPERATION, ARRAY_PTR, ...) \
  [&] { \
    if constexpr(std::is_same_v<float, VALUE_TYPE>) { \
      return rosidl_runtime_c__experimental__FloatArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__FloatArray *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<double, VALUE_TYPE>) { \
      return rosidl_runtime_c__experimental__DoubleArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__DoubleArray *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<long double, VALUE_TYPE>) { \
      return rosidl_runtime_c__experimental__LongDoubleArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__LongDoubleArray *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<char, VALUE_TYPE>) { \
      return rosidl_runtime_c__experimental__CharArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__CharArray *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<char16_t, VALUE_TYPE>) { \
      return rosidl_runtime_c__experimental__WCharArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__WCharArray *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<bool, VALUE_TYPE>) { \
      return rosidl_runtime_c__experimental__BooleanArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__BooleanArray *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<uint8_t, VALUE_TYPE>) { \
      return rosidl_runtime_c__experimental__UInt8Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__UInt8Array *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<int8_t, VALUE_TYPE>) { \
      return rosidl_runtime_c__experimental__Int8Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__Int8Array *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<uint16_t, VALUE_TYPE>) { \
      return rosidl_runtime_c__experimental__UInt16Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__UInt16Array *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<int16_t, VALUE_TYPE>) { \
      return rosidl_runtime_c__experimental__Int16Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__Int16Array *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<uint32_t, VALUE_TYPE>) { \
      return rosidl_runtime_c__experimental__UInt32Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__UInt32Array *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<int32_t, VALUE_TYPE>) { \
      return rosidl_runtime_c__experimental__Int32Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__Int32Array *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<uint64_t, VALUE_TYPE>) { \
      return rosidl_runtime_c__experimental__UInt64Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__UInt64Array *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<int64_t, VALUE_TYPE>) { \
      return rosidl_runtime_c__experimental__Int64Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__Int64Array *)(ARRAY_PTR), __VA_ARGS__); \
    } else { \
      return VALUE_TYPE ## Array__ ## OPERATION((VALUE_TYPE ## Array *)(ARRAY_PTR), __VA_ARGS__); \
    } \
  }()
/* *INDENT-ON* */
#endif

/// @brief Declare a fixed-size typed array wrapper and function signatures.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_DECLARE(STRUCT_NAME, VALUE_TYPE, SIZE) \
  typedef struct STRUCT_NAME ## _s \
  { \
    struct { \
      VALUE_TYPE data[SIZE]; \
    } * value; \
    struct \
    { \
      rosidl_runtime_c__experimental__storage_kind_t kind; \
      union \
      { \
        rosidl_memory_region_t region; \
        struct \
        { \
          VALUE_TYPE data[SIZE]; \
        } local; \
      } storage; \
    } _impl; \
  } STRUCT_NAME; \
  static inline bool STRUCT_NAME ## __init(STRUCT_NAME * array) \
  { \
    return ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_DISPATCH(VALUE_TYPE, init, array, (SIZE)); \
  } \
  static inline bool STRUCT_NAME ## __init_from_region( \
    STRUCT_NAME * array, \
    rosidl_memory_region_t region) \
  { \
    return ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_DISPATCH( \
      VALUE_TYPE, init_from_region, array, region, (SIZE)); \
  } \
  static inline void STRUCT_NAME ## __fini(STRUCT_NAME * array) \
  { \
    ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_DISPATCH(VALUE_TYPE, fini, array, (SIZE)); \
  }

/// @brief Convenience macro declaring and defining an array in one place.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY(STRUCT_NAME, VALUE_TYPE, SIZE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_DECLARE(STRUCT_NAME, VALUE_TYPE, SIZE)

ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(rosidl_runtime_c__experimental__FloatArray,
  float);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(rosidl_runtime_c__experimental__DoubleArray,
  double);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__LongDoubleArray, long double);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(rosidl_runtime_c__experimental__CharArray,
  char);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(rosidl_runtime_c__experimental__WCharArray,
  char16_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__BooleanArray, bool);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(rosidl_runtime_c__experimental__UInt8Array,
  uint8_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(rosidl_runtime_c__experimental__Int8Array,
  int8_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(rosidl_runtime_c__experimental__UInt16Array,
  uint16_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(rosidl_runtime_c__experimental__Int16Array,
  int16_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(rosidl_runtime_c__experimental__UInt32Array,
  uint32_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(rosidl_runtime_c__experimental__Int32Array,
  int32_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(rosidl_runtime_c__experimental__UInt64Array,
  uint64_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(rosidl_runtime_c__experimental__Int64Array,
  int64_t);

#ifdef __cplusplus
}
#endif

#endif  // ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_H_
