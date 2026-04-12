@# Included from rosidl_generator_c/resource/idl__experimental_struct.h.em
@{
from rosidl_parser.definition import AbstractNestedType
from rosidl_parser.definition import AbstractSequence
from rosidl_parser.definition import AbstractString
from rosidl_parser.definition import AbstractWString
from rosidl_parser.definition import Array
from rosidl_parser.definition import BasicType
from rosidl_parser.definition import BoundedSequence
from rosidl_parser.definition import NamespacedType
from rosidl_parser.definition import SERVICE_REQUEST_MESSAGE_SUFFIX
from rosidl_parser.definition import SERVICE_RESPONSE_MESSAGE_SUFFIX
from rosidl_parser.definition import UnboundedSequence
from rosidl_generator_c.experimental import BASIC_IDL_TYPES_TO_EXPERIMENTAL_C
from rosidl_generator_c.experimental import experimental_constraint_field
from rosidl_generator_c.experimental import experimental_field_declare_macro
from rosidl_generator_c.experimental import experimental_field_in_struct
from rosidl_generator_c.experimental import experimental_field_typename
from rosidl_generator_c.experimental import experimental_sequence_constraint_struct
from rosidl_generator_c.experimental import experimental_storage_field_declaration
from rosidl_generator_c.experimental import idl_structure_type_to_experimental_c_include_prefix
from rosidl_generator_c.experimental import idl_structure_type_to_experimental_c_typename
from rosidl_pycommon import convert_camel_case_to_lower_case_underscore

message_typename = idl_structure_type_to_experimental_c_typename(
    message.structure.namespaced_type)
}@
@
@#<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
@# Collect include directives for NamespacedType member types
@{
from collections import OrderedDict
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
        include_prefix = idl_structure_type_to_experimental_c_include_prefix(type_, 'detail')
        member_names = includes.setdefault(include_prefix + '__struct.h', [])
        member_names.append(member.name)
}@
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
@#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

@#<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
@# Per-field type declarations (one macro call per member)
@{
for member in message.structure.members:
    macro_call = experimental_field_declare_macro(message_typename, member)
    if macro_call:
        print(macro_call)
        print('')
}@
@#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

@#<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
@# ExternalStorage struct
typedef struct @(message_typename)__ExternalStorage_s
{
  /// Optional contiguous block encompassing all members.
  rosidl_memory_region_t block;

  /// Per-member storage descriptors.
  struct
  {
@[for member in message.structure.members]@
    @(experimental_storage_field_declaration(member.name, member.type));
@[end for]@
  } members;
} @(message_typename)__ExternalStorage;
@#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

@#<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
@# Message struct
/// Experimental message struct defined in @(package_name)/@('/'.join(interface_path.parts)).
typedef struct @(message_typename)_s
{
@[for member in message.structure.members]@
  @(experimental_field_in_struct(message_typename, member));
@[end for]@
} @(message_typename);
@#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

@#<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
@# Constraint types
@# 1. Per-field SequenceConstraint typedef for each UnboundedSequence member.
@# 2. Constraints struct: one field per member with a variable dimension.
@# 3. Per-message SequenceConstraint: for use as sequence<MsgType> element.
@{
_constraint_fields = []
for _m in message.structure.members:
    _cf = experimental_constraint_field(message_typename, _m)
    if _cf is not None:
        _constraint_fields.append(_cf)
}@
@[for member in message.structure.members]@
@{_sc = experimental_sequence_constraint_struct(message_typename, member)}@
@[if _sc is not None]@
@{_sc_text, _sc_tn, _sc_elem_cmp = _sc}@
@(_sc_text)

static inline bool
@(_sc_tn)__are_equal(
  const @(_sc_tn) * lhs,
  const @(_sc_tn) * rhs)
{
@[if _sc_elem_cmp is not None]@
  return lhs->size == rhs->size &&
    @(_sc_elem_cmp);
@[else]@
  return lhs->size == rhs->size;
@[end if]@
}

@[end if]@
@[end for]@
typedef struct @(message_typename)__Constraints_s
{
@[if _constraint_fields]@
@[for _ctype, _cname in _constraint_fields]@
  @(_ctype) @(_cname);
@[end for]@
@[else]@
  char _placeholder;
@[end if]@
} @(message_typename)__Constraints;

static inline bool
@(message_typename)__Constraints__are_equal(
  const @(message_typename)__Constraints * lhs,
  const @(message_typename)__Constraints * rhs)
{
@[if _constraint_fields]@
  return
@[for _i, (_ctype, _cname) in enumerate(_constraint_fields)]@
@[if _i < len(_constraint_fields) - 1]@
    @(_ctype)__are_equal(&lhs->@(_cname), &rhs->@(_cname)) &&
@[else]@
    @(_ctype)__are_equal(&lhs->@(_cname), &rhs->@(_cname));
@[end if]@
@[end for]@
@[else]@
  (void)lhs;
  (void)rhs;
  return true;
@[end if]@
}

typedef struct @(message_typename)__SequenceConstraint_s
{
  size_t size;
  @(message_typename)__Constraints element;
} @(message_typename)__SequenceConstraint;

static inline bool
@(message_typename)__SequenceConstraint__are_equal(
  const @(message_typename)__SequenceConstraint * lhs,
  const @(message_typename)__SequenceConstraint * rhs)
{
  return lhs->size == rhs->size &&
    @(message_typename)__Constraints__are_equal(&lhs->element, &rhs->element);
}
@#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
