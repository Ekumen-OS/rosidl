@# Included from rosidl_generator_cpp/resource/idl__experimental_struct.hpp.em
@{
from rosidl_generator_cpp import BASIC_TYPE_TO_EXPERIMENTAL_CPP
from rosidl_generator_cpp import EXPERIMENTAL_CHARACTER_TYPES
from rosidl_generator_cpp import escape_string
from rosidl_generator_cpp import escape_wstring
from rosidl_generator_cpp import experimental_member_needs_pmr
from rosidl_generator_cpp import experimental_pmr_init_expr
from rosidl_generator_cpp import msg_type_to_experimental_cpp
from rosidl_parser.definition import AbstractNestedType
from rosidl_parser.definition import AbstractString
from rosidl_parser.definition import AbstractWString
from rosidl_parser.definition import ACTION_FEEDBACK_SUFFIX
from rosidl_parser.definition import ACTION_GOAL_SUFFIX
from rosidl_parser.definition import ACTION_RESULT_SUFFIX
from rosidl_parser.definition import Array
from rosidl_parser.definition import BasicType
from rosidl_parser.definition import BOOLEAN_TYPE
from rosidl_parser.definition import CHARACTER_TYPES
from rosidl_parser.definition import EMPTY_STRUCTURE_REQUIRED_MEMBER_NAME
from rosidl_parser.definition import INTEGER_TYPES
from rosidl_parser.definition import NamespacedType
from rosidl_parser.definition import OCTET_TYPE
from rosidl_parser.definition import SERVICE_REQUEST_MESSAGE_SUFFIX
from rosidl_parser.definition import SERVICE_RESPONSE_MESSAGE_SUFFIX
from rosidl_parser.definition import UNSIGNED_INTEGER_TYPES

message_typename = '::'.join(
    list(message.structure.namespaced_type.namespaces) +
    ['experimental', message.structure.namespaced_type.name])

msvc_common_macros = ('DELETE', 'ERROR', 'NO_ERROR')
}@
@
@#<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
@# Collect include directives for sub-message member types
@{
from collections import OrderedDict
from rosidl_pycommon import convert_camel_case_to_lower_case_underscore
includes = OrderedDict()
for member in message.structure.members:
    type_ = member.type
    if isinstance(type_, AbstractNestedType):
        type_ = type_.value_type
    if isinstance(type_, NamespacedType):
        if (
            message.structure.namespaced_type.namespaces[-1] in ['action', 'srv'] and (
            type_.name.endswith(SERVICE_REQUEST_MESSAGE_SUFFIX) or
            type_.name.endswith(SERVICE_RESPONSE_MESSAGE_SUFFIX))
        ):
            typename = type_.name.rsplit('_', 1)[0]
            if typename == message.structure.namespaced_type.name.rsplit('_', 1)[0]:
                continue
        if (
            type_.name.endswith(ACTION_GOAL_SUFFIX) or
            type_.name.endswith(ACTION_RESULT_SUFFIX) or
            type_.name.endswith(ACTION_FEEDBACK_SUFFIX)
        ):
            typename = type_.name.rsplit('_', 1)[0]
        else:
            typename = type_.name
        member_names = includes.setdefault(
            '/'.join(type_.namespaces + ['detail',
                convert_camel_case_to_lower_case_underscore(typename)]) +
            '__experimental_struct.hpp', [])
        member_names.append(member.name)
}@
@#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
@
@[if includes]@
// Include directives for member types
@[    for header_file, member_names in includes.items()]@
@[        for member_name in member_names]@
// Member '@(member_name)'
@[        end for]@
@[        if header_file in include_directives]@
// already included above
// @
@[        else]@
@{include_directives.add(header_file)}@
@[        end if]@
#include "@(header_file)"
@[    end for]@
@[end if]@
@
@[for ns in message.structure.namespaced_type.namespaces]@
namespace @(ns)
{

@[end for]@
namespace experimental
{

// experimental message struct
struct @(message.structure.namespaced_type.name)
{
  using Type = @(message.structure.namespaced_type.name);

@{
pmr_init_list = []
for member in message.structure.members:
    if experimental_member_needs_pmr(member.type):
        pmr_init_list.append(experimental_pmr_init_expr(member.name, member.type))
}@
  @(message.structure.namespaced_type.name)() = default;

  explicit @(message.structure.namespaced_type.name)(
    std::pmr::memory_resource * mem_res)
@[if pmr_init_list]@
  : @(',\n    '.join(pmr_init_list))
@[end if]@
  {
@[if not pmr_init_list]@
    (void)mem_res;
@[end if]@
  }

  // field types and members
@[for member in message.structure.members]@
  using _@(member.name)_type =
    @(msg_type_to_experimental_cpp(member.type));
  _@(member.name)_type @(member.name);
@[end for]@

@[if len(message.structure.members) != 1 or message.structure.members[0].name != EMPTY_STRUCTURE_REQUIRED_MEMBER_NAME]@
  // setters for named parameter idiom
@[    for member in message.structure.members]@
  Type & set__@(member.name)(
    const @(msg_type_to_experimental_cpp(member.type)) & _arg)
  {
    this->@(member.name) = _arg;
    return *this;
  }
@[    end for]@
@[end if]@

  // constant declarations
@[for constant in message.constants]@
@[  if constant.name in msvc_common_macros]@
  // guard against '@(constant.name)' being predefined by MSVC by temporarily undefining it
#if defined(_WIN32)
#  if defined(@(constant.name))
#    pragma push_macro("@(constant.name)")
#    undef @(constant.name)
#  endif
#endif
@[  end if]@
@[  if isinstance(constant.type, AbstractString)]@
  inline static const std::string @(constant.name) = "@(escape_string(constant.value))";
@[  elif isinstance(constant.type, AbstractWString)]@
  inline static const std::u16string @(constant.name) = u"@(escape_wstring(constant.value))";
@[  else]@
  static constexpr @(BASIC_TYPE_TO_EXPERIMENTAL_CPP[constant.type.typename]) @(constant.name) =
@[    if constant.type.typename in (*INTEGER_TYPES, *CHARACTER_TYPES, BOOLEAN_TYPE, OCTET_TYPE)]@
    @(int(constant.value))@
@[      if constant.type.typename in UNSIGNED_INTEGER_TYPES]@u@
@[      end if]@
@[    elif constant.type.typename == 'float']@
    @(constant.value)f@
@[    else]@
    @(constant.value)@
@[    end if];
@[  end if]@
@[  if constant.name in msvc_common_macros]@
#if defined(_WIN32)
#  pragma warning(suppress : 4602)
#  pragma pop_macro("@(constant.name)")
#endif
@[  end if]@
@[end for]@

  // pointer types
  using RawPtr = @(message.structure.namespaced_type.name) *;
  using ConstRawPtr = const @(message.structure.namespaced_type.name) *;
  using SharedPtr = std::shared_ptr<@(message.structure.namespaced_type.name)>;
  using ConstSharedPtr =
    std::shared_ptr<const @(message.structure.namespaced_type.name)>;
  using UniquePtr = std::unique_ptr<@(message.structure.namespaced_type.name)>;
  using ConstUniquePtr =
    std::unique_ptr<const @(message.structure.namespaced_type.name)>;
  using WeakPtr = std::weak_ptr<@(message.structure.namespaced_type.name)>;
  using ConstWeakPtr =
    std::weak_ptr<const @(message.structure.namespaced_type.name)>;

  // comparison operators
  bool operator==(const @(message.structure.namespaced_type.name) & other) const
  {
@[if not message.structure.members]@
    (void)other;
@[end if]@
@[for member in message.structure.members]@
    if (this->@(member.name) != other.@(member.name)) {
      return false;
    }
@[end for]@
    return true;
  }
  bool operator!=(const @(message.structure.namespaced_type.name) & other) const
  {
    return !this->operator==(other);
  }
};  // struct @(message.structure.namespaced_type.name)

}  // namespace experimental
@[for ns in reversed(message.structure.namespaced_type.namespaces)]@

}  // namespace @(ns)
@[end for]@
