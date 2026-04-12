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

#include "rcutils/allocator.h"
#include "rosidl_runtime_c/experimental/memory.h"
#include "rosidl_runtime_c/experimental/storage.h"
#include "rosidl_runtime_c/experimental/string.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// @file
/// @brief Experimental C11 fixed-size array wrapper macros.

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__CONCAT(x, y) x ## y
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__DISPATCH(name, count) ROSIDL_RUNTIME_C__EXPERIMENTAL__CONCAT(name, count)
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__GET_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, COUNT, ...) COUNT
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__COUNT(...) ROSIDL_RUNTIME_C__EXPERIMENTAL__GET_COUNT(__VA_ARGS__, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

/// @brief Declare a size-1 primitive array model type and function signatures.
/// With a single argument, STRUCT_NAME defaults to ELEMENT_TYPE ## Array.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_STRUCTURE_DECLARE(...) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__DISPATCH( \
    ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_STRUCTURE_DECLARE_, \
    ROSIDL_RUNTIME_C__EXPERIMENTAL__COUNT(__VA_ARGS__))(__VA_ARGS__)

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_STRUCTURE_DECLARE_1(ELEMENT_TYPE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_STRUCTURE_DECLARE_2(ELEMENT_TYPE ## Array, ELEMENT_TYPE)

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_STRUCTURE_DECLARE_2(STRUCT_NAME, ELEMENT_TYPE) \
  typedef struct STRUCT_NAME ## _s \
  { \
    struct { \
      ELEMENT_TYPE data[1U]; \
    } * value; \
    struct \
    { \
      rosidl_runtime_c__experimental__storage_kind_t kind; \
      union \
      { \
        rosidl_memory_region_t region; \
        struct \
        { \
          ELEMENT_TYPE data[1U]; \
        } local; \
      } storage; \
    } _impl; \
  } STRUCT_NAME; \
  bool STRUCT_NAME ## __init(STRUCT_NAME * array, size_t size); \
  bool STRUCT_NAME ## __init_from_region(STRUCT_NAME * array, rosidl_memory_region_t region, \
    size_t size); \
  void STRUCT_NAME ## __fini(STRUCT_NAME * array, size_t size); \
  bool STRUCT_NAME ## __are_equal( \
    const STRUCT_NAME * lhs, const STRUCT_NAME * rhs, size_t size); \
  bool STRUCT_NAME ## __copy( \
    const STRUCT_NAME * input, STRUCT_NAME * output, size_t size);

/// @brief Define a size-1 primitive array model implementation.
/// With a single argument, STRUCT_NAME defaults to ELEMENT_TYPE ## Array.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_STRUCTURE_DEFINE(...) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__DISPATCH( \
    ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_STRUCTURE_DEFINE_, \
    ROSIDL_RUNTIME_C__EXPERIMENTAL__COUNT(__VA_ARGS__))(__VA_ARGS__)

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_STRUCTURE_DEFINE_1(ELEMENT_TYPE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_STRUCTURE_DEFINE_2(ELEMENT_TYPE ## Array, ELEMENT_TYPE)

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_STRUCTURE_DEFINE_2(STRUCT_NAME, ELEMENT_TYPE) \
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
    if (size > (SIZE_MAX / sizeof(ELEMENT_TYPE))) { \
      return false; \
    } \
    if (region.size < (size * sizeof(ELEMENT_TYPE))) { \
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
  } \
  bool STRUCT_NAME ## __are_equal( \
    const STRUCT_NAME * lhs, const STRUCT_NAME * rhs, size_t size) \
  { \
    if (lhs == NULL || rhs == NULL) { \
      return false; \
    } \
    if (lhs->value == NULL || rhs->value == NULL) { \
      return lhs->value == rhs->value; \
    } \
    const ELEMENT_TYPE * _lhs = (const ELEMENT_TYPE *)lhs->value; \
    const ELEMENT_TYPE * _rhs = (const ELEMENT_TYPE *)rhs->value; \
    for (size_t _i = 0U; _i < size; ++_i) { \
      if (_lhs[_i] != _rhs[_i]) { \
        return false; \
      } \
    } \
    return true; \
  } \
  bool STRUCT_NAME ## __copy( \
    const STRUCT_NAME * input, STRUCT_NAME * output, size_t size) \
  { \
    if (input == NULL || output == NULL) { \
      return false; \
    } \
    if (input->value == NULL || output->value == NULL) { \
      return input->value == output->value; \
    } \
    const ELEMENT_TYPE * _src = (const ELEMENT_TYPE *)input->value; \
    ELEMENT_TYPE * _dst = (ELEMENT_TYPE *)output->value; \
    for (size_t _i = 0U; _i < size; ++_i) { \
      _dst[_i] = _src[_i]; \
    } \
    return true; \
  }

/// @brief Declare a size-1 object array model type and function signatures.
/// Layout-compatible with any ELEMENT_TYPE data[N] array for N >= 1, allowing
/// a single set of functions to serve all fixed sizes via pointer cast.
/// With a single argument, STRUCT_NAME defaults to ELEMENT_TYPE ## Array,
/// which satisfies the naming convention required by ARRAY_DISPATCH.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(...) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__DISPATCH( \
    ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE_, \
    ROSIDL_RUNTIME_C__EXPERIMENTAL__COUNT(__VA_ARGS__))(__VA_ARGS__)

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE_1(ELEMENT_TYPE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE_2(ELEMENT_TYPE ## Array, ELEMENT_TYPE)

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE_2(STRUCT_NAME, ELEMENT_TYPE) \
  typedef struct STRUCT_NAME ## _s \
  { \
    struct { \
      ELEMENT_TYPE data[1U]; \
    } * value; \
    struct \
    { \
      rosidl_runtime_c__experimental__storage_kind_t kind; \
      union \
      { \
        rosidl_memory_region_t region; \
        struct \
        { \
          ELEMENT_TYPE data[1U]; \
        } local; \
      } storage; \
    } _impl; \
  } STRUCT_NAME; \
  bool STRUCT_NAME ## __init(STRUCT_NAME * array, size_t size); \
  bool STRUCT_NAME ## __init_with_allocator( \
    STRUCT_NAME * array, size_t size, const rcutils_allocator_t * allocator); \
  bool STRUCT_NAME ## __init_from_region(STRUCT_NAME * array, rosidl_memory_region_t region, \
    size_t size); \
  void STRUCT_NAME ## __fini(STRUCT_NAME * array, size_t size); \
  bool STRUCT_NAME ## __are_equal( \
    const STRUCT_NAME * lhs, const STRUCT_NAME * rhs, size_t size); \
  bool STRUCT_NAME ## __copy( \
    const STRUCT_NAME * input, STRUCT_NAME * output, size_t size);

/// @brief Define a size-1 object array model implementation.
/// With a single argument, STRUCT_NAME defaults to ELEMENT_TYPE ## Array.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DEFINE(...) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__DISPATCH( \
    ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DEFINE_, \
    ROSIDL_RUNTIME_C__EXPERIMENTAL__COUNT(__VA_ARGS__))(__VA_ARGS__)

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DEFINE_1(ELEMENT_TYPE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DEFINE_2(ELEMENT_TYPE ## Array, ELEMENT_TYPE)

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DEFINE_2(STRUCT_NAME, ELEMENT_TYPE) \
  bool STRUCT_NAME ## __init_with_allocator( \
    STRUCT_NAME * array, size_t size, const rcutils_allocator_t * allocator) \
  { \
    if (array == NULL) { \
      return false; \
    } \
    ELEMENT_TYPE * _data = (ELEMENT_TYPE *)&array->_impl.storage.local; \
    for (size_t _i = 0U; _i < size; ++_i) { \
      if (!ELEMENT_TYPE ## __init_with_allocator(&_data[_i], allocator)) { \
        for (; _i-- > 0U; ) { \
          ELEMENT_TYPE ## __fini(&_data[_i]); \
        } \
        return false; \
      } \
    } \
    array->_impl.kind = ROSIDL_RUNTIME_C__EXPERIMENTAL__STORAGE_KIND__LOCAL; \
    array->value = (void *)&array->_impl.storage.local; \
    return true; \
  } \
  bool STRUCT_NAME ## __init(STRUCT_NAME * array, size_t size) \
  { \
    return STRUCT_NAME ## __init_with_allocator(array, size, NULL); \
  } \
  bool STRUCT_NAME ## __init_from_region( \
    STRUCT_NAME * array, rosidl_memory_region_t region, size_t size) \
  { \
    if (array == NULL || region.location.address == NULL) { \
      return false; \
    } \
    if (size > (SIZE_MAX / sizeof(ELEMENT_TYPE))) { \
      return false; \
    } \
    if (region.size < (size * sizeof(ELEMENT_TYPE))) { \
      return false; \
    } \
    ELEMENT_TYPE * _data = (ELEMENT_TYPE *)region.location.address; \
    for (size_t _i = 0U; _i < size; ++_i) { \
      if (!ELEMENT_TYPE ## __init(&_data[_i])) { \
        for (; _i-- > 0U; ) { \
          ELEMENT_TYPE ## __fini(&_data[_i]); \
        } \
        return false; \
      } \
    } \
    array->_impl.kind = ROSIDL_RUNTIME_C__EXPERIMENTAL__STORAGE_KIND__EXTERNAL; \
    array->_impl.storage.region = region; \
    array->value = (void *)region.location.address; \
    return true; \
  } \
  void STRUCT_NAME ## __fini(STRUCT_NAME * array, size_t size) \
  { \
    if (array == NULL) { \
      return; \
    } \
    if (array->value != NULL) { \
      ELEMENT_TYPE * _data = (ELEMENT_TYPE *)array->value; \
      for (size_t _i = 0U; _i < size; ++_i) { \
        ELEMENT_TYPE ## __fini(&_data[_i]); \
      } \
    } \
    if (array->_impl.kind == ROSIDL_RUNTIME_C__EXPERIMENTAL__STORAGE_KIND__LOCAL) { \
      array->value = (void *)&array->_impl.storage.local; \
      return; \
    } \
    array->value = NULL; \
  } \
  bool STRUCT_NAME ## __are_equal( \
    const STRUCT_NAME * lhs, const STRUCT_NAME * rhs, size_t size) \
  { \
    if (lhs == NULL || rhs == NULL) { \
      return false; \
    } \
    if (lhs->value == NULL || rhs->value == NULL) { \
      return lhs->value == rhs->value; \
    } \
    const ELEMENT_TYPE * _lhs = (const ELEMENT_TYPE *)lhs->value; \
    const ELEMENT_TYPE * _rhs = (const ELEMENT_TYPE *)rhs->value; \
    for (size_t _i = 0U; _i < size; ++_i) { \
      if (!ELEMENT_TYPE ## __are_equal(&_lhs[_i], &_rhs[_i])) { \
        return false; \
      } \
    } \
    return true; \
  } \
  bool STRUCT_NAME ## __copy( \
    const STRUCT_NAME * input, STRUCT_NAME * output, size_t size) \
  { \
    if (input == NULL || output == NULL) { \
      return false; \
    } \
    if (input->value == NULL || output->value == NULL) { \
      return input->value == output->value; \
    } \
    const ELEMENT_TYPE * _src = (const ELEMENT_TYPE *)input->value; \
    ELEMENT_TYPE * _dst = (ELEMENT_TYPE *)output->value; \
    for (size_t _i = 0U; _i < size; ++_i) { \
      if (!ELEMENT_TYPE ## __copy(&_src[_i], &_dst[_i])) { \
        return false; \
      } \
    } \
    return true; \
  }

/// @brief Route array operation for primitive models, else fallback to ELEMENT_TYPE operation.
#ifndef __cplusplus

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_0(TYPE, CASE_EXPRESSION) \
  _Generic(((TYPE)0), \
    CASE_EXPRESSION, \
    default: void)

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_1(TYPE, CASE_EXPRESSION, ...) \
  _Generic(((TYPE)0), \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_0(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_2(TYPE, CASE_EXPRESSION, ...) \
  _Generic(((TYPE)0), \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_1(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_3(TYPE, CASE_EXPRESSION, ...) \
  _Generic(((TYPE)0), \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_2(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_4(TYPE, CASE_EXPRESSION, ...) \
  _Generic((TYPE)0, \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_3(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_5(TYPE, CASE_EXPRESSION, ...) \
  _Generic((TYPE)0, \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_4(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_6(TYPE, CASE_EXPRESSION, ...) \
  _Generic((TYPE)0, \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_5(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_7(TYPE, CASE_EXPRESSION, ...) \
  _Generic((TYPE)0, \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_6(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_8(TYPE, CASE_EXPRESSION, ...) \
  _Generic((TYPE)0, \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_7(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_9(TYPE, CASE_EXPRESSION, ...) \
  _Generic((TYPE)0, \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_8(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_10(TYPE, CASE_EXPRESSION, ...) \
  _Generic((TYPE)0, \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_9(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_11(TYPE, CASE_EXPRESSION, ...) \
  _Generic((TYPE)0, \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_10(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_11(TYPE, CASE_EXPRESSION, ...) \
  _Generic((TYPE)0, \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_10(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_12(TYPE, CASE_EXPRESSION, ...) \
  _Generic((TYPE)0, \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_11(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_13(TYPE, CASE_EXPRESSION, ...) \
  _Generic((TYPE)0, \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_12(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_14(TYPE, CASE_EXPRESSION, ...) \
  _Generic((TYPE)0, \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_13(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_15(TYPE, CASE_EXPRESSION, ...) \
  _Generic((TYPE)0, \
    CASE_EXPRESSION, \
    default: ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_14(TYPE, __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH(TYPE, ...) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__DISPATCH( \
    ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH_, \
    ROSIDL_RUNTIME_C__EXPERIMENTAL__COUNT(__VA_ARGS__))(TYPE, __VA_ARGS__)

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_DISPATCH_1(ELEMENT_TYPE, OPERATION, ARRAY_PTR, ...) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH( \
    ELEMENT_TYPE, \
    float: rosidl_runtime_c__experimental__FloatArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__FloatArray *)(ARRAY_PTR), __VA_ARGS__), \
    double: rosidl_runtime_c__experimental__DoubleArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__DoubleArray *)(ARRAY_PTR), __VA_ARGS__), \
    long double: rosidl_runtime_c__experimental__LongDoubleArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__LongDoubleArray *)(ARRAY_PTR), __VA_ARGS__), \
    char: rosidl_runtime_c__experimental__CharArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__CharArray *)(ARRAY_PTR), __VA_ARGS__), \
    char16_t: rosidl_runtime_c__experimental__WCharArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__WCharArray *)(ARRAY_PTR), __VA_ARGS__), \
    bool: rosidl_runtime_c__experimental__BooleanArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__BooleanArray *)(ARRAY_PTR), __VA_ARGS__), \
    uint8_t: rosidl_runtime_c__experimental__UInt8Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__UInt8Array *)(ARRAY_PTR), __VA_ARGS__), \
    int8_t: rosidl_runtime_c__experimental__Int8Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__Int8Array *)(ARRAY_PTR), __VA_ARGS__), \
    uint16_t: rosidl_runtime_c__experimental__UInt16Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__UInt16Array *)(ARRAY_PTR), __VA_ARGS__), \
    int16_t: rosidl_runtime_c__experimental__Int16Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__Int16Array *)(ARRAY_PTR), __VA_ARGS__), \
    uint32_t: rosidl_runtime_c__experimental__UInt32Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__UInt32Array *)(ARRAY_PTR), __VA_ARGS__), \
    int32_t: rosidl_runtime_c__experimental__Int32Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__Int32Array *)(ARRAY_PTR), __VA_ARGS__), \
    uint64_t: rosidl_runtime_c__experimental__UInt64Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__UInt64Array *)(ARRAY_PTR), __VA_ARGS__), \
    int64_t: rosidl_runtime_c__experimental__Int64Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__Int64Array *)(ARRAY_PTR), __VA_ARGS__))

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_DISPATCH_2(ELEMENT_TYPE, OPERATION, LHS_ARRAY_PTR, RHS_ARRAY_PTR, ...) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__TYPE_SWITCH( \
    ELEMENT_TYPE, \
    float: rosidl_runtime_c__experimental__FloatArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__FloatArray *)(LHS_ARRAY_PTR), \
      (rosidl_runtime_c__experimental__FloatArray *)(RHS_ARRAY_PTR), __VA_ARGS__), \
    double: rosidl_runtime_c__experimental__DoubleArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__DoubleArray *)(LHS_ARRAY_PTR), \
      (rosidl_runtime_c__experimental__DoubleArray *)(RHS_ARRAY_PTR), __VA_ARGS__), \
    long double: rosidl_runtime_c__experimental__LongDoubleArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__LongDoubleArray *)(LHS_ARRAY_PTR), \
      (rosidl_runtime_c__experimental__LongDoubleArray *)(RHS_ARRAY_PTR), __VA_ARGS__), \
    char: rosidl_runtime_c__experimental__CharArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__CharArray *)(LHS_ARRAY_PTR), \
      (rosidl_runtime_c__experimental__CharArray *)(RHS_ARRAY_PTR), __VA_ARGS__), \
    char16_t: rosidl_runtime_c__experimental__WCharArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__WCharArray *)(LHS_ARRAY_PTR), \
      (rosidl_runtime_c__experimental__WCharArray *)(RHS_ARRAY_PTR), __VA_ARGS__), \
    bool: rosidl_runtime_c__experimental__BooleanArray__ ## OPERATION( \
      (rosidl_runtime_c__experimental__BooleanArray *)(LHS_ARRAY_PTR), \
      (rosidl_runtime_c__experimental__BooleanArray *)(RHS_ARRAY_PTR), __VA_ARGS__), \
    uint8_t: rosidl_runtime_c__experimental__UInt8Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__UInt8Array *)(LHS_ARRAY_PTR), \
      (rosidl_runtime_c__experimental__UInt8Array *)(RHS_ARRAY_PTR), __VA_ARGS__), \
    int8_t: rosidl_runtime_c__experimental__Int8Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__Int8Array *)(LHS_ARRAY_PTR), \
      (rosidl_runtime_c__experimental__Int8Array *)(RHS_ARRAY_PTR), __VA_ARGS__), \
    uint16_t: rosidl_runtime_c__experimental__UInt16Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__UInt16Array *)(LHS_ARRAY_PTR), \
      (rosidl_runtime_c__experimental__UInt16Array *)(RHS_ARRAY_PTR), __VA_ARGS__), \
    int16_t: rosidl_runtime_c__experimental__Int16Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__Int16Array *)(LHS_ARRAY_PTR), \
      (rosidl_runtime_c__experimental__Int16Array *)(RHS_ARRAY_PTR), __VA_ARGS__), \
    uint32_t: rosidl_runtime_c__experimental__UInt32Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__UInt32Array *)(LHS_ARRAY_PTR), \
      (rosidl_runtime_c__experimental__UInt32Array *)(RHS_ARRAY_PTR), __VA_ARGS__), \
    int32_t: rosidl_runtime_c__experimental__Int32Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__Int32Array *)(LHS_ARRAY_PTR), \
      (rosidl_runtime_c__experimental__Int32Array *)(RHS_ARRAY_PTR), __VA_ARGS__), \
    uint64_t: rosidl_runtime_c__experimental__UInt64Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__UInt64Array *)(LHS_ARRAY_PTR), \
      (rosidl_runtime_c__experimental__UInt64Array *)(RHS_ARRAY_PTR), __VA_ARGS__), \
    int64_t: rosidl_runtime_c__experimental__Int64Array__ ## OPERATION( \
      (rosidl_runtime_c__experimental__Int64Array *)(LHS_ARRAY_PTR), \
      (rosidl_runtime_c__experimental__Int64Array *)(RHS_ARRAY_PTR), __VA_ARGS__))

#else
/* *INDENT-OFF* */
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_DISPATCH_1(ELEMENT_TYPE, OPERATION, ARRAY_PTR, ...) \
  [&] { \
    if constexpr(std::is_same_v<float, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__FloatArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__FloatArray *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<double, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__DoubleArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__DoubleArray *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<long double, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__LongDoubleArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__LongDoubleArray *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<char, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__CharArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__CharArray *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<char16_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__WCharArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__WCharArray *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<bool, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__BooleanArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__BooleanArray *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<uint8_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__UInt8Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__UInt8Array *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<int8_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__Int8Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__Int8Array *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<uint16_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__UInt16Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__UInt16Array *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<int16_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__Int16Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__Int16Array *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<uint32_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__UInt32Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__UInt32Array *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<int32_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__Int32Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__Int32Array *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<uint64_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__UInt64Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__UInt64Array *)(ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<int64_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__Int64Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__Int64Array *)(ARRAY_PTR), __VA_ARGS__); \
    } \
  }()

#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_DISPATCH_2(ELEMENT_TYPE, OPERATION, LHS_ARRAY_PTR, RHS_ARRAY_PTR, ...) \
  [&] { \
    if constexpr(std::is_same_v<float, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__FloatArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__FloatArray *)(LHS_ARRAY_PTR), \
        (rosidl_runtime_c__experimental__FloatArray *)(RHS_ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<double, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__DoubleArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__DoubleArray *)(LHS_ARRAY_PTR), \
        (rosidl_runtime_c__experimental__DoubleArray *)(RHS_ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<long double, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__LongDoubleArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__LongDoubleArray *)(LHS_ARRAY_PTR), \
        (rosidl_runtime_c__experimental__LongDoubleArray *)(RHS_ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<char, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__CharArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__CharArray *)(LHS_ARRAY_PTR), \
        (rosidl_runtime_c__experimental__CharArray *)(RHS_ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<char16_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__WCharArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__WCharArray *)(LHS_ARRAY_PTR), \
        (rosidl_runtime_c__experimental__WCharArray *)(RHS_ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<bool, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__BooleanArray__ ## OPERATION( \
        (rosidl_runtime_c__experimental__BooleanArray *)(LHS_ARRAY_PTR), \
        (rosidl_runtime_c__experimental__BooleanArray *)(RHS_ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<uint8_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__UInt8Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__UInt8Array *)(LHS_ARRAY_PTR), \
        (rosidl_runtime_c__experimental__UInt8Array *)(RHS_ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<int8_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__Int8Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__Int8Array *)(LHS_ARRAY_PTR), \
        (rosidl_runtime_c__experimental__Int8Array *)(RHS_ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<uint16_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__UInt16Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__UInt16Array *)(LHS_ARRAY_PTR), \
        (rosidl_runtime_c__experimental__UInt16Array *)(RHS_ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<int16_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__Int16Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__Int16Array *)(LHS_ARRAY_PTR), \
        (rosidl_runtime_c__experimental__Int16Array *)(RHS_ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<uint32_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__UInt32Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__UInt32Array *)(LHS_ARRAY_PTR), \
        (rosidl_runtime_c__experimental__UInt32Array *)(RHS_ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<int32_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__Int32Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__Int32Array *)(LHS_ARRAY_PTR), \
        (rosidl_runtime_c__experimental__Int32Array *)(RHS_ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<uint64_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__UInt64Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__UInt64Array *)(LHS_ARRAY_PTR), \
        (rosidl_runtime_c__experimental__UInt64Array *)(RHS_ARRAY_PTR), __VA_ARGS__); \
    } else if constexpr(std::is_same_v<int64_t, ELEMENT_TYPE>) { \
      return rosidl_runtime_c__experimental__Int64Array__ ## OPERATION( \
        (rosidl_runtime_c__experimental__Int64Array *)(LHS_ARRAY_PTR), \
        (rosidl_runtime_c__experimental__Int64Array *)(RHS_ARRAY_PTR), __VA_ARGS__); \
    } \
  }()

/* *INDENT-ON* */
#endif

/// @brief Declare a fixed-size typed array of primitives, dispatching to the
/// appropriate primitive array structure implementation.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_DECLARE(STRUCT_NAME, ELEMENT_TYPE, SIZE) \
  typedef struct STRUCT_NAME ## _s \
  { \
    struct { \
      ELEMENT_TYPE data[SIZE]; \
    } * value; \
    struct \
    { \
      rosidl_runtime_c__experimental__storage_kind_t kind; \
      union \
      { \
        rosidl_memory_region_t region; \
        struct \
        { \
          ELEMENT_TYPE data[SIZE]; \
        } local; \
      } storage; \
    } _impl; \
  } STRUCT_NAME; \
  static inline bool STRUCT_NAME ## __init(STRUCT_NAME * array) \
  { \
    return ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_DISPATCH_1(ELEMENT_TYPE, init, array, SIZE); \
  } \
  static inline bool STRUCT_NAME ## __init_from_region( \
    STRUCT_NAME * array, \
    rosidl_memory_region_t region) \
  { \
    return ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_DISPATCH_1( \
      ELEMENT_TYPE, init_from_region, array, region, SIZE); \
  } \
  static inline void STRUCT_NAME ## __fini(STRUCT_NAME * array) \
  { \
    ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_DISPATCH_1(ELEMENT_TYPE, fini, array, SIZE); \
  } \
  static inline bool STRUCT_NAME ## __init_with_allocator( \
    STRUCT_NAME * array, \
    const rcutils_allocator_t * allocator) \
  { \
    (void)allocator; \
    return STRUCT_NAME ## __init(array); \
  } \
  static inline bool STRUCT_NAME ## __are_equal( \
    const STRUCT_NAME * lhs, const STRUCT_NAME * rhs) \
  { \
    return ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_DISPATCH_2(ELEMENT_TYPE, are_equal, lhs, rhs, SIZE); \
  } \
  static inline bool STRUCT_NAME ## __copy( \
    const STRUCT_NAME * input, STRUCT_NAME * output) \
  { \
    return ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_DISPATCH_2(ELEMENT_TYPE, copy, input, output, SIZE); \
  }

/// @brief Convenience macro declaring and defining a primitive array in one place.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY(STRUCT_NAME, ELEMENT_TYPE, SIZE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_DECLARE(STRUCT_NAME, ELEMENT_TYPE, SIZE)

/// @brief Dispatch a fixed-size array operation for object element types to
/// ELEMENT_TYPE ## Array__ ## OPERATION.
/// Requires ELEMENT_TYPE ## Array to be declared with ARRAY_STRUCTURE_DECLARE,
/// following the ELEMENT_TYPE ## Array naming convention.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_DISPATCH(ELEMENT_TYPE, OPERATION, ARRAY_PTR, ...) \
  ELEMENT_TYPE ## Array__ ## OPERATION((ELEMENT_TYPE ## Array *)(ARRAY_PTR), __VA_ARGS__)

/// @brief Declare a fixed-size typed array of objects, dispatching to
/// ELEMENT_TYPE ## Array structure operations.
/// Requires ELEMENT_TYPE ## Array to be declared with ARRAY_STRUCTURE_DECLARE.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_DECLARE(STRUCT_NAME, ELEMENT_TYPE, SIZE) \
  typedef struct STRUCT_NAME ## _s \
  { \
    struct { \
      ELEMENT_TYPE data[SIZE]; \
    } * value; \
    struct \
    { \
      rosidl_runtime_c__experimental__storage_kind_t kind; \
      union \
      { \
        rosidl_memory_region_t region; \
        struct \
        { \
          ELEMENT_TYPE data[SIZE]; \
        } local; \
      } storage; \
    } _impl; \
  } STRUCT_NAME; \
  static inline bool STRUCT_NAME ## __init(STRUCT_NAME * array) \
  { \
    return ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_DISPATCH( \
      ELEMENT_TYPE, init, array, (SIZE)); \
  } \
  static inline bool STRUCT_NAME ## __init_with_allocator( \
    STRUCT_NAME * array, \
    const rcutils_allocator_t * allocator) \
  { \
    return ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_DISPATCH( \
      ELEMENT_TYPE, init_with_allocator, array, (SIZE), allocator); \
  } \
  static inline bool STRUCT_NAME ## __init_from_region( \
    STRUCT_NAME * array, \
    rosidl_memory_region_t region) \
  { \
    return ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_DISPATCH( \
      ELEMENT_TYPE, init_from_region, array, region, (SIZE)); \
  } \
  static inline void STRUCT_NAME ## __fini(STRUCT_NAME * array) \
  { \
    ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_DISPATCH( \
      ELEMENT_TYPE, fini, array, (SIZE)); \
  } \
  static inline bool STRUCT_NAME ## __are_equal( \
    const STRUCT_NAME * lhs, const STRUCT_NAME * rhs) \
  { \
    return ELEMENT_TYPE ## Array__are_equal( \
      (const ELEMENT_TYPE ## Array *)(lhs), \
      (const ELEMENT_TYPE ## Array *)(rhs), (SIZE)); \
  } \
  static inline bool STRUCT_NAME ## __copy( \
    const STRUCT_NAME * input, STRUCT_NAME * output) \
  { \
    return ELEMENT_TYPE ## Array__copy( \
      (const ELEMENT_TYPE ## Array *)(input), \
      (ELEMENT_TYPE ## Array *)(output), (SIZE)); \
  }

/// @brief Convenience macro declaring and defining an array in one place.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY(STRUCT_NAME, ELEMENT_TYPE, SIZE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_DECLARE(STRUCT_NAME, ELEMENT_TYPE, SIZE)

ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__FloatArray, float);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__DoubleArray, double);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__LongDoubleArray, long double);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__CharArray, char);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__WCharArray, char16_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__BooleanArray, bool);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__UInt8Array, uint8_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__Int8Array, int8_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__UInt16Array, uint16_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__Int16Array, int16_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__UInt32Array, uint32_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__Int32Array, int32_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__UInt64Array, uint64_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__Int64Array, int64_t);

ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__String);
ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_STRUCTURE_DECLARE(
  rosidl_runtime_c__experimental__WString);

#ifdef __cplusplus
}
#endif

#endif  // ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_H_
