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

#ifndef ROSIDL_RUNTIME_CPP__CONSTRAINTS_HPP_
#define ROSIDL_RUNTIME_CPP__CONSTRAINTS_HPP_

#include <cstddef>
#include <functional>
#include <string_view>

namespace rosidl_runtime_cpp
{

using ConstraintReportCallback =
  std::function<void(std::string_view field_path, int reason_code)>;

// A zero size adds no limit; callers must retain and enforce the baseline
// separately when applying compatible per-operation constraints.
struct StringConstraint
{
  std::size_t size{0};

  bool operator==(const StringConstraint & other) const {return size == other.size;}
  bool operator!=(const StringConstraint & other) const {return !(*this == other);}

  bool CheckCompatible(
    const StringConstraint & baseline,
    const ConstraintReportCallback & report = {},
    std::string_view field_path = "") const
  {
    return check_size(size, baseline.size, report, field_path);
  }

private:
  static bool check_size(
    std::size_t requested, std::size_t baseline,
    const ConstraintReportCallback & report, std::string_view field_path)
  {
    if (baseline != 0 && requested > baseline) {
      if (report) {
        report(field_path, 0);
      }
      return false;
    }
    return true;
  }
};

struct SequenceConstraint
{
  std::size_t size{0};

  bool operator==(const SequenceConstraint & other) const {return size == other.size;}
  bool operator!=(const SequenceConstraint & other) const {return !(*this == other);}

  bool CheckCompatible(
    const SequenceConstraint & baseline,
    const ConstraintReportCallback & report = {},
    std::string_view field_path = "") const
  {
    if (baseline.size != 0 && size > baseline.size) {
      if (report) {
        report(field_path, 0);
      }
      return false;
    }
    return true;
  }
};

template<typename ElementConstraint>
struct NestedSequenceConstraint
{
  std::size_t size{0};
  ElementConstraint element{};

  bool operator==(const NestedSequenceConstraint & other) const
  {
    return size == other.size && element == other.element;
  }

  bool operator!=(const NestedSequenceConstraint & other) const {return !(*this == other);}

  bool CheckCompatible(
    const NestedSequenceConstraint & baseline,
    const ConstraintReportCallback & report = {},
    std::string_view field_path = "") const
  {
    if (baseline.size != 0 && size > baseline.size) {
      if (report) {
        report(field_path, 0);
      }
      return false;
    }
    const std::string_view element_path =
      field_path.empty() ? std::string_view("element") : field_path;
    return element.CheckCompatible(baseline.element, report, element_path);
  }
};

}  // namespace rosidl_runtime_cpp

#endif  // ROSIDL_RUNTIME_CPP__CONSTRAINTS_HPP_
