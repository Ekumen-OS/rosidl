@# Included from rosidl_typesupport_introspection_cpp/resource/idl__rosidl_typesupport_introspection_cpp.hpp.em
@{
# Get optional force_experimental flag (set by experimental wrapper templates).
try:
    force_experimental
except NameError:
    force_experimental = False

TEMPLATE(
    'msg__rosidl_typesupport_introspection_cpp.hpp.em',
    package_name=package_name, interface_path=interface_path, message=service.request_message,
    include_directives=include_directives, force_experimental=force_experimental)
}@

@{
TEMPLATE(
    'msg__rosidl_typesupport_introspection_cpp.hpp.em',
    package_name=package_name, interface_path=interface_path, message=service.response_message,
    include_directives=include_directives, force_experimental=force_experimental)
}@

@{
TEMPLATE(
    'msg__rosidl_typesupport_introspection_cpp.hpp.em',
    package_name=package_name, interface_path=interface_path, message=service.event_message,
    include_directives=include_directives, force_experimental=force_experimental)
}@

@{
header_files = [
    'rosidl_runtime_c/service_type_support_struct.h',
    'rosidl_typesupport_interface/macros.h',
    'rosidl_typesupport_introspection_cpp/visibility_control.h',
]

# For C symbol names, use a single token (e.g., 'srv_experimental') to
# avoid breaking the 4-argument ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME macro.
interface_parent_parts = list(interface_path.parents[0].parts)
if force_experimental:
    interface_parent_parts = [interface_parent_parts[0] + '_experimental']
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

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_service_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__SERVICE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, @(', '.join([package_name] + interface_parent_parts)), @(service.namespaced_type.name))();

#ifdef __cplusplus
}
#endif
