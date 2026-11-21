// generated from rosidl_generator_cpp/resource/idl__region_views.hpp.em
// with input from @(package_name):@(interface_path)
// generated code does not contain a copyright notice
@{
from rosidl_generator_cpp import MSG_TYPE_TO_CPP
from rosidl_parser.definition import AbstractSequence, AbstractString, AbstractWString
from rosidl_parser.definition import Array, BasicType, Message, NamespacedType
from rosidl_pycommon import convert_camel_case_to_lower_case_underscore

messages = content.get_elements_of_type(Message)
stem = convert_camel_case_to_lower_case_underscore(interface_path.stem)
parts = [package_name] + list(interface_path.parents[0].parts)
guard = '__'.join(part.upper() for part in parts + ['views', stem]) + '__REGION_HPP_'

def element_type(t, BasicType=BasicType, AbstractString=AbstractString,
                 AbstractWString=AbstractWString, NamespacedType=NamespacedType,
                 MSG_TYPE_TO_CPP=MSG_TYPE_TO_CPP):
    if isinstance(t, BasicType):
        return MSG_TYPE_TO_CPP[t.typename]
    if isinstance(t, (AbstractString, AbstractWString)):
        char = 'char16_t' if isinstance(t, AbstractWString) else 'char'
        bound = t.maximum_size if t.has_maximum_size() else 0
        return 'rosidl_runtime_cpp::views::BasicString<{}, {}>'.format(char, bound)
    if isinstance(t, NamespacedType):
        return '::' + '::'.join(t.namespaces + ['views', t.name + 'RegionView<Mutable>'])
    raise ValueError('unsupported region element: {}'.format(t))

def field_type(t, BasicType=BasicType, Array=Array, AbstractSequence=AbstractSequence,
               AbstractString=AbstractString, AbstractWString=AbstractWString,
               element_type=element_type):
    if isinstance(t, BasicType):
        return 'rosidl_runtime_cpp::views::Scalar<{}>'.format(element_type(t))
    if isinstance(t, Array):
        return 'rosidl_runtime_cpp::views::Array<{}, {}>'.format(element_type(t.value_type), t.size)
    if isinstance(t, AbstractSequence):
        bound = t.maximum_size if t.has_maximum_size() else 0
        if isinstance(t.value_type, (AbstractString, AbstractWString)):
            char = 'char16_t' if isinstance(t.value_type, AbstractWString) else 'char'
            return 'rosidl_runtime_cpp::views::BasicSequence<{}, {}, rosidl_runtime_cpp::views::StringStorage<{}>>'.format(
                element_type(t.value_type), bound, char)
        return 'rosidl_runtime_cpp::views::BasicSequence<{}, {}>'.format(
            element_type(t.value_type), bound)
    return element_type(t)

def storage_element_type(t, BasicType=BasicType, AbstractString=AbstractString,
                         AbstractWString=AbstractWString, NamespacedType=NamespacedType,
                         element_type=element_type):
    if isinstance(t, BasicType):
        return 'rosidl_runtime_cpp::Memory<{}>'.format(element_type(t))
    if isinstance(t, (AbstractString, AbstractWString)):
        char = 'char16_t' if isinstance(t, AbstractWString) else 'char'
        return 'rosidl_runtime_cpp::views::StringStorage<{}>'.format(char)
    if isinstance(t, NamespacedType):
        return '::' + '::'.join(t.namespaces + ['views', t.name + 'RegionStorage'])
    raise ValueError('unsupported region storage: {}'.format(t))

def storage_type(t, BasicType=BasicType, Array=Array, AbstractSequence=AbstractSequence,
                 element_type=element_type, storage_element_type=storage_element_type):
    if isinstance(t, Array):
        if isinstance(t.value_type, BasicType):
            return 'rosidl_runtime_cpp::MemoryRegion<{}>'.format(element_type(t.value_type))
        return 'std::array<{}, {}>'.format(storage_element_type(t.value_type), t.size)
    if isinstance(t, AbstractSequence):
        if isinstance(t.value_type, BasicType):
            return 'rosidl_runtime_cpp::views::PrimitiveSequenceStorage<{}>'.format(
                element_type(t.value_type))
        return 'rosidl_runtime_cpp::views::ElementSequenceStorage<{}>'.format(
            storage_element_type(t.value_type))
    return storage_element_type(t)

def storage_init(member, Array=Array, BasicType=BasicType, AbstractSequence=AbstractSequence):
    t = member.type
    name = member.name
    source = 'storage.members.' + name
    if isinstance(t, Array) and not isinstance(t.value_type, BasicType):
        args = ', '.join('std::make_tuple({}[{}])'.format(source, i) for i in range(t.size))
        return '_{}(std::piecewise_construct, {})'.format(name, args)
    if isinstance(t, AbstractSequence):
        value = 'region' if isinstance(t.value_type, BasicType) else 'elements'
        return '_{}({}.{}, {}.size)'.format(name, source, value, source)
    return '_{}({})'.format(name, source)

nested_headers = set()
for msg in messages:
    for member in msg.structure.members:
        t = member.type.value_type if isinstance(member.type, (Array, AbstractSequence)) else member.type
        if isinstance(t, NamespacedType):
            nested_headers.add('/'.join(t.namespaces + [
                'views', convert_camel_case_to_lower_case_underscore(t.name) + '__region.hpp']))
}@
#ifndef @(guard)
#define @(guard)

#include <array>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "@('/'.join(parts))/views/@(stem).hpp"
@[for header in sorted(nested_headers)]@
#include "@(header)"
@[end for]@
#include "rosidl_runtime_cpp/views/array.hpp"
#include "rosidl_runtime_cpp/views/region_storage.hpp"
#include "rosidl_runtime_cpp/views/scalar.hpp"
#include "rosidl_runtime_cpp/views/sequence.hpp"
#include "rosidl_runtime_cpp/views/string.hpp"

@[for ns in parts]@
namespace @(ns)
{

@[end for]@
namespace views
{

@[for message in messages]@
@{
name = message.structure.namespaced_type.name
storage_initializers = []
for member in message.structure.members:
    storage_initializers.append(storage_init(member))
}@
struct @(name)RegionStorage
{
  rosidl_runtime_cpp::MemoryRegion<void> block{};
  bool prepopulated{false};
  struct Members
  {
@[for member in message.structure.members]@
    @(storage_type(member.type)) @(member.name){};
@[end for]@
  } members;
};

template<bool Mutable = true>
class @(name)RegionView
{
public:
  using ExternalStorage = @(name)RegionStorage;
  using Constraints = typename @(name)View<true>::Constraints;
  using PlainMessage = ::@('::'.join(parts + [name]));

  @(name)RegionView()
  {
    assign_plain(PlainMessage{});
  }

  /// Borrows the supplied regions; they must outlive the view and its references.
  explicit @(name)RegionView(const ExternalStorage & storage)
@[if message.structure.members]@
  : @(',\n    '.join(storage_initializers)),
    _storage(storage), _owns(false)
@[else]@
  : _storage(storage), _owns(false)
@[end if]@
  {}

  @(name)RegionView(const @(name)RegionView &) = delete;
  @(name)RegionView & operator=(const @(name)RegionView &) = delete;
  @(name)RegionView(@(name)RegionView && other)
@[if message.structure.members]@
  : @(',\n    '.join('_{}(std::move(other._{}))'.format(m.name, m.name) for m in message.structure.members)),
    _storage(std::move(other._storage)),
@[else]@
  : _storage(std::move(other._storage)),
@[end if]@
    _valid(std::exchange(other._valid, false)), _owns(other._owns)
  {}

  bool owns_storage() const {require_valid(); return _owns;}
  rosidl_runtime_cpp::MemoryRegion<void> backing_storage() const
  {
    require_valid();
    return _storage ? _storage->block : rosidl_runtime_cpp::MemoryRegion<void>{};
  }

@[for member in message.structure.members]@
  auto & @(member.name)()
  {
    require_valid();
    if constexpr (Mutable) {
      return _@(member.name)@('.get()' if isinstance(member.type, BasicType) else '');
    } else {
      return std::as_const(_@(member.name))@('.get()' if isinstance(member.type, BasicType) else '');
    }
  }
  const auto & @(member.name)() const
  {
    require_valid();
    return _@(member.name)@('.get()' if isinstance(member.type, BasicType) else '');
  }

  template<typename Value, bool M = Mutable,
    std::enable_if_t<M && std::is_assignable_v<@(field_type(member.type)) &, Value &&>, int> = 0>
  void @(member.name)(Value && value)
  {
    require_valid();
    _@(member.name) = std::forward<Value>(value);
  }

@[end for]@
private:
  friend struct rosidl_runtime_cpp::views::detail::RegionViewAccess;

  void require_valid() const
  {
    if (!_valid) {
      throw std::logic_error("Access to a moved-from region view");
    }
  }

  void assign_plain(const PlainMessage & message)
  {
@[for member in message.structure.members]@
@{t = member.type}@
@[if isinstance(t, BasicType)]@
    _@(member.name) = message.@(member.name);
@[elif isinstance(t, (AbstractString, AbstractWString))]@
    _@(member.name).assign(std::basic_string_view<@('char16_t' if isinstance(t, AbstractWString) else 'char')>(
      message.@(member.name).data(), message.@(member.name).size()));
@[elif isinstance(t, NamespacedType)]@
    rosidl_runtime_cpp::views::detail::RegionViewAccess::assign(_@(member.name), message.@(member.name));
@[elif isinstance(t, Array)]@
    for (std::size_t i = 0; i < @(t.size); ++i) {
@[if isinstance(t.value_type, BasicType)]@
      _@(member.name)[i] = message.@(member.name)[i];
@[elif isinstance(t.value_type, (AbstractString, AbstractWString))]@
      _@(member.name)[i].assign(std::basic_string_view<@('char16_t' if isinstance(t.value_type, AbstractWString) else 'char')>(
        message.@(member.name)[i].data(), message.@(member.name)[i].size()));
@[else]@
      rosidl_runtime_cpp::views::detail::RegionViewAccess::assign(_@(member.name)[i], message.@(member.name)[i]);
@[end if]@
    }
@[elif isinstance(t, AbstractSequence)]@
@[if isinstance(t.value_type, BasicType)]@
    _@(member.name).assign(message.@(member.name).begin(), message.@(member.name).end());
@[else]@
    _@(member.name).clear();
    for (const auto & element : message.@(member.name)) {
      auto & target = _@(member.name).emplace_back();
@[if isinstance(t.value_type, (AbstractString, AbstractWString))]@
      target.assign(std::basic_string_view<@('char16_t' if isinstance(t.value_type, AbstractWString) else 'char')>(
        element.data(), element.size()));
@[else]@
      rosidl_runtime_cpp::views::detail::RegionViewAccess::assign(target, element);
@[end if]@
    }
@[end if]@
@[end if]@
@[end for]@
  }

@[for member in message.structure.members]@
  @(field_type(member.type)) _@(member.name){};
@[end for]@
  std::optional<ExternalStorage> _storage;
  bool _valid{true};
  bool _owns{true};
};

using Region@(name) = @(name)RegionView<true>;
using ConstRegion@(name) = @(name)RegionView<false>;

@[end for]@
}  // namespace views
@[for ns in reversed(parts)]@
}  // namespace @(ns)
@[end for]@

#endif  // @(guard)
