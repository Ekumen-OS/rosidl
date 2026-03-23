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

#ifndef ROSIDL_RUNTIME_CPP__EXPERIMENTAL__SEQUENCE_HPP_
#define ROSIDL_RUNTIME_CPP__EXPERIMENTAL__SEQUENCE_HPP_

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory_resource>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "rosidl_runtime_cpp/experimental/memory.hpp"

namespace rosidl_runtime_cpp
{

/// @file
/// @brief Experimental vector-like sequence with fixed-region or PMR storage.

/// @brief Vector-like sequence with optional upper bound.
///
/// Storage can be either fixed (`MemoryRegion`) or dynamic (`std::pmr::memory_resource`).
/// Scalar element types use contiguous fast paths.
/// @tparam T Element type.
/// @tparam UpperBound Maximum elements (`0` means unbounded).
template<typename T, std::size_t UpperBound = 0>
class BasicSequence
{
public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = T &;
  using const_reference = const T &;
  using pointer = T *;
  using const_pointer = const T *;

private:
  template<bool IsConst>
  class StridedIterator
  {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = typename std::conditional<IsConst, const T *, T *>::type;
    using reference = typename std::conditional<IsConst, const T &, T &>::type;
    using byte_pointer = typename std::conditional<IsConst, const std::byte *, std::byte *>::type;

    StridedIterator(byte_pointer raw_pointer, std::size_t stride)
    : raw_pointer_(raw_pointer), stride_(stride)
    {}

    template<bool B = IsConst, typename std::enable_if<B, int>::type = 0>
    StridedIterator(const StridedIterator<false> & other)
    : raw_pointer_(other.raw_pointer_), stride_(other.stride_)
    {}

    reference operator*() const {return *reinterpret_cast<pointer>(raw_pointer_);}
    pointer operator->() const {return reinterpret_cast<pointer>(raw_pointer_);}
    reference operator[](difference_type offset) const {return *(*this + offset);}
    StridedIterator & operator++() {raw_pointer_ += stride_; return *this;}
    StridedIterator operator++(int) {auto copy = *this; ++(*this); return copy;}
    StridedIterator & operator--() {raw_pointer_ -= stride_; return *this;}
    StridedIterator operator--(int) {auto copy = *this; --(*this); return copy;}
    StridedIterator & operator+=(difference_type offset)
    {
      raw_pointer_ += offset * static_cast<difference_type>(stride_); return *this;
    }
    StridedIterator & operator-=(difference_type offset)
    {
      raw_pointer_ -= offset * static_cast<difference_type>(stride_); return *this;
    }
    StridedIterator operator+(difference_type offset) const
    {
      auto copy = *this; copy += offset; return copy;
    }
    StridedIterator operator-(difference_type offset) const
    {
      auto copy = *this; copy -= offset; return copy;
    }
    difference_type operator-(const StridedIterator & other) const
    {
      return (raw_pointer_ - other.raw_pointer_) / static_cast<difference_type>(stride_);
    }
    bool operator==(const StridedIterator & other) const
    {
      return raw_pointer_ == other.raw_pointer_;
    }
    bool operator!=(const StridedIterator & other) const {return !(*this == other);}
    bool operator<(const StridedIterator & other) const {return raw_pointer_ < other.raw_pointer_;}
    bool operator>(const StridedIterator & other) const {return other < *this;}
    bool operator<=(const StridedIterator & other) const {return !(other < *this);}
    bool operator>=(const StridedIterator & other) const {return !(*this < other);}

private:
    byte_pointer raw_pointer_;
    std::size_t stride_;
    template<bool>
    friend class StridedIterator;
  };

public:
  using iterator = typename std::conditional<
    std::is_scalar<T>::value, pointer, StridedIterator<false>>::type;
  using const_iterator = typename std::conditional<
    std::is_scalar<T>::value, const_pointer, StridedIterator<true>>::type;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  /// @brief Construct with default PMR resource.
  BasicSequence()
  : storage_pool_(std::pmr::get_default_resource()), storage_(make_empty_memory_region()),
    capacity_(0), size_(0)
  {}

  /// @brief Construct with explicit PMR resource.
  /// @param storage_pool Resource pointer (defaults to global PMR resource when null).
  explicit BasicSequence(std::pmr::memory_resource * storage_pool)
  : storage_pool_(storage_pool ? storage_pool : std::pmr::get_default_resource()),
    storage_(make_empty_memory_region()),
    capacity_(0),
    size_(0)
  {}

  /// @brief Construct over fixed external storage.
  /// @param storage_region External memory region.
  explicit BasicSequence(MemoryRegion storage_region)
  : storage_pool_(nullptr), storage_(storage_region), capacity_(0), size_(0)
  {
    capacity_ = clamp_to_upper_bound(capacity_from_region(storage_region));
  }

  explicit BasicSequence(
    size_type count,
    std::pmr::memory_resource * storage_pool = std::pmr::get_default_resource())
  : BasicSequence(storage_pool)
  {
    resize(count);
  }

  BasicSequence(
    size_type count,
    const value_type & value,
    std::pmr::memory_resource * storage_pool = std::pmr::get_default_resource())
  : BasicSequence(storage_pool)
  {
    assign(count, value);
  }
  template<typename InputIterator>
  BasicSequence(
    InputIterator first,
    InputIterator last,
    std::pmr::memory_resource * storage_pool = std::pmr::get_default_resource())
  : BasicSequence(storage_pool)
  {
    assign(first, last);
  }

  BasicSequence(
    std::initializer_list<value_type> values,
    std::pmr::memory_resource * storage_pool = std::pmr::get_default_resource())
  : BasicSequence(storage_pool)
  {
    assign(values.begin(), values.end());
  }

  BasicSequence(const std::vector<T> & source_vector)  // NOLINT(runtime/explicit)
  : BasicSequence(std::pmr::get_default_resource())
  {
    *this = source_vector;
  }

  BasicSequence(std::vector<T> && source_vector)  // NOLINT(runtime/explicit)
  : BasicSequence(std::pmr::get_default_resource())
  {
    *this = std::move(source_vector);
  }

  BasicSequence(const BasicSequence & other)
  : BasicSequence(other.storage_pool_ ? other.storage_pool_ : std::pmr::get_default_resource())
  {
    assign(other.begin(), other.end());
  }

  BasicSequence(BasicSequence && other) noexcept
  : storage_pool_(other.storage_pool_),
    storage_(other.storage_),
    capacity_(other.capacity_),
    size_(other.size_)
  {
    other.storage_pool_ = std::pmr::get_default_resource();
    other.storage_ = make_empty_memory_region();
    other.capacity_ = 0;
    other.size_ = 0;
  }

  ~BasicSequence()
  {
    clear();
    release_owned_storage();
  }

  BasicSequence & operator=(const BasicSequence & other)
  {
    if (this == &other) {
      return *this;
    }
    clear();
    release_owned_storage();
    if (storage_pool_) {
      storage_.size = 0;
      capacity_ = 0;
    }
    assign(other.begin(), other.end());
    return *this;
  }
  BasicSequence & operator=(BasicSequence && other) noexcept
  {
    if (this == &other) {
      return *this;
    }
    clear();
    release_owned_storage();
    storage_pool_ = other.storage_pool_;
    storage_ = other.storage_;
    capacity_ = other.capacity_;
    size_ = other.size_;
    other.storage_pool_ = std::pmr::get_default_resource();
    other.storage_ = make_empty_memory_region();
    other.capacity_ = 0;
    other.size_ = 0;
    return *this;
  }
  BasicSequence & operator=(const std::vector<T> & source_vector)
  {
    assign(source_vector.begin(), source_vector.end());
    return *this;
  }

  BasicSequence & operator=(std::vector<T> && source_vector)
  {
    clear();
    ensure_capacity_or_fail(source_vector.size());
    if (std::is_scalar<T>::value && !source_vector.empty()) {
      std::memmove(data(), source_vector.data(), source_vector.size() * sizeof(T));
      size_ = source_vector.size();
      return *this;
    }
    for (size_type position = 0; position < source_vector.size(); ++position) {
      emplace_back(std::move(source_vector[position]));
    }
    return *this;
  }
  operator std::vector<T>() const
  {
    std::vector<T> vector;
    vector.reserve(size_);
    for (size_type position = 0; position < size_; ++position) {
      vector.push_back((*this)[position]);
    }
    return vector;
  }
  reference at(size_type position)
  {
    if (position >= size_) {
      throw std::out_of_range("Sequence::at: index out of range");
    }
    return (*this)[position];
  }

  const_reference at(size_type position) const
  {
    if (position >= size_) {
      throw std::out_of_range("Sequence::at: index out of range");
    }
    return (*this)[position];
  }
  reference operator[](size_type position) noexcept
  {
    if (std::is_scalar<T>::value) {
      return data()[position];
    }
    return *reinterpret_cast<pointer>(
      static_cast<std::byte *>(storage_.location.address) + position * sizeof(T));
  }

  const_reference operator[](size_type position) const noexcept
  {
    if (std::is_scalar<T>::value) {
      return data()[position];
    }
    return *reinterpret_cast<const_pointer>(
      static_cast<const std::byte *>(storage_.location.address) + position * sizeof(T));
  }
  reference front() noexcept {return (*this)[0];}
  const_reference front() const noexcept {return (*this)[0];}
  reference back() noexcept {return (*this)[size_ - 1];}
  const_reference back() const noexcept {return (*this)[size_ - 1];}
  pointer data() noexcept {return reinterpret_cast<pointer>(storage_.location.address);}
  const_pointer data() const noexcept
  {
    return reinterpret_cast<const_pointer>(storage_.location.address);
  }
  iterator begin() noexcept
  {
    if (std::is_scalar<T>::value) {
      return data();
    }
    return iterator(static_cast<std::byte *>(storage_.location.address), sizeof(T));
  }

  const_iterator begin() const noexcept
  {
    if (std::is_scalar<T>::value) {
      return data();
    }
    return const_iterator(
      static_cast<const std::byte *>(storage_.location.address),
      sizeof(T));
  }
  const_iterator cbegin() const noexcept {return begin();}
  iterator end() noexcept
  {
    if (std::is_scalar<T>::value) {
      return data() + size_;
    }
    return iterator(
      static_cast<std::byte *>(storage_.location.address) + size_ * sizeof(T),
      sizeof(T));
  }

  const_iterator end() const noexcept
  {
    if (std::is_scalar<T>::value) {
      return data() + size_;
    }
    return const_iterator(
      static_cast<const std::byte *>(storage_.location.address) + size_ * sizeof(T),
      sizeof(T));
  }
  const_iterator cend() const noexcept {return end();}
  reverse_iterator rbegin() noexcept {return reverse_iterator(end());}
  const_reverse_iterator rbegin() const noexcept {return const_reverse_iterator(end());}
  const_reverse_iterator crbegin() const noexcept {return rbegin();}
  reverse_iterator rend() noexcept {return reverse_iterator(begin());}
  const_reverse_iterator rend() const noexcept {return const_reverse_iterator(begin());}
  const_reverse_iterator crend() const noexcept {return rend();}
  bool empty() const noexcept {return size_ == 0;}
  size_type size() const noexcept {return size_;}
  size_type capacity() const noexcept {return capacity_;}

  size_type max_size() const noexcept
  {
    const size_type runtime_max_size = std::numeric_limits<size_type>::max() / sizeof(T);
    return UpperBound > 0 ? std::min(runtime_max_size,
        static_cast<size_type>(UpperBound)) : runtime_max_size;
  }

  /// @brief Whether this sequence uses a fixed external region.
  bool using_fixed_storage() const noexcept {return !storage_pool_;}

  /// @brief PMR resource for dynamic storage.
  std::pmr::memory_resource * resource() const noexcept {return storage_pool_;}

  void reserve(size_type new_capacity) {ensure_capacity_or_fail(new_capacity);}

  void shrink_to_fit()
  {
    if (!storage_pool_ || size_ == capacity_) {
      return;
    }
    reallocate_owned_storage(size_);
  }

  void clear() noexcept
  {
    destroy_elements(0, size_);
    size_ = 0;
  }
  void push_back(const value_type & value) {emplace_back(value);}
  void push_back(value_type && value) {emplace_back(std::move(value));}
  template<typename ... Arguments>
  reference emplace_back(Arguments && ... arguments)
  {
    ensure_capacity_or_fail(size_ + 1);
    pointer element_pointer = reinterpret_cast<pointer>(
      static_cast<std::byte *>(storage_.location.address) + size_ * sizeof(T));
    emplace_value(element_pointer, std::forward<Arguments>(arguments)...);
    ++size_;
    return *element_pointer;
  }

  void pop_back()
  {
    if (size_ == 0) {
      return;
    }
    destroy_elements(size_ - 1, size_);
    --size_;
  }

  void resize(size_type new_size)
  {
    if (new_size < size_) {
      destroy_elements(new_size, size_);
      size_ = new_size;
      return;
    }
    ensure_capacity_or_fail(new_size);
    if (std::is_scalar<T>::value) {
      std::fill_n(data() + size_, new_size - size_, T{});
    } else {
      for (size_type position = size_; position < new_size; ++position) {
        ::new (static_cast<void *>(&(*this)[position])) T();
      }
    }
    size_ = new_size;
  }

  void resize(size_type new_size, const value_type & value)
  {
    if (new_size < size_) {
      destroy_elements(new_size, size_);
      size_ = new_size;
      return;
    }
    ensure_capacity_or_fail(new_size);
    if (std::is_scalar<T>::value) {
      std::fill_n(data() + size_, new_size - size_, value);
    } else {
      for (size_type position = size_; position < new_size; ++position) {
        ::new (static_cast<void *>(&(*this)[position])) T(value);
      }
    }
    size_ = new_size;
  }

  void assign(size_type count, const value_type & value)
  {
    clear();
    ensure_capacity_or_fail(count);
    if (std::is_scalar<T>::value) {
      std::fill_n(data(), count, value);
      size_ = count;
      return;
    }
    for (size_type position = 0; position < count; ++position) {
      emplace_back(value);
    }
  }

  template<typename InputIterator>
  std::void_t<typename std::iterator_traits<InputIterator>::iterator_category>
  assign(InputIterator first, InputIterator last)
  {
    clear();
    for (auto current = first; current != last; ++current) {
      emplace_back(*current);
    }
  }

  void swap(BasicSequence & other) noexcept
  {
    std::swap(storage_pool_, other.storage_pool_);
    std::swap(storage_, other.storage_);
    std::swap(capacity_, other.capacity_);
    std::swap(size_, other.size_);
  }

private:
  static MemoryRegion make_empty_memory_region()
  {
    MemoryRegion storage_region{};
    storage_region.location.address = nullptr;
    storage_region.location.attributes = 0;
    storage_region.size = 0;
    return storage_region;
  }

  static size_type capacity_from_region(const MemoryRegion & storage_region)
  {
    return storage_region.size / sizeof(T);
  }

  static size_type clamp_to_upper_bound(size_type capacity)
  {
    return UpperBound > 0 ? std::min(capacity, static_cast<size_type>(UpperBound)) : capacity;
  }

  void ensure_capacity_or_fail(size_type requested_capacity)
  {
    if (UpperBound > 0 && requested_capacity > static_cast<size_type>(UpperBound)) {
      throw std::length_error("Sequence upper bound exceeded");
    }
    if (requested_capacity <= capacity_) {
      return;
    }
    if (!storage_pool_) {
      throw std::length_error("Sequence capacity exceeded for fixed storage");
    }
    size_type new_capacity = capacity_ == 0 ? requested_capacity : capacity_;
    while (new_capacity < requested_capacity) {
      new_capacity *= 2;
      if (new_capacity == 0) {
        new_capacity = requested_capacity;
        break;
      }
    }
    if (UpperBound > 0) {
      new_capacity = std::min(new_capacity, static_cast<size_type>(UpperBound));
    }
    reallocate_owned_storage(new_capacity);
  }
  template<typename ... Arguments>
  void emplace_value(pointer element_pointer, Arguments && ... arguments)
  {
    if (std::is_scalar<T>::value) {
      *element_pointer = T(std::forward<Arguments>(arguments)...);
    } else {
      ::new (static_cast<void *>(element_pointer)) T(std::forward<Arguments>(arguments)...);
    }
  }

  void reallocate_owned_storage(size_type new_capacity)
  {
    void * new_address = storage_pool_->allocate(new_capacity * sizeof(T), alignof(T));
    if (std::is_scalar<T>::value) {
      if (size_ > 0 && storage_.location.address != nullptr) {
        std::memmove(new_address, storage_.location.address, size_ * sizeof(T));
      }
    } else {
      auto * new_storage_pointer = static_cast<std::byte *>(new_address);
      for (size_type position = 0; position < size_; ++position) {
        auto * destination_pointer = reinterpret_cast<pointer>(
          new_storage_pointer + position * sizeof(T));
        ::new (static_cast<void *>(destination_pointer)) T(std::move((*this)[position]));
      }
      destroy_elements(0, size_);
    }

    release_owned_storage();
    storage_.location.address = new_address;
    storage_.location.attributes = 0;
    storage_.size = new_capacity * sizeof(T);
    capacity_ = new_capacity;
  }

  void release_owned_storage()
  {
    if (!storage_pool_ || storage_.location.address == nullptr) {
      return;
    }
    storage_pool_->deallocate(storage_.location.address, storage_.size, alignof(T));
    storage_ = make_empty_memory_region();
    capacity_ = 0;
  }

  void destroy_elements(size_type first, size_type last) noexcept
  {
    if (std::is_scalar<T>::value || std::is_trivially_destructible<T>::value) {
      return;
    }
    for (size_type position = first; position < last; ++position) {
      (&(*this)[position])->~T();
    }
  }

  std::pmr::memory_resource * storage_pool_;
  MemoryRegion storage_;
  size_type capacity_;
  size_type size_;
};

template<typename T>
class Sequence : public BasicSequence<T>
{
public:
  using BasicSequence<T>::BasicSequence;
};

template<typename T, std::size_t UpperBound>
class BoundedSequence : public BasicSequence<T, UpperBound>
{
public:
  using BasicSequence<T, UpperBound>::BasicSequence;
};

}  // namespace rosidl_runtime_cpp

#endif  // ROSIDL_RUNTIME_CPP__EXPERIMENTAL__SEQUENCE_HPP_
