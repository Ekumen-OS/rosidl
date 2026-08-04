@# generated from rosidl_typesupport_introspection_cpp/resource/idl__experimental_type_support.cpp.em
@# with input from @(package_name):@(interface_path)
@# generated code does not contain a copyright notice
@{
include_directives = set()
}@
@#######################################################################
@# Handle message
@#######################################################################
@{
from rosidl_parser.definition import Message
}@
@[for message in content.get_elements_of_type(Message)]@

@{
TEMPLATE(
    'msg__type_support.cpp.em',
    package_name=package_name, interface_path=interface_path, message=message,
    include_directives=include_directives, force_experimental=True)
}@
@[end for]@
@
@#######################################################################
@# Handle service
@#######################################################################
@{
from rosidl_parser.definition import Service
}@
@[for service in content.get_elements_of_type(Service)]@

@{
TEMPLATE(
    'srv__type_support.cpp.em',
    package_name=package_name, interface_path=interface_path, service=service,
    include_directives=include_directives, force_experimental=True)
}@
@[end for]@
