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

#ifndef ROSIDL_RUNTIME_CPP__VIEWS__VIEW_HANDLE_HPP_
#define ROSIDL_RUNTIME_CPP__VIEWS__VIEW_HANDLE_HPP_

#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "rosidl_runtime_c/memory.h"

namespace rosidl_runtime_cpp::views
{

/// Owns a view over externally managed regions, but never owns the regions.
/**
 * The supplied regions must describe all storage accessed by the view, and
 * their owner must keep them alive until the view is destroyed or released.
 * The view's destructor must not deallocate that storage. References obtained
 * from the view or regions() must not be used after either terminal operation
 * or after moving the handle.
 */
template<typename ViewT>
class ViewHandle
{
public:
  using Regions = std::vector<rosidl_memory_region_t>;

  ViewHandle(std::unique_ptr<ViewT> view, Regions regions)
  : regions_(std::move(regions)), view_(std::move(view))
  {
    if (!view_ || regions_.empty()) {
      throw std::invalid_argument("ViewHandle requires a view and at least one region");
    }
    for (const auto & region : regions_) {
      if (!region.location.address && region.size != 0) {
        throw std::invalid_argument("ViewHandle region has no address");
      }
    }
  }

  ViewHandle(const ViewHandle &) = delete;
  ViewHandle & operator=(const ViewHandle &) = delete;
  ViewHandle(ViewHandle &&) noexcept = default;

  ViewHandle & operator=(ViewHandle && other) noexcept
  {
    if (this != &other) {
      view_.reset();
      regions_ = std::move(other.regions_);
      view_ = std::move(other.view_);
    }
    return *this;
  }

  ViewT & view()
  {
    require_view();
    return *view_;
  }

  const ViewT & view() const
  {
    require_view();
    return *view_;
  }

  const Regions & regions() const
  {
    require_view();
    return regions_;
  }

  void destroy()
  {
    require_view();
    view_.reset();
    regions_.clear();
  }

  Regions release()
  {
    require_view();
    view_.reset();
    return std::move(regions_);
  }

private:
  void require_view() const
  {
    if (!view_) {
      throw std::logic_error("ViewHandle has already been consumed or moved");
    }
  }

  Regions regions_;
  std::unique_ptr<ViewT> view_;
};

}  // namespace rosidl_runtime_cpp::views

#endif  // ROSIDL_RUNTIME_CPP__VIEWS__VIEW_HANDLE_HPP_
