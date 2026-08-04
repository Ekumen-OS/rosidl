# Copyright 2026 Ekumen Inc.
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

"""Helper functions for rosidl_typesupport_introspection_cpp templates."""

from rosidl_parser.definition import (
    AbstractNestedType,
    AbstractString,
    AbstractWString,
    Array,
    BasicType,
    NamespacedType,
)


def experimental_type_name(namespaced_type, experimental_context=False):
    """
    Get the C++ qualified name for a nested NamespacedType.

    If experimental_context is True, insert 'experimental' into the namespace
    (before the type name) so the type resolves to the experimental variant.
    """
    ns = list(namespaced_type.namespaced_name())
    if experimental_context and 'experimental' not in ns:
        ns.insert(-1, 'experimental')
    return '::'.join(ns)


def accessor_element_type(member_type, experimental_context=False):
    """
    Return the C++ element type used by the introspection accessor functions.

    For standard messages the element type mirrors the C++ standard-library
    containers (std::vector<T> / std::array<T, N>); for experimental messages
    it mirrors the rosidl_runtime_cpp experimental containers
    (Sequence<T> / Array<T, N>), so the casts in the generated accessors match
    the actual member types of the experimental message struct.
    """
    from rosidl_parser.definition import AbstractNestedType  # noqa: F401
    from rosidl_parser.definition import AbstractString  # noqa: F401
    from rosidl_parser.definition import AbstractWString  # noqa: F401
    from rosidl_parser.definition import BasicType  # noqa: F401
    from rosidl_parser.definition import NamespacedType  # noqa: F401
    vt = member_type.value_type if isinstance(member_type, AbstractNestedType) else member_type
    if isinstance(vt, BasicType):
        if experimental_context:
            from rosidl_generator_cpp.experimental import BASIC_TYPE_TO_EXPERIMENTAL_CPP
            return BASIC_TYPE_TO_EXPERIMENTAL_CPP[vt.typename]
        from rosidl_generator_cpp import MSG_TYPE_TO_CPP
        return MSG_TYPE_TO_CPP[vt.typename]
    if isinstance(vt, AbstractString):
        return 'rosidl_runtime_cpp::String' if experimental_context else 'std::string'
    if isinstance(vt, AbstractWString):
        return 'rosidl_runtime_cpp::WString' if experimental_context else 'std::u16string'
    if isinstance(vt, NamespacedType):
        return experimental_type_name(vt, experimental_context)
    assert False, vt


def accessor_container_type(member_type, experimental_context=False):
    """
    Return the C++ container type cast target for a nested member.

    Standard messages use std::vector<T> / std::array<T, N>; experimental
    messages use rosidl_runtime_cpp::Sequence<T> / rosidl_runtime_cpp::Array<T, N>,
    which provide the same size()/resize()/operator[]/data() API used by the
    generated introspection accessor functions.
    """
    from rosidl_parser.definition import Array  # noqa: F401
    elem = accessor_element_type(member_type, experimental_context)
    if isinstance(member_type, Array):
        if experimental_context:
            return 'rosidl_runtime_cpp::Array<{0}, {1}>'.format(elem, member_type.size)
        return 'std::array<{0}, {1}>'.format(elem, member_type.size)
    if experimental_context:
        return 'rosidl_runtime_cpp::Sequence<{0}>'.format(elem)
    return 'std::vector<{0}>'.format(elem)
