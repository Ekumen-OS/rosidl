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

#ifndef ROSIDL_RUNTIME_CPP__VIEWS__MESSAGE_VIEW_HPP_
#define ROSIDL_RUNTIME_CPP__VIEWS__MESSAGE_VIEW_HPP_

#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace rosidl_runtime_cpp::views
{

/// Owns a plain message or borrows one for a generated method-based view.
///
/// Borrowed views do not extend the lifetime of the referenced message.
template<typename MessageT, bool Mutable>
class MessageView
{
public:
  using message_type = MessageT;
  using pointer = std::conditional_t<Mutable, MessageT *, const MessageT *>;

  MessageView()
  : owner_(std::make_unique<MessageT>()), message_(owner_.get())
  {}

  explicit MessageView(MessageT && message)
  : owner_(std::make_unique<MessageT>(std::move(message))), message_(owner_.get())
  {}

  explicit MessageView(const MessageT && message)
  : owner_(std::make_unique<MessageT>(message)), message_(owner_.get())
  {}

  template<bool M = Mutable, std::enable_if_t<M, int> = 0>
  explicit MessageView(MessageT & message)
  : message_(&message)
  {}

  template<bool M = Mutable, std::enable_if_t<M, int> = 0>
  explicit MessageView(const MessageT & message)
  : owner_(std::make_unique<MessageT>(message)), message_(owner_.get())
  {}

  template<bool M = Mutable, std::enable_if_t<!M, int> = 0>
  explicit MessageView(const MessageT & message)
  : message_(&message)
  {}

  MessageView(const MessageView &) = delete;
  MessageView & operator=(const MessageView &) = delete;
  MessageView(MessageView && other) noexcept
  : owner_(std::move(other.owner_)), message_(std::exchange(other.message_, nullptr))
  {}

  MessageView & operator=(MessageView && other) noexcept
  {
    if (this != &other) {
      owner_ = std::move(other.owner_);
      message_ = std::exchange(other.message_, nullptr);
    }
    return *this;
  }

  bool owns_message() const noexcept {return static_cast<bool>(owner_);}

protected:
  template<bool M = Mutable, std::enable_if_t<M, int> = 0>
  MessageT & message()
  {
    if (!message_) {
      throw std::logic_error("Access to a moved-from message view");
    }
    return *message_;
  }

  const MessageT & message() const
  {
    if (!message_) {
      throw std::logic_error("Access to a moved-from message view");
    }
    return *message_;
  }

private:
  std::unique_ptr<MessageT> owner_;
  pointer message_{nullptr};
};

/// Non-owning facade; the container must outlive it and any element views.
template<typename ContainerT, typename ViewT, bool Mutable>
class MessageSequenceView
{
public:
  using container_pointer = std::conditional_t<Mutable, ContainerT *, const ContainerT *>;

  class iterator
  {
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = ViewT;
    using difference_type = std::ptrdiff_t;
    using reference = ViewT;
    using pointer = void;

    iterator() = default;

    ViewT operator*() const {return ViewT((*container_)[index_]);}
    iterator & operator++() {++index_; return *this;}

    iterator operator++(int)
    {
      iterator previous = *this;
      ++(*this);
      return previous;
    }

    bool operator==(const iterator & other) const
    {
      return container_ == other.container_ && index_ == other.index_;
    }

    bool operator!=(const iterator & other) const {return !(*this == other);}

private:
    friend class MessageSequenceView;

    iterator(container_pointer container, std::size_t index)
    : container_(container), index_(index)
    {}

    container_pointer container_{nullptr};
    std::size_t index_{0};
  };

  explicit MessageSequenceView(container_pointer container)
  : container_(container)
  {
    if (!container_) {
      throw std::invalid_argument("MessageSequenceView requires a container");
    }
  }

  std::size_t size() const {return container_->size();}
  bool empty() const {return size() == 0;}
  iterator begin() const {return iterator(container_, 0);}
  iterator end() const {return iterator(container_, size());}

  ViewT operator[](std::size_t index) const
  {
    if (index >= size()) {
      throw std::out_of_range("MessageSequenceView index out of range");
    }
    return ViewT((*container_)[index]);
  }

  ViewT at(std::size_t index) const {return (*this)[index];}
  ViewT front() const {return (*this)[0];}
  ViewT back() const {return (*this)[size() - 1];}

private:
  container_pointer container_;
};

}  // namespace rosidl_runtime_cpp::views

#endif  // ROSIDL_RUNTIME_CPP__VIEWS__MESSAGE_VIEW_HPP_
