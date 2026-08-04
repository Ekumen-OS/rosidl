@# Included from rosidl_typesupport_introspection_cpp/resource/idl__rosidl_typesupport_introspection_cpp.hpp.em
@{
from rosidl_pycommon import convert_camel_case_to_lower_case_underscore

# Get optional force_experimental flag (set by experimental wrapper templates).
# When True, always generate experimental-message code paths regardless of namespace.
try:
    force_experimental
except NameError:
    force_experimental = False

# For C symbol names, use a single token (e.g., 'msg_experimental') to
# avoid breaking the 4-argument ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME macro.
interface_parent_parts = list(interface_path.parents[0].parts)
if force_experimental:
    interface_parent_parts = [interface_parent_parts[0] + '_experimental']

header_files = [
    'rosidl_runtime_c/message_type_support_struct.h',
    'rosidl_typesupport_interface/macros.h',
    'rosidl_typesupport_introspection_cpp/visibility_control.h',
]
}@
@[for header_file in header_files]@
@[    if header_file in include_directives]@
// already included above
// @
@[    else]@
@{include_directives.add(header_file)}@
@[    end if]@
#include "@(header_file)"
@[end for]@

#ifdef __cplusplus
extern "C"
{
#endif

// TODO(dirk-thomas) these visibility macros should be message package specific
ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, @(', '.join([package_name] + interface_parent_parts)), @(message.structure.namespaced_type.name))();

#ifdef __cplusplus
}
#endif
