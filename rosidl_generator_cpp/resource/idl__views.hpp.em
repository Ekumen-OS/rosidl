// generated from rosidl_generator_cpp/resource/idl__views.hpp.em
// with input from @(package_name):@(interface_path)
// generated code does not contain a copyright notice
@{
from rosidl_pycommon import convert_camel_case_to_lower_case_underscore
from rosidl_parser.definition import AbstractSequence
from rosidl_parser.definition import AbstractString
from rosidl_parser.definition import AbstractWString
from rosidl_parser.definition import Array
from rosidl_parser.definition import BoundedSequence
from rosidl_parser.definition import Message
from rosidl_parser.definition import NamespacedType

messages = content.get_elements_of_type(Message)
include_parts = [package_name] + list(interface_path.parents[0].parts) + [
    'views', convert_camel_case_to_lower_case_underscore(interface_path.stem)]
guard = '__'.join(part.upper() for part in include_parts) + '_HPP_'
plain_header = '/'.join(
    [package_name] + list(interface_path.parents[0].parts) +
    [convert_camel_case_to_lower_case_underscore(interface_path.stem)]) + '.hpp'

def nested_view_type(type_):
    return '::' + '::'.join(
        type_.namespaces + ['views', type_.name + 'View<Mutable>'])

def nested_const_view_type(type_):
    return '::' + '::'.join(
        type_.namespaces + ['views', type_.name + 'View<false>'])

def nested_constraints_type(type_):
    return '::' + '::'.join(
        type_.namespaces + ['views', type_.name + 'View<true>', 'Constraints'])

def field_constraint_type(
        type_, AbstractSequence=AbstractSequence, AbstractString=AbstractString,
        AbstractWString=AbstractWString, Array=Array, NamespacedType=NamespacedType,
        nested_constraints_type=nested_constraints_type):
    if isinstance(type_, (AbstractString, AbstractWString)):
        return 'rosidl_runtime_cpp::StringConstraint'
    if isinstance(type_, AbstractSequence):
        element_type = type_.value_type
        if isinstance(element_type, (AbstractString, AbstractWString)):
            return ('rosidl_runtime_cpp::NestedSequenceConstraint<'
                    'rosidl_runtime_cpp::StringConstraint>')
        if isinstance(element_type, NamespacedType):
            nested_type = nested_constraints_type(element_type)
            return 'rosidl_runtime_cpp::NestedSequenceConstraint<' + nested_type + '>'
        return 'rosidl_runtime_cpp::SequenceConstraint'
    if isinstance(type_, Array):
        element_type = type_.value_type
        if isinstance(element_type, (AbstractString, AbstractWString)):
            return 'rosidl_runtime_cpp::StringConstraint'
        if isinstance(element_type, NamespacedType):
            return nested_constraints_type(element_type)
        return None
    if isinstance(type_, NamespacedType):
        return nested_constraints_type(type_)
    return None

nested_headers = set()
for msg in messages:
    for member in msg.structure.members:
        type_ = member.type
        if isinstance(type_, (AbstractSequence, Array)):
            type_ = type_.value_type
        if isinstance(type_, NamespacedType):
            nested_headers.add('/'.join(
                type_.namespaces + [
                    'views', convert_camel_case_to_lower_case_underscore(type_.name) + '.hpp']))
}@
#ifndef @(guard)
#define @(guard)

#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "@(plain_header)"
@[for nested_header in sorted(nested_headers)]@
#include "@(nested_header)"
@[end for]@
#include "rosidl_runtime_cpp/constraints.hpp"
#include "rosidl_runtime_cpp/views/message_view.hpp"

@[for ns in [package_name] + list(interface_path.parents[0].parts)]@
namespace @(ns)
{

@[end for]@
namespace views
{

@[for message in messages]@
@{
message_type = '::' + '::'.join(
    message.structure.namespaced_type.namespaces +
    [message.structure.namespaced_type.name])
}@
template<bool Mutable = true>
class @(message.structure.namespaced_type.name)View
  : public rosidl_runtime_cpp::views::MessageView<
    @(message_type), Mutable>
{
  using Base = rosidl_runtime_cpp::views::MessageView<
    @(message_type), Mutable>;

public:
  using Base::Base;

  struct Constraints
  {
@{
constraint_members = [
    (member, field_constraint_type(member.type))
    for member in message.structure.members
]
constraint_members = [(member, type_name) for member, type_name in constraint_members if type_name]
validation_fields = []
for member in message.structure.members:
    type_ = member.type
    if isinstance(type_, (AbstractString, AbstractWString)):
        validation_fields.append(
            (member, 'bounded_string' if type_.has_maximum_size() else 'string'))
    elif isinstance(type_, AbstractSequence):
        element_type = type_.value_type
        if isinstance(element_type, (AbstractString, AbstractWString)):
            kind = 'sequence_bounded_string' if element_type.has_maximum_size() \
                else 'sequence_string'
        elif isinstance(element_type, NamespacedType):
            kind = 'sequence_nested'
        else:
            kind = 'sequence'
        validation_fields.append((member, kind))
    elif isinstance(type_, Array):
        element_type = type_.value_type
        if isinstance(element_type, (AbstractString, AbstractWString)):
            kind = 'array_bounded_string' if element_type.has_maximum_size() else 'array_string'
        elif isinstance(element_type, NamespacedType):
            kind = 'array_nested'
        else:
            kind = None
        if kind:
            validation_fields.append((member, kind))
    elif isinstance(type_, NamespacedType):
        validation_fields.append((member, 'nested'))
needs_item_path = any(
    kind in ('sequence_string', 'sequence_bounded_string', 'sequence_nested',
             'array_string', 'array_bounded_string', 'array_nested')
    for _, kind in validation_fields)
}@
@[for member, type_name in constraint_members]@
    @(type_name) @(member.name){};
@[end for]@
@[if constraint_members]@

@[end if]@
    bool operator==(const Constraints & other) const
    {
@[if not constraint_members]@
      (void)other;
      return true;
@[else]@
      return
@[for i, (member, _) in enumerate(constraint_members)]@
        @(member.name) == other.@(member.name)
@[if i < len(constraint_members) - 1]@
        &&
@[end if]@
@[end for]@
      ;
@[end if]@
    }

    bool operator!=(const Constraints & other) const {return !(*this == other);}

    bool CheckCompatible(
      const Constraints & baseline,
      rosidl_runtime_cpp::ConstraintReportCallback report = {},
      std::string_view prefix = "") const
    {
@[if not constraint_members]@
      (void)baseline;
      (void)report;
      (void)prefix;
@[else]@
      std::string field_path;
@[end if]@
@[for member, _ in constraint_members]@
      field_path.assign(prefix);
      if (!field_path.empty()) {
        field_path += '.';
      }
      field_path += "@(member.name)";
@{
type_ = member.type
bound_checks = []
if isinstance(type_, (AbstractString, AbstractWString)) and type_.has_maximum_size():
    bound_checks.append((member.name + '.size', type_.maximum_size))
elif isinstance(type_, (AbstractSequence, Array)):
    if isinstance(type_, BoundedSequence):
        bound_checks.append((member.name + '.size', type_.maximum_size))
    element = type_.value_type
    if isinstance(element, (AbstractString, AbstractWString)) and element.has_maximum_size():
        suffix = '.element.size' if isinstance(type_, AbstractSequence) else '.size'
        bound_checks.append((member.name + suffix, element.maximum_size))
}@
@[for expression, maximum in bound_checks]@
      if (@(expression) > @(maximum)) {
        if (report) {
          report(field_path, 0);
        }
        return false;
      }
@[end for]@
      if (!@(member.name).CheckCompatible(baseline.@(member.name), report, field_path)) {
        return false;
      }
@[end for]@
      return true;
    }

@[for region_validation in (False, True)]@
@[if region_validation]@
    template<typename MessageViewT>
    bool CheckCompatibleView(
      const MessageViewT & message,
@[else]@
    bool CheckCompatible(
      const @(message.structure.namespaced_type.name) & message,
@[end if]@
      rosidl_runtime_cpp::ConstraintReportCallback report = {},
      std::string_view prefix = "") const
    {
@[if not validation_fields]@
      (void)message;
      (void)report;
      (void)prefix;
@[else]@
      std::string field_path;
@[if needs_item_path]@
      std::string item_path;
@[end if]@
@[for member, kind in validation_fields]@
      field_path.assign(prefix);
      if (!field_path.empty()) {
        field_path += '.';
      }
      field_path += "@(member.name)";
@[if kind == 'string']@
      if (@(member.name).size != 0 && message.@(member.name)@('()' if region_validation else '').size() > @(member.name).size) {
        if (report) {
          report(field_path, 0);
        }
        return false;
      }
@[elif kind == 'bounded_string']@
      if (message.@(member.name)@('()' if region_validation else '').size() > @(member.type.maximum_size) ||
        (@(member.name).size != 0 && message.@(member.name)@('()' if region_validation else '').size() > @(member.name).size))
      {
        if (report) {
          report(field_path, 0);
        }
        return false;
      }
@[elif kind in ('sequence', 'sequence_string', 'sequence_bounded_string', 'sequence_nested')]@
      if (@(member.name).size != 0 && message.@(member.name)@('()' if region_validation else '').size() > @(member.name).size) {
        if (report) {
          report(field_path, 0);
        }
        return false;
      }
@[if kind == 'sequence_string']@
      if (@(member.name).element.size != 0) {
        for (std::size_t i = 0; i < message.@(member.name)@('()' if region_validation else '').size(); ++i) {
          if (message.@(member.name)@('()' if region_validation else '')[i].size() > @(member.name).element.size) {
            if (report) {
              item_path = field_path + "[" + std::to_string(i) + "]";
              report(item_path, 0);
            }
            return false;
          }
        }
      }
@[elif kind == 'sequence_bounded_string']@
      for (std::size_t i = 0; i < message.@(member.name)@('()' if region_validation else '').size(); ++i) {
        if (message.@(member.name)@('()' if region_validation else '')[i].size() > @(member.type.value_type.maximum_size) ||
          (@(member.name).element.size != 0 && message.@(member.name)@('()' if region_validation else '')[i].size() > @(member.name).element.size))
        {
          if (report) {
            item_path = field_path + "[" + std::to_string(i) + "]";
            report(item_path, 0);
          }
          return false;
        }
      }
@[elif kind == 'sequence_nested']@
      for (std::size_t i = 0; i < message.@(member.name)@('()' if region_validation else '').size(); ++i) {
        item_path = field_path + "[" + std::to_string(i) + "]";
        if (!@(member.name).element.@('CheckCompatibleView' if region_validation else 'CheckCompatible')(message.@(member.name)@('()' if region_validation else '')[i], report, item_path)) {
          return false;
        }
      }
@[end if]@
@[elif kind == 'array_string']@
      if (@(member.name).size != 0) {
        for (std::size_t i = 0; i < message.@(member.name)@('()' if region_validation else '').size(); ++i) {
          if (message.@(member.name)@('()' if region_validation else '')[i].size() > @(member.name).size) {
            if (report) {
              item_path = field_path + "[" + std::to_string(i) + "]";
              report(item_path, 0);
            }
            return false;
          }
        }
      }
@[elif kind == 'array_bounded_string']@
      for (std::size_t i = 0; i < message.@(member.name)@('()' if region_validation else '').size(); ++i) {
        if (message.@(member.name)@('()' if region_validation else '')[i].size() > @(member.type.value_type.maximum_size) ||
          (@(member.name).size != 0 && message.@(member.name)@('()' if region_validation else '')[i].size() > @(member.name).size))
        {
          if (report) {
            item_path = field_path + "[" + std::to_string(i) + "]";
            report(item_path, 0);
          }
          return false;
        }
      }
@[elif kind == 'nested']@
      if (!@(member.name).@('CheckCompatibleView' if region_validation else 'CheckCompatible')(message.@(member.name)@('()' if region_validation else ''), report, field_path)) {
        return false;
      }
@[elif kind == 'array_nested']@
      for (std::size_t i = 0; i < message.@(member.name)@('()' if region_validation else '').size(); ++i) {
        item_path = field_path + "[" + std::to_string(i) + "]";
        if (!@(member.name).@('CheckCompatibleView' if region_validation else 'CheckCompatible')(message.@(member.name)@('()' if region_validation else '')[i], report, item_path)) {
          return false;
        }
      }
@[end if]@
@[end for]@
@[end if]@
      return true;
    }
@[end for]@
  };

@[for i, member in enumerate(message.structure.members)]@
@{
is_nested = isinstance(member.type, NamespacedType)
nested_view = nested_view_type(member.type) if is_nested else None
nested_const_view = nested_const_view_type(member.type) if is_nested else None
is_nested_container = isinstance(member.type, (AbstractSequence, Array)) and \
    isinstance(member.type.value_type, NamespacedType)
if is_nested_container:
    nested_view = nested_view_type(member.type.value_type)
    nested_const_view = nested_const_view_type(member.type.value_type)
    message_type = '::' + '::'.join(
        message.structure.namespaced_type.namespaces +
        [message.structure.namespaced_type.name])
    container_type = 'decltype(std::declval<' + message_type + '>().' + member.name + ')'
}@
@[if is_nested_container]@
  auto @(member.name)()
  {
    using FieldView = rosidl_runtime_cpp::views::MessageSequenceView<
      @(container_type), @(nested_view), Mutable>;
    return FieldView(&this->message().@(member.name));
  }

  auto @(member.name)() const
  {
    using FieldView = rosidl_runtime_cpp::views::MessageSequenceView<
      @(container_type), @(nested_const_view), false>;
    return FieldView(&this->message().@(member.name));
  }

  template<typename Value, bool M = Mutable, typename std::enable_if<M, int>::type = 0>
  void @(member.name)(Value && value)
  {
    this->message().@(member.name) = std::forward<Value>(value);
  }
@[elif is_nested]@
  @(nested_view) @(member.name)()
  {
    return @(nested_view)(this->message().@(member.name));
  }

  @(nested_const_view) @(member.name)() const
  {
    return @(nested_const_view)(this->message().@(member.name));
  }

  template<typename Value, bool M = Mutable, typename std::enable_if<M, int>::type = 0>
  void @(member.name)(Value && value)
  {
    this->message().@(member.name) = std::forward<Value>(value);
  }
@[else]@
  auto & @(member.name)()
  {
    return this->message().@(member.name);
  }

  const auto & @(member.name)() const
  {
    return this->message().@(member.name);
  }

  template<typename Value, bool M = Mutable, typename std::enable_if<M, int>::type = 0>
  void @(member.name)(Value && value)
  {
    this->message().@(member.name) = std::forward<Value>(value);
  }
@[end if]@
@[if i < len(message.structure.members) - 1]@

@[end if]@
@[end for]@
};

using @(message.structure.namespaced_type.name) =
  @(message.structure.namespaced_type.name)View<true>;
using Const@(message.structure.namespaced_type.name) =
  @(message.structure.namespaced_type.name)View<false>;

@[end for]@
}  // namespace views
@[for ns in reversed([package_name] + list(interface_path.parents[0].parts))]@

}  // namespace @(ns)
@[end for]@

#endif  // @(guard)
