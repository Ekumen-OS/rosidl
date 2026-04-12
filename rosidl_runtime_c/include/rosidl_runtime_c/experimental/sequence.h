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
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_BOUNDED_SEQUENCE_DECLARE(STRUCT_NAME, VALUE_TYPE, \
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
    STRUCT_NAME * _sequence); \
  bool STRUCT_NAME ## __init_with_allocator( \
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
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(STRUCT_NAME, VALUE_TYPE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_BOUNDED_SEQUENCE_DECLARE(STRUCT_NAME, VALUE_TYPE, 0U)

/// @brief Define dynamic typed sequence functions.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_BOUNDED_SEQUENCE_DEFINE(STRUCT_NAME, VALUE_TYPE, \
    UPPER_BOUND) \
  bool STRUCT_NAME ## __init_with_allocator( \
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
  bool STRUCT_NAME ## __init( \
    STRUCT_NAME * _sequence) \
  { \
    return STRUCT_NAME ## __init_with_allocator(_sequence, NULL); \
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

/// @brief Define sequence functions declared with ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DEFINE(STRUCT_NAME, VALUE_TYPE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_BOUNDED_SEQUENCE_DEFINE(STRUCT_NAME, VALUE_TYPE, 0U)

/// @brief Convenience macro declaring and defining a bounded sequence in one place.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_BOUNDED_SEQUENCE(STRUCT_NAME, VALUE_TYPE, UPPER_BOUND) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_BOUNDED_SEQUENCE_DECLARE(STRUCT_NAME, VALUE_TYPE, UPPER_BOUND); \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_BOUNDED_SEQUENCE_DEFINE(STRUCT_NAME, VALUE_TYPE, UPPER_BOUND)

/// @brief Convenience macro declaring and defining a sequence in one place.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE(STRUCT_NAME, VALUE_TYPE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(STRUCT_NAME, VALUE_TYPE); \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DEFINE(STRUCT_NAME, VALUE_TYPE)

/// @brief Declare a bounded sequence of object (non-primitive) elements.
///
/// Each element is of type ELEMENT_TYPE, which must expose:
///   bool ELEMENT_TYPE ## __init(ELEMENT_TYPE *, const rcutils_allocator_t *)
///   void ELEMENT_TYPE ## __fini(ELEMENT_TYPE *)
///   bool ELEMENT_TYPE ## __are_equal(const ELEMENT_TYPE *, const ELEMENT_TYPE *)
///   bool ELEMENT_TYPE ## __copy(const ELEMENT_TYPE *, ELEMENT_TYPE *)
///
/// Unlike the primitive BOUNDED_SEQUENCE, elements are heap-allocated individually
/// (no realloc), so ELEMENT_TYPE may itself contain pointers into managed memory.
/// push_back takes a pointer (not a value) to avoid requiring copyability by value.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE_DECLARE( \
    STRUCT_NAME, ELEMENT_TYPE, UPPER_BOUND) \
  typedef struct STRUCT_NAME ## _s \
  { \
    ELEMENT_TYPE * value; \
    size_t size; \
    size_t capacity; \
    struct \
    { \
      rosidl_runtime_c__experimental__storage_kind_t kind; \
      union \
      { \
        rosidl_memory_region_t region; \
        ELEMENT_TYPE * data; \
      } storage; \
      rcutils_allocator_t allocator; \
      ELEMENT_TYPE ## __ExternalStorage * element_storage_pool; \
      size_t element_storage_pool_size; \
    } _impl; \
  } STRUCT_NAME; \
  bool STRUCT_NAME ## __init( \
    STRUCT_NAME * _sequence); \
  bool STRUCT_NAME ## __init_with_allocator( \
    STRUCT_NAME * _sequence, \
    const rcutils_allocator_t * allocator); \
  bool STRUCT_NAME ## __init_with_storage( \
    STRUCT_NAME * _sequence, \
    const rcutils_allocator_t * allocator, \
    ELEMENT_TYPE ## __ExternalStorage * element_storage_pool, \
    size_t element_storage_pool_size); \
  bool STRUCT_NAME ## __init_from_region( \
    STRUCT_NAME * _sequence, \
    rosidl_memory_region_t region); \
  bool STRUCT_NAME ## __init_region_storage( \
    STRUCT_NAME * _sequence, \
    rosidl_memory_region_t region, \
    ELEMENT_TYPE ## __ExternalStorage * element_storage_pool, \
    size_t element_storage_pool_size); \
  void STRUCT_NAME ## __fini( \
    STRUCT_NAME * _sequence); \
  bool STRUCT_NAME ## __reserve( \
    STRUCT_NAME * _sequence, \
    size_t requested_capacity); \
  bool STRUCT_NAME ## __push_back( \
    STRUCT_NAME * _sequence, \
    const ELEMENT_TYPE * value); \
  bool STRUCT_NAME ## __are_equal( \
    const STRUCT_NAME * lhs, \
    const STRUCT_NAME * rhs); \
  bool STRUCT_NAME ## __copy( \
    const STRUCT_NAME * input, \
    STRUCT_NAME * output);

/// @brief Declare an unbounded sequence of object (non-primitive) elements.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(STRUCT_NAME, ELEMENT_TYPE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE_DECLARE(STRUCT_NAME, ELEMENT_TYPE, 0U)

/// @brief Define a bounded sequence of object elements.
///
/// reserve allocates a new backing buffer and per-element-initialises every new
/// slot, then copies existing elements; it does NOT use realloc (which would
/// invalidate per-element internal pointers such as those in String / sub-message
/// fields).
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE_DEFINE( \
    STRUCT_NAME, ELEMENT_TYPE, UPPER_BOUND) \
  bool STRUCT_NAME ## __init_with_allocator( \
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
    _sequence->_impl.element_storage_pool = NULL; \
    _sequence->_impl.element_storage_pool_size = 0U; \
    return rcutils_allocator_is_valid(&_sequence->_impl.allocator); \
  } \
  bool STRUCT_NAME ## __init( \
    STRUCT_NAME * _sequence) \
  { \
    return STRUCT_NAME ## __init_with_allocator(_sequence, NULL); \
  } \
  bool STRUCT_NAME ## __init_with_storage( \
    STRUCT_NAME * _sequence, \
    const rcutils_allocator_t * allocator, \
    ELEMENT_TYPE ## __ExternalStorage * element_storage_pool, \
    size_t element_storage_pool_size) \
  { \
    if (_sequence == NULL) { \
      return false; \
    } \
    if (element_storage_pool == NULL || element_storage_pool_size == 0U) { \
      return STRUCT_NAME ## __init_with_allocator(_sequence, allocator); \
    } \
    _sequence->value = NULL; \
    _sequence->size = 0U; \
    _sequence->capacity = 0U; \
    _sequence->_impl.kind = ROSIDL_RUNTIME_C__EXPERIMENTAL__STORAGE_KIND__MANAGED; \
    _sequence->_impl.storage.data = NULL; \
    _sequence->_impl.allocator = \
      rosidl_runtime_c__experimental__detail__allocator_or_default(allocator); \
    if (!rcutils_allocator_is_valid(&_sequence->_impl.allocator)) { \
      return false; \
    } \
    rcutils_allocator_t default_allocator = rcutils_get_default_allocator(); \
    size_t pool_bytes = element_storage_pool_size * sizeof(ELEMENT_TYPE ## __ExternalStorage); \
    _sequence->_impl.element_storage_pool = \
      (ELEMENT_TYPE ## __ExternalStorage *)default_allocator.allocate( \
        pool_bytes, default_allocator.state); \
    if (_sequence->_impl.element_storage_pool == NULL) { \
      return false; \
    } \
    (void)memcpy( \
      _sequence->_impl.element_storage_pool, element_storage_pool, pool_bytes); \
    _sequence->_impl.element_storage_pool_size = element_storage_pool_size; \
    return true; \
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
    _sequence->value = (ELEMENT_TYPE *)region.location.address; \
    _sequence->size = 0U; \
    _sequence->capacity = region.size / sizeof(ELEMENT_TYPE); \
    if ((UPPER_BOUND) > 0U && _sequence->capacity > (size_t)(UPPER_BOUND)) { \
      _sequence->capacity = (size_t)(UPPER_BOUND); \
    } \
    _sequence->_impl.element_storage_pool = NULL; \
    _sequence->_impl.element_storage_pool_size = 0U; \
    size_t _init_i; \
    for (_init_i = 0U; _init_i < _sequence->capacity; ++_init_i) { \
      if (!ELEMENT_TYPE ## __init_with_allocator( \
          &_sequence->value[_init_i], &_sequence->_impl.allocator)) \
      { \
        for (; _init_i-- > 0U; ) { \
          ELEMENT_TYPE ## __fini(&_sequence->value[_init_i]); \
        } \
        _sequence->value = NULL; \
        _sequence->capacity = 0U; \
        return false; \
      } \
    } \
    return true; \
  } \
  bool STRUCT_NAME ## __init_region_storage( \
    STRUCT_NAME * _sequence, \
    rosidl_memory_region_t region, \
    ELEMENT_TYPE ## __ExternalStorage * element_storage_pool, \
    size_t element_storage_pool_size) \
  { \
    if (_sequence == NULL || region.location.address == NULL) { \
      return false; \
    } \
    if (element_storage_pool == NULL || element_storage_pool_size == 0U) { \
      return STRUCT_NAME ## __init_from_region(_sequence, region); \
    } \
    _sequence->_impl.kind = ROSIDL_RUNTIME_C__EXPERIMENTAL__STORAGE_KIND__EXTERNAL; \
    _sequence->_impl.storage.region = region; \
    _sequence->_impl.allocator = rcutils_get_default_allocator(); \
    _sequence->value = (ELEMENT_TYPE *)region.location.address; \
    _sequence->size = 0U; \
    _sequence->capacity = region.size / sizeof(ELEMENT_TYPE); \
    if ((UPPER_BOUND) > 0U && _sequence->capacity > (size_t)(UPPER_BOUND)) { \
      _sequence->capacity = (size_t)(UPPER_BOUND); \
    } \
    rcutils_allocator_t default_allocator = rcutils_get_default_allocator(); \
    size_t pool_bytes = element_storage_pool_size * sizeof(ELEMENT_TYPE ## __ExternalStorage); \
    _sequence->_impl.element_storage_pool = \
      (ELEMENT_TYPE ## __ExternalStorage *)default_allocator.allocate( \
        pool_bytes, default_allocator.state); \
    if (_sequence->_impl.element_storage_pool == NULL) { \
      return false; \
    } \
    (void)memcpy( \
      _sequence->_impl.element_storage_pool, element_storage_pool, pool_bytes); \
    _sequence->_impl.element_storage_pool_size = element_storage_pool_size; \
    if (_sequence->capacity > element_storage_pool_size) { \
      _sequence->capacity = element_storage_pool_size; \
    } \
    size_t _init_i; \
    for (_init_i = 0U; _init_i < _sequence->capacity; ++_init_i) { \
      bool _init_success; \
      if (_init_i < _sequence->_impl.element_storage_pool_size) { \
        _init_success = ELEMENT_TYPE ## __init_from_storage( \
          &_sequence->value[_init_i], \
          &_sequence->_impl.element_storage_pool[_init_i]); \
      } else { \
        _init_success = ELEMENT_TYPE ## __init_with_allocator( \
          &_sequence->value[_init_i], &_sequence->_impl.allocator); \
      } \
      if (!_init_success) \
      { \
        for (; _init_i-- > 0U; ) { \
          ELEMENT_TYPE ## __fini(&_sequence->value[_init_i]); \
        } \
        default_allocator.deallocate( \
          _sequence->_impl.element_storage_pool, default_allocator.state); \
        _sequence->_impl.element_storage_pool = NULL; \
        _sequence->_impl.element_storage_pool_size = 0U; \
        _sequence->value = NULL; \
        _sequence->capacity = 0U; \
        return false; \
      } \
    } \
    return true; \
  } \
  void STRUCT_NAME ## __fini( \
    STRUCT_NAME * _sequence) \
  { \
    if (_sequence == NULL) { \
      return; \
    } \
    if (_sequence->value != NULL) { \
      for (size_t _i = 0U; _i < _sequence->capacity; ++_i) { \
        ELEMENT_TYPE ## __fini(&_sequence->value[_i]); \
      } \
    } \
    if (_sequence->_impl.kind == ROSIDL_RUNTIME_C__EXPERIMENTAL__STORAGE_KIND__MANAGED && \
      _sequence->_impl.storage.data != NULL) \
    { \
      _sequence->_impl.allocator.deallocate( \
        _sequence->_impl.storage.data, _sequence->_impl.allocator.state); \
    } \
    if (_sequence->_impl.element_storage_pool != NULL) { \
      rcutils_allocator_t default_allocator = rcutils_get_default_allocator(); \
      default_allocator.deallocate( \
        _sequence->_impl.element_storage_pool, default_allocator.state); \
      _sequence->_impl.element_storage_pool = NULL; \
    } \
    _sequence->value = NULL; \
    _sequence->size = 0U; \
    _sequence->capacity = 0U; \
    _sequence->_impl.element_storage_pool_size = 0U; \
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
    if (_sequence->_impl.element_storage_pool != NULL && \
      requested_capacity > _sequence->_impl.element_storage_pool_size) \
    { \
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
    if (_sequence->_impl.element_storage_pool != NULL && \
      new_capacity > _sequence->_impl.element_storage_pool_size) \
    { \
      new_capacity = _sequence->_impl.element_storage_pool_size; \
    } \
    size_t new_bytes = 0U; \
    if (!rosidl_runtime_c__experimental__detail__compute_bytes( \
        new_capacity, sizeof(ELEMENT_TYPE), &new_bytes)) \
    { \
      return false; \
    } \
    ELEMENT_TYPE * new_data = \
      (ELEMENT_TYPE *)_sequence->_impl.allocator.allocate( \
        new_bytes, _sequence->_impl.allocator.state); \
    if (new_data == NULL) { \
      return false; \
    } \
    size_t _i; \
    for (_i = 0U; _i < new_capacity; ++_i) { \
      bool _init_success; \
      if (_sequence->_impl.element_storage_pool != NULL && \
        _i < _sequence->_impl.element_storage_pool_size) \
      { \
        _init_success = ELEMENT_TYPE ## __init_from_storage( \
          &new_data[_i], &_sequence->_impl.element_storage_pool[_i]); \
      } else { \
        _init_success = ELEMENT_TYPE ## __init_with_allocator( \
          &new_data[_i], &_sequence->_impl.allocator); \
      } \
      if (!_init_success) { \
        for (; _i-- > 0U; ) { \
          ELEMENT_TYPE ## __fini(&new_data[_i]); \
        } \
        _sequence->_impl.allocator.deallocate(new_data, _sequence->_impl.allocator.state); \
        return false; \
      } \
    } \
    for (_i = 0U; _i < _sequence->size; ++_i) { \
      if (!ELEMENT_TYPE ## __copy(&_sequence->value[_i], &new_data[_i])) { \
        for (size_t _j = 0U; _j < new_capacity; ++_j) { \
          ELEMENT_TYPE ## __fini(&new_data[_j]); \
        } \
        _sequence->_impl.allocator.deallocate(new_data, _sequence->_impl.allocator.state); \
        return false; \
      } \
    } \
    for (_i = 0U; _i < _sequence->capacity; ++_i) { \
      ELEMENT_TYPE ## __fini(&_sequence->value[_i]); \
    } \
    if (_sequence->_impl.storage.data != NULL) { \
      _sequence->_impl.allocator.deallocate( \
        _sequence->_impl.storage.data, _sequence->_impl.allocator.state); \
    } \
    _sequence->_impl.storage.data = new_data; \
    _sequence->value = new_data; \
    _sequence->capacity = new_capacity; \
    return true; \
  } \
  bool STRUCT_NAME ## __push_back( \
    STRUCT_NAME * _sequence, \
    const ELEMENT_TYPE * value) \
  { \
    if (_sequence == NULL || value == NULL) { \
      return false; \
    } \
    if (!STRUCT_NAME ## __reserve(_sequence, _sequence->size + 1U)) { \
      return false; \
    } \
    if (!ELEMENT_TYPE ## __copy(value, &_sequence->value[_sequence->size])) { \
      return false; \
    } \
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
    for (size_t _i = 0U; _i < lhs->size; ++_i) { \
      if (!ELEMENT_TYPE ## __are_equal(&lhs->value[_i], &rhs->value[_i])) { \
        return false; \
      } \
    } \
    return true; \
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
    if (output->_impl.element_storage_pool != NULL) { \
      rcutils_allocator_t default_allocator = rcutils_get_default_allocator(); \
      default_allocator.deallocate( \
        output->_impl.element_storage_pool, default_allocator.state); \
      output->_impl.element_storage_pool = NULL; \
      output->_impl.element_storage_pool_size = 0U; \
    } \
    if (input->_impl.element_storage_pool != NULL && \
      input->_impl.element_storage_pool_size > 0U) \
    { \
      rcutils_allocator_t default_allocator = rcutils_get_default_allocator(); \
      size_t pool_bytes = \
        input->_impl.element_storage_pool_size * sizeof(ELEMENT_TYPE ## __ExternalStorage); \
      output->_impl.element_storage_pool = \
        (ELEMENT_TYPE ## __ExternalStorage *)default_allocator.allocate( \
          pool_bytes, default_allocator.state); \
      if (output->_impl.element_storage_pool == NULL) { \
        return false; \
      } \
      (void)memcpy( \
        output->_impl.element_storage_pool, \
        input->_impl.element_storage_pool, \
        pool_bytes); \
      output->_impl.element_storage_pool_size = input->_impl.element_storage_pool_size; \
    } \
    if (output->size > input->size) { \
      output->size = input->size; \
    } \
    if (!STRUCT_NAME ## __reserve(output, input->size)) { \
      if (output->_impl.element_storage_pool != NULL) { \
        rcutils_allocator_t default_allocator = rcutils_get_default_allocator(); \
        default_allocator.deallocate( \
          output->_impl.element_storage_pool, default_allocator.state); \
        output->_impl.element_storage_pool = NULL; \
        output->_impl.element_storage_pool_size = 0U; \
      } \
      return false; \
    } \
    for (size_t _i = 0U; _i < input->size; ++_i) { \
      if (!ELEMENT_TYPE ## __copy(&input->value[_i], &output->value[_i])) { \
        output->size = _i; \
        return false; \
      } \
    } \
    output->size = input->size; \
    return true; \
  }

/// @brief Define an unbounded sequence of object elements.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DEFINE(STRUCT_NAME, ELEMENT_TYPE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE_DEFINE( \
    STRUCT_NAME, ELEMENT_TYPE, 0U)

/// @brief Convenience macro declaring and defining a bounded object sequence in one place.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE( \
    STRUCT_NAME, ELEMENT_TYPE, UPPER_BOUND) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE_DECLARE( \
    STRUCT_NAME, ELEMENT_TYPE, UPPER_BOUND); \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE_DEFINE( \
    STRUCT_NAME, ELEMENT_TYPE, UPPER_BOUND)

/// @brief Convenience macro declaring and defining an unbounded object sequence in one place.
#define ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE(STRUCT_NAME, ELEMENT_TYPE) \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(STRUCT_NAME, ELEMENT_TYPE); \
  ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DEFINE(STRUCT_NAME, ELEMENT_TYPE)

#include "rosidl_runtime_c/experimental/string.h"

// Sequence types for unbounded string/wstring.
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(
  rosidl_runtime_c__experimental__StringSequence,
  rosidl_runtime_c__experimental__String);
ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE(
  rosidl_runtime_c__experimental__WStringSequence,
  rosidl_runtime_c__experimental__WString);

// Sequence types for all primitive ROSIDL C types.
ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(rosidl_runtime_c__experimental__FloatSequence,
  float);
ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(rosidl_runtime_c__experimental__DoubleSequence,
  double);
ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(rosidl_runtime_c__experimental__LongDoubleSequence,
  long double);
ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(rosidl_runtime_c__experimental__CharSequence,
  char);
ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(rosidl_runtime_c__experimental__WCharSequence,
  char16_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(rosidl_runtime_c__experimental__BooleanSequence,
  bool);
ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(rosidl_runtime_c__experimental__UInt8Sequence,
  uint8_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(rosidl_runtime_c__experimental__Int8Sequence,
  int8_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(rosidl_runtime_c__experimental__UInt16Sequence,
  uint16_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(rosidl_runtime_c__experimental__Int16Sequence,
  int16_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(rosidl_runtime_c__experimental__UInt32Sequence,
  uint32_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(rosidl_runtime_c__experimental__Int32Sequence,
  int32_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(rosidl_runtime_c__experimental__UInt64Sequence,
  uint64_t);
ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_SEQUENCE_DECLARE(rosidl_runtime_c__experimental__Int64Sequence,
  int64_t);

#ifdef __cplusplus
}
#endif

#endif  // ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_H_
