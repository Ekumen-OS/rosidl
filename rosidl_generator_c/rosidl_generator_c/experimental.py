# Copyright 2026 Ekumen, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Experimental code generation helpers for rosidl_generator_c."""

from rosidl_parser.definition import AbstractSequence
from rosidl_parser.definition import AbstractString
from rosidl_parser.definition import AbstractWString
from rosidl_parser.definition import Array
from rosidl_parser.definition import BasicType
from rosidl_parser.definition import BoundedSequence
from rosidl_parser.definition import NamespacedType
from rosidl_parser.definition import UnboundedSequence
from rosidl_pycommon import convert_camel_case_to_lower_case_underscore

# Mapping from IDL basic type names to C types for experimental message members.
# 'char' maps to 'char' (matching rosidl_runtime_c__experimental__Char) and
# 'wchar' maps to 'char16_t' (matching rosidl_runtime_c__experimental__WChar),
# unlike the regular C generator ('signed char' / 'uint16_t').
BASIC_IDL_TYPES_TO_EXPERIMENTAL_C = {
    'float': 'float',
    'double': 'double',
    'long double': 'long double',
    'char': 'char',
    'wchar': 'char16_t',
    'boolean': 'bool',
    'octet': 'uint8_t',
    'uint8': 'uint8_t',
    'int8': 'int8_t',
    'uint16': 'uint16_t',
    'int16': 'int16_t',
    'uint32': 'uint32_t',
    'int32': 'int32_t',
    'uint64': 'uint64_t',
    'int64': 'int64_t',
}

# Mapping from IDL basic type names to pre-declared scalar type names in
# rosidl_runtime_c/experimental/scalar.h.  These types are always available, so
# scalar fields can use them directly without per-field redeclaration.
BASIC_IDL_TYPES_TO_EXPERIMENTAL_C_SCALAR = {
    'float': 'rosidl_runtime_c__experimental__Float',
    'double': 'rosidl_runtime_c__experimental__Double',
    'long double': 'rosidl_runtime_c__experimental__LongDouble',
    'char': 'rosidl_runtime_c__experimental__Char',
    'wchar': 'rosidl_runtime_c__experimental__WChar',
    'boolean': 'rosidl_runtime_c__experimental__Boolean',
    'octet': 'rosidl_runtime_c__experimental__UInt8',
    'uint8': 'rosidl_runtime_c__experimental__UInt8',
    'int8': 'rosidl_runtime_c__experimental__Int8',
    'uint16': 'rosidl_runtime_c__experimental__UInt16',
    'int16': 'rosidl_runtime_c__experimental__Int16',
    'uint32': 'rosidl_runtime_c__experimental__UInt32',
    'int32': 'rosidl_runtime_c__experimental__Int32',
    'uint64': 'rosidl_runtime_c__experimental__UInt64',
    'int64': 'rosidl_runtime_c__experimental__Int64',
}

# Mapping from IDL basic type names to pre-declared primitive sequence type names in
# rosidl_runtime_c/experimental/sequence.h.  These types are always available, so
# fields of type sequence<primitive> can use them directly without per-field redeclaration.
BASIC_IDL_TYPES_TO_EXPERIMENTAL_C_SEQUENCE = {
    'float': 'rosidl_runtime_c__experimental__FloatSequence',
    'double': 'rosidl_runtime_c__experimental__DoubleSequence',
    'long double': 'rosidl_runtime_c__experimental__LongDoubleSequence',
    'char': 'rosidl_runtime_c__experimental__CharSequence',
    'wchar': 'rosidl_runtime_c__experimental__WCharSequence',
    'boolean': 'rosidl_runtime_c__experimental__BooleanSequence',
    'octet': 'rosidl_runtime_c__experimental__UInt8Sequence',
    'uint8': 'rosidl_runtime_c__experimental__UInt8Sequence',
    'int8': 'rosidl_runtime_c__experimental__Int8Sequence',
    'uint16': 'rosidl_runtime_c__experimental__UInt16Sequence',
    'int16': 'rosidl_runtime_c__experimental__Int16Sequence',
    'uint32': 'rosidl_runtime_c__experimental__UInt32Sequence',
    'int32': 'rosidl_runtime_c__experimental__Int32Sequence',
    'uint64': 'rosidl_runtime_c__experimental__UInt64Sequence',
    'int64': 'rosidl_runtime_c__experimental__Int64Sequence',
}


def idl_structure_type_to_experimental_c_typename(namespaced_type):
    """Return the C experimental typename for a namespaced IDL type.

    Inserts 'experimental' between the namespaces and the type name.
    Example: std_msgs::msg::String  ->  std_msgs__msg__experimental__String
    """
    parts = list(namespaced_type.namespaces) + ['experimental', namespaced_type.name]
    return '__'.join(parts)


def idl_structure_type_to_experimental_c_include_prefix(namespaced_type, subdirectory=None):
    """Return the include path prefix for an experimental C message type.

    Example: std_msgs::msg::String, 'detail'  ->  std_msgs/msg/experimental/detail/string
    """
    parts = [
        convert_camel_case_to_lower_case_underscore(x)
        for x in namespaced_type.namespaced_name()
    ]
    # Insert 'experimental' before the last component (type name)
    parts[-1:-1] = ['experimental']
    if subdirectory is not None:
        parts[-1:-1] = [subdirectory]
    include_prefix = '/'.join(parts)
    # Strip service / action suffixes (same as the regular C generator)
    for suffix in ('__request', '__response', '__goal', '__result',
                   '__feedback', '__send_goal', '__get_result'):
        if include_prefix.endswith(suffix):
            include_prefix = include_prefix[:-len(suffix)]
            break
    return include_prefix


def experimental_field_typename(message_experimental_typename, member_name):
    """Return the per-field typedef name for a message member.

    Example: pkg__msg__experimental__Msg, foo  ->  pkg__msg__experimental__Msg__foo
    """
    return '{}__{}'.format(message_experimental_typename, member_name)


def experimental_element_c_type(message_experimental_typename, member_name, vt):
    """Return the C type name for elements of a complex-element array or sequence.

    For bounded strings the element type gets its own per-field typedef (so that
    the bound is encoded in the type name).  For unbounded strings the pre-declared
    runtime type is used.  For NamespacedType the experimental typename is returned.
    """
    field_tn = experimental_field_typename(message_experimental_typename, member_name)
    if isinstance(vt, AbstractString):
        if vt.has_maximum_size():
            return '{}_elem'.format(field_tn)
        return 'rosidl_runtime_c__experimental__String'
    if isinstance(vt, AbstractWString):
        if vt.has_maximum_size():
            return '{}_elem'.format(field_tn)
        return 'rosidl_runtime_c__experimental__WString'
    if isinstance(vt, NamespacedType):
        return idl_structure_type_to_experimental_c_typename(vt)
    assert False, 'Unexpected element type: ' + str(vt)


def experimental_field_declare_macro(message_typename, member):
    """Return the single DECLARE macro call for this member's per-field type.

    Returns None for NamespacedType members (the sub-message type is used directly,
    no per-field typedef is needed).

    Rules
    -----
    BasicType          -> SCALAR_DECLARE(field_tn, c_type)
    String             -> BASIC_STRING_DECLARE(field_tn, char)
    String<N>          -> BASIC_BOUNDED_STRING_DECLARE(field_tn, char, N)
    WString            -> BASIC_STRING_DECLARE(field_tn, char16_t)
    WString<N>         -> BASIC_BOUNDED_STRING_DECLARE(field_tn, char16_t, N)
    NamespacedType     -> None  (sub-message experimental type used directly)
    Array<Basic,N>     -> PRIMITIVE_ARRAY_DECLARE(field_tn, c_type, N)
    Array<String,N>    -> BASIC_STRING_DECLARE(elem_tn, char)
                         ARRAY_DECLARE(field_tn, elem_tn, N)
    Array<String<B>,N> -> BASIC_BOUNDED_STRING_DECLARE(elem_tn, char, B)
                         ARRAY_DECLARE(field_tn, elem_tn, N)
    Array<Sub,N>       -> ARRAY_DECLARE(field_tn, sub_tn, N)
    Sequence<Basic>    -> None  (uses pre-declared rosidl_runtime_c__experimental__<Type>Sequence)
    BSeq<Basic,B>      -> PRIMITIVE_BOUNDED_SEQUENCE_DECLARE(field_tn, c_type, B)
    Sequence<String>   -> SEQUENCE_DECLARE(field_tn, rosidl_runtime_c__experimental__String)
    Sequence<String<B>>-> BASIC_BOUNDED_STRING_DECLARE(elem_tn, char, B)
                         SEQUENCE_DECLARE(field_tn, elem_tn)
    BSeq<String>       -> BOUNDED_SEQUENCE_DECLARE(field_tn, rosidl_runtime_c__experimental__String, B)
    BSeq<String<B2>>   -> BASIC_BOUNDED_STRING_DECLARE(elem_tn, char, B2)
                         BOUNDED_SEQUENCE_DECLARE(field_tn, elem_tn, B)
    Sequence<Sub>      -> SEQUENCE_DECLARE(field_tn, sub_tn)
    BSeq<Sub,B>        -> BOUNDED_SEQUENCE_DECLARE(field_tn, sub_tn, B)
    """
    type_ = member.type
    field_tn = experimental_field_typename(message_typename, member.name)

    if isinstance(type_, BasicType):
        return None  # use primitive C type directly

    if isinstance(type_, AbstractString):
        if type_.has_maximum_size():
            return (
                'ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING_DECLARE'
                '({}, char, {}U)'.format(field_tn, type_.maximum_size))
        return None  # use pre-declared rosidl_runtime_c__experimental__String

    if isinstance(type_, AbstractWString):
        if type_.has_maximum_size():
            return (
                'ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING_DECLARE'
                '({}, char16_t, {}U)'.format(field_tn, type_.maximum_size))
        return None  # use pre-declared rosidl_runtime_c__experimental__WString

    if isinstance(type_, NamespacedType):
        return None  # sub-message type used directly in the struct

    if isinstance(type_, Array):
        vt = type_.value_type
        if isinstance(vt, BasicType):
            c_type = BASIC_IDL_TYPES_TO_EXPERIMENTAL_C[vt.typename]
            return 'ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_ARRAY_DECLARE({}, {}, {}U)'.format(
                field_tn, c_type, type_.size)
        # Complex element array: optional elem typedef, then ARRAY_DECLARE
        lines = []
        if isinstance(vt, AbstractString):
            if vt.has_maximum_size():
                elem_tn = '{}_elem'.format(field_tn)
                lines.append(
                    'ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING_DECLARE'
                    '({}, char, {}U)'.format(elem_tn, vt.maximum_size))
            else:
                elem_tn = 'rosidl_runtime_c__experimental__String'
        elif isinstance(vt, AbstractWString):
            if vt.has_maximum_size():
                elem_tn = '{}_elem'.format(field_tn)
                lines.append(
                    'ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING_DECLARE'
                    '({}, char16_t, {}U)'.format(elem_tn, vt.maximum_size))
            else:
                elem_tn = 'rosidl_runtime_c__experimental__WString'
        else:
            # NamespacedType
            elem_tn = idl_structure_type_to_experimental_c_typename(vt)
        lines.append(
            'ROSIDL_RUNTIME_C__EXPERIMENTAL__ARRAY_DECLARE({}, {}, {}U)'.format(
                field_tn, elem_tn, type_.size))
        return '\n'.join(lines)

    if isinstance(type_, BoundedSequence):
        vt = type_.value_type
        if isinstance(vt, BasicType):
            c_type = BASIC_IDL_TYPES_TO_EXPERIMENTAL_C[vt.typename]
            return (
                'ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_BOUNDED_SEQUENCE_DECLARE'
                '({}, {}, {}U)'.format(field_tn, c_type, type_.maximum_size))
        elem_c = experimental_element_c_type(message_typename, member.name, vt)
        lines = []
        if isinstance(vt, AbstractString) and vt.has_maximum_size():
            lines.append(
                'ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING_DECLARE'
                '({}, char, {}U)'.format(elem_c, vt.maximum_size))
        elif isinstance(vt, AbstractWString) and vt.has_maximum_size():
            lines.append(
                'ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING_DECLARE'
                '({}, char16_t, {}U)'.format(elem_c, vt.maximum_size))
        lines.append(
            'ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE_DECLARE'
            '({}, {}, {}U)'.format(field_tn, elem_c, type_.maximum_size))
        return '\n'.join(lines)

    if isinstance(type_, UnboundedSequence):
        vt = type_.value_type
        if isinstance(vt, BasicType):
            return None  # use pre-declared rosidl_runtime_c__experimental__<Type>Sequence
        elem_c = experimental_element_c_type(message_typename, member.name, vt)
        lines = []
        if isinstance(vt, AbstractString) and vt.has_maximum_size():
            lines.append(
                'ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING_DECLARE'
                '({}, char, {}U)'.format(elem_c, vt.maximum_size))
        elif isinstance(vt, AbstractWString) and vt.has_maximum_size():
            lines.append(
                'ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING_DECLARE'
                '({}, char16_t, {}U)'.format(elem_c, vt.maximum_size))
        lines.append(
            'ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DECLARE'
            '({}, {})'.format(field_tn, elem_c))
        return '\n'.join(lines)

    assert False, 'Unhandled member type: ' + str(type_)


def experimental_field_define_macro(message_typename, member):
    """Return the single DEFINE macro call for this member's per-field type.

    Returns None for member types that are either static-inline (PRIMITIVE_ARRAY_DECLARE,
    ARRAY_DECLARE) or defined externally (NamespacedType sub-messages).

    The pattern mirrors experimental_field_declare_macro.
    """
    type_ = member.type
    field_tn = experimental_field_typename(message_typename, member.name)

    if isinstance(type_, BasicType):
        return None

    if isinstance(type_, AbstractString):
        if not type_.has_maximum_size():
            return None
        return (
            'ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING_DEFINE'
            '({}, char, {}U)'.format(field_tn, type_.maximum_size))

    if isinstance(type_, AbstractWString):
        if not type_.has_maximum_size():
            return None
        return (
            'ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING_DEFINE'
            '({}, char16_t, {}U)'.format(field_tn, type_.maximum_size))

    if isinstance(type_, NamespacedType):
        return None  # sub-message functions defined in sub-message code

    if isinstance(type_, Array):
        vt = type_.value_type
        if isinstance(vt, BasicType):
            return None  # PRIMITIVE_ARRAY_DECLARE emits static-inline definitions
        # Complex-element array: define the optional elem string typedef, then the
        # ARRAY is also static-inline (no separate DEFINE needed).
        elem_tn = '{}_elem'.format(field_tn)
        if isinstance(vt, (AbstractString, AbstractWString)):
            if not vt.has_maximum_size():
                return None  # use pre-declared type
            char_type = 'char' if isinstance(vt, AbstractString) else 'char16_t'
            return (
                'ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING_DEFINE'
                '({}, {}, {}U)'.format(elem_tn, char_type, vt.maximum_size))
        # NamespacedType elements: defined in sub-message code
        return None

    if isinstance(type_, BoundedSequence):
        vt = type_.value_type
        if isinstance(vt, BasicType):
            c_type = BASIC_IDL_TYPES_TO_EXPERIMENTAL_C[vt.typename]
            return (
                'ROSIDL_RUNTIME_C__EXPERIMENTAL__PRIMITIVE_BOUNDED_SEQUENCE_DEFINE'
                '({}, {}, {}U)'.format(field_tn, c_type, type_.maximum_size))
        elem_c = experimental_element_c_type(message_typename, member.name, vt)
        lines = []
        if isinstance(vt, AbstractString) and vt.has_maximum_size():
            lines.append(
                'ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING_DEFINE'
                '({}, char, {}U)'.format(elem_c, vt.maximum_size))
        elif isinstance(vt, AbstractWString) and vt.has_maximum_size():
            lines.append(
                'ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING_DEFINE'
                '({}, char16_t, {}U)'.format(elem_c, vt.maximum_size))
        lines.append(
            'ROSIDL_RUNTIME_C__EXPERIMENTAL__BOUNDED_SEQUENCE_DEFINE'
            '({}, {}, {}U)'.format(field_tn, elem_c, type_.maximum_size))
        return '\n'.join(lines)

    if isinstance(type_, UnboundedSequence):
        vt = type_.value_type
        if isinstance(vt, BasicType):
            return None  # pre-declared type, defined in rosidl_runtime_c
        elem_c = experimental_element_c_type(message_typename, member.name, vt)
        lines = []
        if isinstance(vt, AbstractString) and vt.has_maximum_size():
            lines.append(
                'ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING_DEFINE'
                '({}, char, {}U)'.format(elem_c, vt.maximum_size))
        elif isinstance(vt, AbstractWString) and vt.has_maximum_size():
            lines.append(
                'ROSIDL_RUNTIME_C__EXPERIMENTAL__BASIC_BOUNDED_STRING_DEFINE'
                '({}, char16_t, {}U)'.format(elem_c, vt.maximum_size))
        lines.append(
            'ROSIDL_RUNTIME_C__EXPERIMENTAL__SEQUENCE_DEFINE'
            '({}, {})'.format(field_tn, elem_c))
        return '\n'.join(lines)

    assert False, 'Unhandled member type: ' + str(type_)


def experimental_field_in_struct(message_experimental_typename, member):
    """Return the declaration string for a field inside the experimental message struct body."""
    type_ = member.type
    if isinstance(type_, BasicType):
        scalar_typename = BASIC_IDL_TYPES_TO_EXPERIMENTAL_C_SCALAR[type_.typename]
        return '{} {}'.format(scalar_typename, member.name)
    if isinstance(type_, NamespacedType):
        sub_typename = idl_structure_type_to_experimental_c_typename(type_)
        return '{} {}'.format(sub_typename, member.name)
    if isinstance(type_, AbstractString) and not type_.has_maximum_size():
        return 'rosidl_runtime_c__experimental__String {}'.format(member.name)
    if isinstance(type_, AbstractWString) and not type_.has_maximum_size():
        return 'rosidl_runtime_c__experimental__WString {}'.format(member.name)
    if isinstance(type_, UnboundedSequence) and isinstance(type_.value_type, BasicType):
        seq_typename = BASIC_IDL_TYPES_TO_EXPERIMENTAL_C_SEQUENCE[type_.value_type.typename]
        return '{} {}'.format(seq_typename, member.name)
    field_typename = experimental_field_typename(message_experimental_typename, member.name)
    return '{} {}'.format(field_typename, member.name)


def experimental_field_needs_allocator(member_type):
    """Return True if this member type's __init requires a rcutils_allocator_t * argument."""
    if isinstance(member_type, BasicType):
        return False  # Scalar: inline local storage, no allocator
    if isinstance(member_type, Array):
        return not isinstance(member_type.value_type, BasicType)
    return True  # strings, sequences, sub-messages all need allocators


def experimental_constraint_field(msg_tn, member):
    """Return (constraint_type_name, field_name) for the Constraints struct, or None.

    Only members with at least one variable dimension contribute a field:
      - Unbounded string/wstring                  → StringConstraint
      - NamespacedType (sub-message)              → {sub_tn}__Constraints
      - Array of unbounded string/wstring         → StringConstraint (shared cap)
      - Array of NamespacedType                   → {sub_tn}__Constraints
      - UnboundedSequence<any>                    → {field_tn}__SequenceConstraint
      - BasicType, bounded string, BoundedSequence → None (fully fixed)
    """
    type_ = member.type
    name = member.name

    if isinstance(type_, BasicType):
        return None

    if isinstance(type_, (AbstractString, AbstractWString)):
        if type_.has_maximum_size():
            return None
        return ('rosidl_runtime_c__experimental__StringConstraint', name)

    if isinstance(type_, NamespacedType):
        sub_tn = idl_structure_type_to_experimental_c_typename(type_)
        return ('{}__Constraints'.format(sub_tn), name)

    if isinstance(type_, Array):
        vt = type_.value_type
        if isinstance(vt, BasicType):
            return None
        if isinstance(vt, (AbstractString, AbstractWString)):
            if vt.has_maximum_size():
                return None
            return ('rosidl_runtime_c__experimental__StringConstraint', name)
        if isinstance(vt, NamespacedType):
            sub_tn = idl_structure_type_to_experimental_c_typename(vt)
            return ('{}__Constraints'.format(sub_tn), name)
        return None

    if isinstance(type_, BoundedSequence):
        return None  # fully bounded — no runtime constraint

    if isinstance(type_, UnboundedSequence):
        field_tn = experimental_field_typename(msg_tn, name)
        return ('{}__SequenceConstraint'.format(field_tn), name)

    return None


def experimental_sequence_constraint_struct(msg_tn, member):
    """Return (text, type_name, element_cmp_expr) for the per-field
    SequenceConstraint typedef of an UnboundedSequence member, or None.

    element_cmp_expr is the boolean C expression for comparing the element
    field, or None when the SequenceConstraint has no element field.
    """
    type_ = member.type
    if not isinstance(type_, UnboundedSequence):
        return None

    field_tn = experimental_field_typename(msg_tn, member.name)
    struct_tn = '{}__SequenceConstraint'.format(field_tn)
    vt = type_.value_type

    element_type = None
    if isinstance(vt, AbstractString) and not vt.has_maximum_size():
        element_type = 'rosidl_runtime_c__experimental__StringConstraint'
    elif isinstance(vt, AbstractWString) and not vt.has_maximum_size():
        element_type = 'rosidl_runtime_c__experimental__StringConstraint'
    elif isinstance(vt, NamespacedType):
        sub_tn = idl_structure_type_to_experimental_c_typename(vt)
        element_type = '{}__Constraints'.format(sub_tn)
    # BasicType or bounded string/wstring: no element constraint

    lines = ['typedef struct {', '  size_t size;']
    if element_type is not None:
        lines.append('  {} element;'.format(element_type))
    lines.append('}} {};'.format(struct_tn))
    text = '\n'.join(lines)

    element_cmp = None
    if element_type is not None:
        element_cmp = (
            '{}__are_equal(&lhs->element, &rhs->element)'.format(element_type))

    return (text, struct_tn, element_cmp)


def experimental_storage_field_declaration(member_name, member_type):
    """Return the C declaration for a member's field inside ExternalStorage.

    Uses type-erased rosidl_memory_t / rosidl_memory_region_t structs throughout;
    no generated-type names appear here.
    For fixed-size arrays of non-primitive types a C-style array suffix is used.
    For sequences of non-primitive types, a struct with both the sequence region
    and element storage pool array is used, matching the C++ implementation.
    """
    type_ = member_type
    if isinstance(type_, BasicType):
        return 'rosidl_memory_t {}'.format(member_name)
    if isinstance(type_, (AbstractString, AbstractWString)):
        return 'rosidl_memory_region_t {}'.format(member_name)
    if isinstance(type_, NamespacedType):
        sub_tn = idl_structure_type_to_experimental_c_typename(type_)
        return '{}__ExternalStorage {}'.format(sub_tn, member_name)
    if isinstance(type_, Array):
        vt = type_.value_type
        if isinstance(vt, BasicType):
            return 'rosidl_memory_region_t {}'.format(member_name)
        if isinstance(vt, (AbstractString, AbstractWString)):
            return 'rosidl_memory_region_t {}[{}U]'.format(member_name, type_.size)
        if isinstance(vt, NamespacedType):
            sub_tn = idl_structure_type_to_experimental_c_typename(vt)
            return '{}__ExternalStorage {}[{}U]'.format(sub_tn, member_name, type_.size)
    if isinstance(type_, AbstractSequence):
        vt = type_.value_type
        if isinstance(vt, BasicType):
            # Primitive sequence: just the backing buffer region
            return 'rosidl_memory_region_t {}'.format(member_name)
        # Complex element sequence: need both sequence region and element storage pool
        lines = []
        lines.append('struct {')
        lines.append('  rosidl_memory_region_t region;')
        if isinstance(vt, (AbstractString, AbstractWString)):
            lines.append('  rosidl_memory_region_t * element_storage_pool;')
        elif isinstance(vt, NamespacedType):
            sub_tn = idl_structure_type_to_experimental_c_typename(vt)
            lines.append('  {}__ExternalStorage * element_storage_pool;'.format(sub_tn))
        lines.append('  size_t element_storage_pool_size;')
        lines.append('}} {}'.format(member_name))
        return '\n    '.join(lines)
    assert False, 'Unhandled type: ' + str(type_)
