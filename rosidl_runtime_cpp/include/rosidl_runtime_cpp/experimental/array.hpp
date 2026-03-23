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

#ifndef ROSIDL_RUNTIME_CPP__EXPERIMENTAL__ARRAY_HPP_
#define ROSIDL_RUNTIME_CPP__EXPERIMENTAL__ARRAY_HPP_

#include <array>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

#include "rosidl_runtime_cpp/experimental/memory.hpp"

namespace rosidl_runtime_cpp
{

/// @file
/// @brief Experimental fixed-size array with optional strided external storage.

/// @brief `std::array`-like container over inline or external memory.
/// @tparam T Element type.
/// @tparam N Number of elements.
template<typename T, std::size_t N>
class Array
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

    /// @brief Construct a strided iterator.
    /// @param raw_pointer Byte pointer to element storage.
    /// @param stride Step size in bytes between consecutive elements.
    StridedIterator(byte_pointer raw_pointer, std::size_t stride)
    : raw_pointer_(raw_pointer), stride_(stride)
    {}

    template<bool B = IsConst, typename std::enable_if<B, int>::type = 0>
    StridedIterator(const StridedIterator<false> & other)
    : raw_pointer_(other.raw_pointer_), stride_(other.stride_)
    {}

    reference operator*() const
    {
      return *reinterpret_cast<pointer>(raw_pointer_);
    }

    pointer operator->() const
    {
      return reinterpret_cast<pointer>(raw_pointer_);
    }

    reference operator[](difference_type offset) const
    {
      return *(*this + offset);
    }

    StridedIterator & operator++()
    {
      raw_pointer_ += stride_;
      return *this;
    }

    StridedIterator operator++(int)
    {
      auto copy = *this;
      ++(*this);
      return copy;
    }

    StridedIterator & operator--()
    {
      raw_pointer_ -= stride_;
      return *this;
    }

    StridedIterator operator--(int)
    {
      auto copy = *this;
      --(*this);
      return copy;
    }

    StridedIterator & operator+=(difference_type offset)
    {
      raw_pointer_ += offset * static_cast<difference_type>(stride_);
      return *this;
    }

    StridedIterator & operator-=(difference_type offset)
    {
      raw_pointer_ -= offset * static_cast<difference_type>(stride_);
      return *this;
    }

    StridedIterator operator+(difference_type offset) const
    {
      auto copy = *this;
      copy += offset;
      return copy;
    }

    StridedIterator operator-(difference_type offset) const
    {
      auto copy = *this;
      copy -= offset;
      return copy;
    }

    difference_type operator-(const StridedIterator & other) const
    {
      return (raw_pointer_ - other.raw_pointer_) /
             static_cast<difference_type>(stride_);
    }

    bool operator==(const StridedIterator & other) const
    {
      return raw_pointer_ == other.raw_pointer_;
    }

    bool operator!=(const StridedIterator & other) const
    {
      return !(*this == other);
    }

    bool operator<(const StridedIterator & other) const
    {
      return raw_pointer_ < other.raw_pointer_;
    }

    bool operator>(const StridedIterator & other) const
    {
      return other < *this;
    }

    bool operator<=(const StridedIterator & other) const
    {
      return !(other < *this);
    }

    bool operator>=(const StridedIterator & other) const
    {
      return !(*this < other);
    }

private:
    byte_pointer raw_pointer_;
    std::size_t stride_;

    template<bool>
    friend class StridedIterator;
  };

public:
  using iterator = StridedIterator<false>;
  using const_iterator = StridedIterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

private:
  struct InternalStorage
  {
    T data[N]{};
  };

public:
  /// @brief Construct with internal storage.
  Array()
  : storage_(InternalStorage{})
  {}

  /// @brief Construct from `std::array`.
  /// @param array Source array.
  Array(const std::array<T, N> & array)  // NOLINT(runtime/explicit)
  : storage_(InternalStorage{})
  {
    *this = array;
  }

  /// @brief Construct from moved `std::array`.
  /// @param array Source array.
  Array(std::array<T, N> && array)  // NOLINT(runtime/explicit)
  : storage_(InternalStorage{})
  {
    *this = std::move(array);
  }

  /// @brief Construct over an external memory region.
  /// @param region External memory region.
  explicit Array(MemoryRegion region)
  : storage_(region)
  {}

  /// @brief Assign from `std::array`.
  /// @param array Source array.
  /// @return Reference to this array.
  Array & operator=(const std::array<T, N> & array)
  {
    for (size_type i = 0; i < N; ++i) {
      (*this)[i] = array[i];
    }
    return *this;
  }

  /// @brief Move-assign from `std::array`.
  /// @param array Source array.
  /// @return Reference to this array.
  Array & operator=(std::array<T, N> && array)
  {
    for (size_type i = 0; i < N; ++i) {
      (*this)[i] = std::move(array[i]);
    }
    return *this;
  }

  /// @brief Convert to `std::array`.
  /// @return Copied `std::array` value.
  operator std::array<T, N>() const
  {
    std::array<T, N> array{};
    for (size_type i = 0; i < N; ++i) {
      array[i] = (*this)[i];
    }
    return array;
  }

  /// @brief Bounds-checked element access.
  /// @param position Element index.
  /// @return Element reference.
  reference at(size_type position)
  {
    if (position >= N) {
      throw std::out_of_range("Array::at: index out of range");
    }
    return (*this)[position];
  }

  /// @brief Bounds-checked element access.
  /// @param position Element index.
  /// @return Element reference.
  const_reference at(size_type position) const
  {
    if (position >= N) {
      throw std::out_of_range("Array::at: index out of range");
    }
    return (*this)[position];
  }

  /// @brief Unchecked element access.
  reference operator[](size_type position) noexcept
  {
    if (std::holds_alternative<MemoryRegion>(storage_)) {
      const auto & region = std::get<MemoryRegion>(storage_);
      return *reinterpret_cast<pointer>(
        static_cast<std::byte *>(region.location.address) + position * sizeof(T));
    }
    return std::get<InternalStorage>(storage_).data[position];
  }

  /// @brief Unchecked element access.
  const_reference operator[](size_type position) const noexcept
  {
    if (std::holds_alternative<MemoryRegion>(storage_)) {
      const auto & region = std::get<MemoryRegion>(storage_);
      return *reinterpret_cast<const_pointer>(
        static_cast<const std::byte *>(region.location.address) + position * sizeof(T));
    }
    return std::get<InternalStorage>(storage_).data[position];
  }

  /// @brief First element.
  reference front() noexcept {return (*this)[0];}

  /// @brief First element.
  const_reference front() const noexcept {return (*this)[0];}

  /// @brief Last element.
  reference back() noexcept {return (*this)[N - 1];}

  /// @brief Last element.
  const_reference back() const noexcept {return (*this)[N - 1];}

  /// @brief Pointer to first element.
  pointer data() noexcept
  {
    if (std::holds_alternative<MemoryRegion>(storage_)) {
      return reinterpret_cast<pointer>(std::get<MemoryRegion>(storage_).location.address);
    }
    return std::get<InternalStorage>(storage_).data;
  }

  /// @brief Pointer to first element.
  const_pointer data() const noexcept
  {
    if (std::holds_alternative<MemoryRegion>(storage_)) {
      return reinterpret_cast<const_pointer>(
        std::get<MemoryRegion>(storage_).location.address);
    }
    return std::get<InternalStorage>(storage_).data;
  }

  iterator begin() noexcept {return make_iterator(0);}
  const_iterator begin() const noexcept {return make_const_iterator(0);}
  const_iterator cbegin() const noexcept {return begin();}
  iterator end() noexcept {return make_iterator(N);}
  const_iterator end() const noexcept {return make_const_iterator(N);}
  const_iterator cend() const noexcept {return end();}
  reverse_iterator rbegin() noexcept {return reverse_iterator(end());}
  const_reverse_iterator rbegin() const noexcept {return const_reverse_iterator(end());}
  const_reverse_iterator crbegin() const noexcept {return rbegin();}
  reverse_iterator rend() noexcept {return reverse_iterator(begin());}
  const_reverse_iterator rend() const noexcept {return const_reverse_iterator(begin());}
  const_reverse_iterator crend() const noexcept {return rend();}
  constexpr bool empty() const noexcept {return N == 0;}
  constexpr size_type size() const noexcept {return N;}
  constexpr size_type max_size() const noexcept {return N;}

  /// @brief Fill all elements.
  /// @param value Value to assign.
  void fill(const value_type & value)
  {
    for (auto & element : *this) {
      element = value;
    }
  }

  /// @brief Swap storage backing and values.
  /// @param other Array to swap with.
  void swap(Array & other) noexcept
  {
    storage_.swap(other.storage_);
  }

private:
  iterator make_iterator(size_type position) noexcept
  {
    if (std::holds_alternative<MemoryRegion>(storage_)) {
      const auto & region = std::get<MemoryRegion>(storage_);
      return iterator(
        static_cast<std::byte *>(region.location.address) + position * sizeof(T),
        sizeof(T));
    }
    return iterator(
      reinterpret_cast<std::byte *>(std::get<InternalStorage>(storage_).data) +
      position * sizeof(T),
      sizeof(T));
  }

  const_iterator make_const_iterator(size_type position) const noexcept
  {
    if (std::holds_alternative<MemoryRegion>(storage_)) {
      const auto & region = std::get<MemoryRegion>(storage_);
      return const_iterator(
        static_cast<const std::byte *>(region.location.address) + position * sizeof(T),
        sizeof(T));
    }
    return const_iterator(
      reinterpret_cast<const std::byte *>(std::get<InternalStorage>(storage_).data) +
      position * sizeof(T),
      sizeof(T));
  }

  std::variant<MemoryRegion, InternalStorage> storage_;
};

}  // namespace rosidl_runtime_cpp

#endif  // ROSIDL_RUNTIME_CPP__EXPERIMENTAL__ARRAY_HPP_
