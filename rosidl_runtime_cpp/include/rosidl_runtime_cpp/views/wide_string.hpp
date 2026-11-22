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

#ifndef ROSIDL_RUNTIME_CPP__VIEWS__WIDE_STRING_HPP_
#define ROSIDL_RUNTIME_CPP__VIEWS__WIDE_STRING_HPP_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "rosidl_runtime_cpp/views/region_storage.hpp"

namespace rosidl_runtime_cpp::views
{

/// UTF-16 code units backed by 32-bit slots rather than contiguous char16_t storage.
/// References and substrings borrow storage and are invalidated when it is replaced or released.
template<std::size_t UpperBound = 0>
class WideString
{
public:
  using value_type = char16_t;
  using size_type = std::size_t;
  using ExternalStorage = StringStorage<std::uint32_t>;

  class reference
  {
public:
    explicit reference(std::uint32_t * slot)
    : slot_(slot) {}
    operator char16_t() const {return static_cast<char16_t>(*slot_);}
    reference & operator=(char16_t value) {*slot_ = value; return *this;}
    reference & operator=(const reference & other) {return *this = static_cast<char16_t>(other);}

private:
    std::uint32_t * slot_;
  };

  template<bool Const>
  class Iterator
  {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = char16_t;
    using difference_type = std::ptrdiff_t;
    using reference = std::conditional_t<Const, char16_t, typename WideString::reference>;
    using pointer = void;
    using slot_type = std::conditional_t<Const, const std::uint32_t, std::uint32_t>;

    Iterator() = default;
    explicit Iterator(slot_type * slot)
    : slot_(slot) {}
    reference operator*() const
    {
      if constexpr (Const) {
        return static_cast<char16_t>(*slot_);
      } else {
        return typename WideString::reference(slot_);
      }
    }
    Iterator & operator++() {++slot_; return *this;}
    Iterator operator++(int) {auto old = *this; ++*this; return old;}
    bool operator==(const Iterator & other) const {return slot_ == other.slot_;}
    bool operator!=(const Iterator & other) const {return !(*this == other);}

private:
    slot_type * slot_{nullptr};
  };

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;

  template<bool Const>
  class Substring
  {
public:
    using slot_type = std::conditional_t<Const, const std::uint32_t, std::uint32_t>;
    using iterator_type = Iterator<Const>;
    using reference_type = std::conditional_t<Const, char16_t, typename WideString::reference>;
    Substring(slot_type * data, size_type size)
    : data_(data), size_(size) {}
    size_type size() const noexcept {return size_;}
    bool empty() const noexcept {return size_ == 0;}
    reference_type operator[](size_type index) const
    {
      if constexpr (Const) {
        return static_cast<char16_t>(data_[index]);
      } else {
        return typename WideString::reference(data_ + index);
      }
    }
    iterator_type begin() const {return iterator_type(data_);}
    iterator_type end() const {return iterator_type(size_ ? data_ + size_ : data_);}

private:
    slot_type * data_;
    size_type size_;
  };

  WideString() = default;
  WideString(std::u16string_view value)  // NOLINT(runtime/explicit)
  {
    assign(value);
  }
  WideString(const char16_t * value)  // NOLINT(runtime/explicit)
  {
    assign(value);
  }
  explicit WideString(const ExternalStorage & storage)
  : external_(storage.region), external_storage_(true), size_(storage.size.value_or(0))
  {
    if (!external_ || external_.size() % sizeof(std::uint32_t) != 0 ||
      reinterpret_cast<std::uintptr_t>(external_.data()) % alignof(std::uint32_t) != 0)
    {
      throw std::invalid_argument("Invalid wide string storage");
    }
    if (size_ > capacity()) {
      throw std::length_error("Wide string size exceeds storage");
    }
    for (size_type i = 0; i < size_; ++i) {
      if (slots()[i] > std::numeric_limits<char16_t>::max()) {
        throw std::range_error("Wide string wire value exceeds UTF-16 code unit");
      }
    }
  }

  WideString(const WideString & other)
  {
    assign_slots(other);
  }
  WideString & operator=(const WideString & other)
  {
    if (this != &other) {assign_slots(other);}
    return *this;
  }
  WideString(WideString && other) noexcept
  : external_(other.external_), owned_(std::move(other.owned_)),
    external_storage_(other.external_storage_), size_(other.size_)
  {
    other.external_ = {};
    other.external_storage_ = false;
    other.size_ = 0;
  }
  WideString & operator=(WideString && other)
  {
    if (this == &other) {return *this;}
    if (external_storage_) {
      assign_slots(other);
    } else {
      external_ = other.external_;
      owned_ = std::move(other.owned_);
      external_storage_ = other.external_storage_;
      size_ = other.size_;
      other.external_ = {};
      other.external_storage_ = false;
      other.size_ = 0;
    }
    return *this;
  }

  WideString & operator=(std::u16string_view value) {assign(value); return *this;}
  WideString & operator=(const char16_t * value) {assign(value); return *this;}

  size_type size() const noexcept {return size_;}
  bool empty() const noexcept {return size_ == 0;}
  size_type capacity() const noexcept
  {
    const auto capacity = external_storage_ ? external_.capacity() : owned_.size();
    return UpperBound ? std::min(capacity, UpperBound) : capacity;
  }

  reference operator[](size_type index) {return reference(slots() + index);}
  char16_t operator[](size_type index) const {return static_cast<char16_t>(slots()[index]);}
  reference at(size_type index)
  {
    if (index >= size_) {throw std::out_of_range("Wide string index out of range");}
    return (*this)[index];
  }
  char16_t at(size_type index) const
  {
    if (index >= size_) {throw std::out_of_range("Wide string index out of range");}
    return (*this)[index];
  }

  iterator begin() {return iterator(slots());}
  iterator end() {return iterator(size_ ? slots() + size_ : slots());}
  const_iterator begin() const {return const_iterator(slots());}
  const_iterator end() const {return const_iterator(size_ ? slots() + size_ : slots());}

  Substring<false> substr(size_type pos, size_type count = std::string_view::npos)
  {
    if (pos > size_) {throw std::out_of_range("Wide string position out of range");}
    return Substring<false>(pos ? slots() + pos : slots(), std::min(count, size_ - pos));
  }
  Substring<true> substr(size_type pos, size_type count = std::string_view::npos) const
  {
    if (pos > size_) {throw std::out_of_range("Wide string position out of range");}
    return Substring<true>(pos ? slots() + pos : slots(), std::min(count, size_ - pos));
  }

  void clear() noexcept {size_ = 0;}
  void resize(size_type size)
  {
    ensure_capacity(size);
    for (size_type i = size_; i < size; ++i) {slots()[i] = 0;}
    size_ = size;
  }
  void push_back(char16_t value)
  {
    ensure_capacity(size_ + 1);
    slots()[size_++] = value;
  }
  void assign(std::u16string_view value)
  {
    ensure_capacity(value.size());
    for (size_type i = 0; i < value.size(); ++i) {slots()[i] = value[i];}
    size_ = value.size();
  }
  std::u16string to_string() const
  {
    std::u16string result;
    result.reserve(size_);
    for (char16_t value : *this) {result.push_back(value);}
    return result;
  }

private:
  void assign_slots(const WideString & other)
  {
    ensure_capacity(other.size_);
    if (other.size_) {
      std::memmove(slots(), other.slots(), other.size_ * sizeof(std::uint32_t));
    }
    size_ = other.size_;
  }
  std::uint32_t * slots() {return external_storage_ ? external_.data() : owned_.data();}
  const std::uint32_t * slots() const
  {
    return external_storage_ ? external_.data() : owned_.data();
  }
  void ensure_capacity(size_type size)
  {
    if ((UpperBound && size > UpperBound) || (external_storage_ && size > capacity())) {
      throw std::length_error("Wide string capacity exceeded");
    }
    if (!external_storage_ && size > owned_.size()) {owned_.resize(size);}
  }

  MemoryRegion<std::uint32_t> external_{};
  std::vector<std::uint32_t> owned_{};
  bool external_storage_{false};
  size_type size_{0};
};

}  // namespace rosidl_runtime_cpp::views

#endif  // ROSIDL_RUNTIME_CPP__VIEWS__WIDE_STRING_HPP_
