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

#ifndef ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_H_
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <uchar.h>

#include "rcutils/allocator.h"
#include "rosidl_runtime_c/experimental/detail/value_helpers.h"
#include "rosidl_runtime_c/experimental/memory.h"
#include "rosidl_runtime_c/experimental/storage.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// @file
/// @brief Experimental C11 sequence wrapper macros.

/// @brief Declare a bounded dynamic typed sequence wrapper and function signatures.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE_DECLARE(STRUCT_NAME, VALUE_TYPE, \
    UPPER_BOUND) \
  typedef struct STRUCT_NAME ## _s \
  { \
    VALUE_TYPE * value; \
    size_t size; \
    size_t capacity; \
    struct \
    { \
      rosidl_runtime_c__experimental__storage_kind_t kind; \
      union \
      { \
        rosidl_memory_region_t region; \
        VALUE_TYPE * data; \
      } storage; \
      rcutils_allocator_t allocator; \
    } _impl; \
  } STRUCT_NAME; \
  bool STRUCT_NAME ## __init( \
    STRUCT_NAME * _sequence, \
    const rcutils_allocator_t * allocator); \
  bool STRUCT_NAME ## __init_from_region( \
    STRUCT_NAME * _sequence, \
    rosidl_memory_region_t region); \
  void STRUCT_NAME ## __fini( \
    STRUCT_NAME * _sequence); \
  bool STRUCT_NAME ## __reserve( \
    STRUCT_NAME * _sequence, \
    size_t requested_capacity); \
  bool STRUCT_NAME ## __resize( \
    STRUCT_NAME * _sequence, \
    size_t new_size); \
  bool STRUCT_NAME ## __push_back( \
    STRUCT_NAME * _sequence, \
    VALUE_TYPE value); \
  bool STRUCT_NAME ## __are_equal( \
    const STRUCT_NAME * lhs, \
    const STRUCT_NAME * rhs); \
  bool STRUCT_NAME ## __copy( \
    const STRUCT_NAME * input, \
    STRUCT_NAME * output);

/// @brief Declare a dynamic typed sequence wrapper and function signatures.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(STRUCT_NAME, VALUE_TYPE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE_DECLARE(STRUCT_NAME, VALUE_TYPE, 0U)

/// @brief Define dynamic typed sequence functions.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE_DEFINE(STRUCT_NAME, VALUE_TYPE, \
    UPPER_BOUND) \
  bool STRUCT_NAME ## __init( \
    STRUCT_NAME * _sequence, \
    const rcutils_allocator_t * allocator) \
  { \
    if (_sequence == NULL) { \
      return false; \
    } \
    _sequence->value = NULL; \
    _sequence->size = 0U; \
    _sequence->capacity = 0U; \
    _sequence->_impl.kind = ROSIDL_RUNTIME_C__EXPERIMENTAL__STORAGE_KIND__MANAGED; \
    _sequence->_impl.storage.data = NULL; \
    _sequence->_impl.allocator = \
      rosidl_runtime_c__experimental__detail__allocator_or_default(allocator); \
    return rcutils_allocator_is_valid(&_sequence->_impl.allocator); \
  } \
  bool STRUCT_NAME ## __init_from_region( \
    STRUCT_NAME * _sequence, \
    rosidl_memory_region_t region) \
  { \
    if (_sequence == NULL || region.location.address == NULL) { \
      return false; \
    } \
    _sequence->_impl.kind = ROSIDL_RUNTIME_C__EXPERIMENTAL__STORAGE_KIND__EXTERNAL; \
    _sequence->_impl.storage.region = region; \
    _sequence->_impl.allocator = rcutils_get_default_allocator(); \
    _sequence->value = (VALUE_TYPE *)region.location.address; \
    _sequence->size = 0U; \
    _sequence->capacity = region.size / sizeof(VALUE_TYPE); \
    if ((UPPER_BOUND) > 0U && _sequence->capacity > (size_t)(UPPER_BOUND)) { \
      _sequence->capacity = (size_t)(UPPER_BOUND); \
    } \
    return true; \
  } \
  void STRUCT_NAME ## __fini( \
    STRUCT_NAME * _sequence) \
  { \
    if (_sequence == NULL) { \
      return; \
    } \
    if (_sequence->_impl.kind == ROSIDL_RUNTIME_C__EXPERIMENTAL__STORAGE_KIND__MANAGED && \
      _sequence->_impl.storage.data != NULL) \
    { \
      _sequence->_impl.allocator.deallocate(_sequence->_impl.storage.data, \
        _sequence->_impl.allocator.state); \
    } \
    _sequence->value = NULL; \
    _sequence->size = 0U; \
    _sequence->capacity = 0U; \
    _sequence->_impl.storage.data = NULL; \
  } \
  bool STRUCT_NAME ## __reserve( \
    STRUCT_NAME * _sequence, \
    size_t requested_capacity) \
  { \
    if (_sequence == NULL) { \
      return false; \
    } \
    if ((UPPER_BOUND) > 0U && requested_capacity > (size_t)(UPPER_BOUND)) { \
      return false; \
    } \
    if (requested_capacity <= _sequence->capacity) { \
      return true; \
    } \
    if (_sequence->_impl.kind != ROSIDL_RUNTIME_C__EXPERIMENTAL__STORAGE_KIND__MANAGED) { \
      return false; \
    } \
    size_t new_capacity = rosidl_runtime_c__experimental__detail__next_capacity( \
      _sequence->capacity, requested_capacity); \
    if ((UPPER_BOUND) > 0U && new_capacity > (size_t)(UPPER_BOUND)) { \
      new_capacity = (size_t)(UPPER_BOUND); \
    } \
    size_t new_bytes = 0U; \
    if (!rosidl_runtime_c__experimental__detail__compute_bytes(new_capacity, sizeof(VALUE_TYPE), \
      &new_bytes)) { \
      return false; \
    } \
    void * reallocated = _sequence->_impl.allocator.reallocate( \
      _sequence->_impl.storage.data, new_bytes, _sequence->_impl.allocator.state); \
    if (reallocated == NULL) { \
      return false; \
    } \
    _sequence->_impl.storage.data = (VALUE_TYPE *)reallocated; \
    _sequence->value = _sequence->_impl.storage.data; \
    _sequence->capacity = new_capacity; \
    return true; \
  } \
  bool STRUCT_NAME ## __resize( \
    STRUCT_NAME * _sequence, \
    size_t new_size) \
  { \
    if (_sequence == NULL) { \
      return false; \
    } \
    if (new_size > _sequence->capacity && \
      !STRUCT_NAME ## __reserve(_sequence, new_size)) \
    { \
      return false; \
    } \
    if (new_size > _sequence->size) { \
      (void)memset(&_sequence->value[_sequence->size], 0, \
        (new_size - _sequence->size) * sizeof(VALUE_TYPE)); \
    } \
    _sequence->size = new_size; \
    return true; \
  } \
  bool STRUCT_NAME ## __push_back( \
    STRUCT_NAME * _sequence, \
    VALUE_TYPE value) \
  { \
    if (_sequence == NULL) { \
      return false; \
    } \
    if (_sequence->size == _sequence->capacity && \
      !STRUCT_NAME ## __reserve(_sequence, _sequence->size + 1U)) \
    { \
      return false; \
    } \
    _sequence->value[_sequence->size] = value; \
    ++_sequence->size; \
    return true; \
  } \
  bool STRUCT_NAME ## __are_equal( \
    const STRUCT_NAME * lhs, \
    const STRUCT_NAME * rhs) \
  { \
    if (lhs == NULL || rhs == NULL) { \
      return false; \
    } \
    if (lhs->size != rhs->size) { \
      return false; \
    } \
    if (lhs->size == 0U) { \
      return true; \
    } \
    if (lhs->value == NULL || rhs->value == NULL) { \
      return false; \
    } \
    return memcmp(lhs->value, rhs->value, lhs->size * sizeof(VALUE_TYPE)) == 0; \
  } \
  bool STRUCT_NAME ## __copy( \
    const STRUCT_NAME * input, \
    STRUCT_NAME * output) \
  { \
    if (input == NULL || output == NULL) { \
      return false; \
    } \
    if (input->size > 0U && input->value == NULL) { \
      return false; \
    } \
    if (!STRUCT_NAME ## __resize(output, input->size)) { \
      return false; \
    } \
    if (input->size > 0U) { \
      (void)memcpy(output->value, input->value, input->size * sizeof(VALUE_TYPE)); \
    } \
    return true; \
  }

/// @brief Define sequence functions declared with ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DEFINE(STRUCT_NAME, VALUE_TYPE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE_DEFINE(STRUCT_NAME, VALUE_TYPE, 0U)

/// @brief Convenience macro declaring and defining a bounded sequence in one place.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE(STRUCT_NAME, VALUE_TYPE, UPPER_BOUND) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE_DECLARE(STRUCT_NAME, VALUE_TYPE, UPPER_BOUND); \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE_DEFINE(STRUCT_NAME, VALUE_TYPE, UPPER_BOUND)

/// @brief Convenience macro declaring and defining a sequence in one place.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE(STRUCT_NAME, VALUE_TYPE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(STRUCT_NAME, VALUE_TYPE); \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DEFINE(STRUCT_NAME, VALUE_TYPE)

// Sequence types for all primitive ROSIDL C types.
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(rosidl_runtime_c__experimental__FloatSequence,
  float);
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(rosidl_runtime_c__experimental__DoubleSequence,
  double);
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(rosidl_runtime_c__experimental__LongDoubleSequence,
  long double);
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(rosidl_runtime_c__experimental__CharSequence,
  char);
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(rosidl_runtime_c__experimental__WCharSequence,
  char16_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(rosidl_runtime_c__experimental__BooleanSequence,
  bool);
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(rosidl_runtime_c__experimental__OctetSequence,
  uint8_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(rosidl_runtime_c__experimental__UInt8Sequence,
  uint8_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(rosidl_runtime_c__experimental__Int8Sequence,
  int8_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(rosidl_runtime_c__experimental__UInt16Sequence,
  uint16_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(rosidl_runtime_c__experimental__Int16Sequence,
  int16_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(rosidl_runtime_c__experimental__UInt32Sequence,
  uint32_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(rosidl_runtime_c__experimental__Int32Sequence,
  int32_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(rosidl_runtime_c__experimental__UInt64Sequence,
  uint64_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(rosidl_runtime_c__experimental__Int64Sequence,
  int64_t);

#ifdef __cplusplus
}
#endif

#endif  // ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_H_
