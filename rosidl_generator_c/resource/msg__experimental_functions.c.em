@# Included from rosidl_generator_c/resource/idl__experimental_functions.c.em
@{
from ast import literal_eval
from rosidl_parser.definition import AbstractNestedType
from rosidl_parser.definition import AbstractSequence
from rosidl_parser.definition import AbstractString
from rosidl_parser.definition import AbstractWString
from rosidl_parser.definition import Array
from rosidl_parser.definition import BasicType
from rosidl_parser.definition import BoundedSequence
from rosidl_parser.definition import NamespacedType
from rosidl_generator_c import interface_path_to_string
from rosidl_generator_c import value_to_c
from rosidl_generator_c.experimental import BASIC_IDL_TYPES_TO_EXPERIMENTAL_C
from rosidl_generator_c.experimental import experimental_element_c_type
from rosidl_generator_c.experimental import experimental_field_define_macro
from rosidl_generator_c.experimental import experimental_field_needs_allocator
from rosidl_generator_c.experimental import experimental_field_typename
from rosidl_generator_c.experimental import idl_structure_type_to_experimental_c_include_prefix
from rosidl_generator_c.experimental import idl_structure_type_to_experimental_c_typename

message_typename = idl_structure_type_to_experimental_c_typename(
    message.structure.namespaced_type)
}@
@
@#<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
@# Include directives for NamespacedType member functions
@{
from collections import OrderedDict
includes = OrderedDict()
for member in message.structure.members:
    type_ = member.type
    if isinstance(type_, AbstractNestedType):
        type_ = type_.value_type
    if isinstance(type_, NamespacedType):
        include_prefix = idl_structure_type_to_experimental_c_include_prefix(type_, 'detail')
        member_names = includes.setdefault(include_prefix + '__functions.h', [])
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
@# DEFINE macros for all per-field types (one macro call per member)
@{
for member in message.structure.members:
    macro_call = experimental_field_define_macro(message_typename, member)
    if macro_call:
        print(macro_call)
        print('')
}@
@#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

@#<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
@# __init / __init_with_allocator
bool
@(message_typename)__init_with_allocator(
  @(message_typename) * msg,
  const rcutils_allocator_t * allocator)
{
  if (!msg) {
    return false;
  }
@{
lines = []
for member in message.structure.members:
    field_tn = experimental_field_typename(message_typename, member.name)
    type_ = member.type

    lines.append('// ' + member.name)

    if isinstance(type_, BasicType):
        lines.append('if (!{}__init(&msg->{})) {{'.format(field_tn, member.name))
        lines.append('  {}__fini(msg);'.format(message_typename))
        lines.append('  return false;')
        lines.append('}')
        if member.has_annotation('default'):
            lines.append('msg->{}.value->data = {};'.format(
                member.name,
                value_to_c(type_, member.get_annotation_value('default')['value'])))

    elif isinstance(type_, (AbstractString, AbstractWString)):
        lines.append('if (!{}__init_with_allocator(&msg->{}, allocator)) {{'.format(field_tn, member.name))
        lines.append('  {}__fini(msg);'.format(message_typename))
        lines.append('  return false;')
        lines.append('}')
        if member.has_annotation('default'):
            lines.append('{')
            lines.append('  bool success = {}__assign(&msg->{}, {});'.format(
                field_tn, member.name,
                value_to_c(type_, member.get_annotation_value('default')['value'])))
            lines.append('  if (!success) {')
            lines.append('    {}__fini(msg);'.format(message_typename))
            lines.append('    return false;')
            lines.append('  }')
            lines.append('}')

    elif isinstance(type_, NamespacedType):
        sub_tn = idl_structure_type_to_experimental_c_typename(type_)
        lines.append('if (!{}__init_with_allocator(&msg->{}, allocator)) {{'.format(sub_tn, member.name))
        lines.append('  {}__fini(msg);'.format(message_typename))
        lines.append('  return false;')
        lines.append('}')

    elif isinstance(type_, Array):
        vt = type_.value_type
        if isinstance(vt, BasicType):
            lines.append('if (!{}__init(&msg->{})) {{'.format(field_tn, member.name))
            lines.append('  {}__fini(msg);'.format(message_typename))
            lines.append('  return false;')
            lines.append('}')
            if member.has_annotation('default'):
                for i, dv in enumerate(literal_eval(
                        member.get_annotation_value('default')['value'])):
                    lines.append('msg->{}.value->data[{}] = {};'.format(
                        member.name, i, value_to_c(vt, dv)))
        else:
            lines.append('if (!{}__init_with_allocator(&msg->{}, allocator)) {{'.format(field_tn, member.name))
            lines.append('  {}__fini(msg);'.format(message_typename))
            lines.append('  return false;')
            lines.append('}')
            if member.has_annotation('default') and isinstance(vt, (AbstractString, AbstractWString)):
                elem_type = experimental_element_c_type(message_typename, member.name, vt)
                for i, dv in enumerate(literal_eval(
                        member.get_annotation_value('default')['value'])):
                    lines.append('{')
                    lines.append('  bool success = {}__assign(&msg->{}.data[{}], {});'.format(
                        elem_type, member.name, i, value_to_c(vt, dv)))
                    lines.append('  if (!success) {')
                    lines.append('    {}__fini(msg);'.format(message_typename))
                    lines.append('    return false;')
                    lines.append('  }')
                    lines.append('}')

    elif isinstance(type_, AbstractSequence):
        lines.append('if (!{}__init_with_allocator(&msg->{}, allocator)) {{'.format(field_tn, member.name))
        lines.append('  {}__fini(msg);'.format(message_typename))
        lines.append('  return false;')
        lines.append('}')
        vt = type_.value_type
        if member.has_annotation('default') and isinstance(vt, BasicType):
            default_vals = literal_eval(member.get_annotation_value('default')['value'])
            lines.append('{')
            lines.append('  bool success = {}__resize(&msg->{}, {}U);'.format(
                field_tn, member.name, len(default_vals)))
            lines.append('  if (!success) {')
            lines.append('    {}__fini(msg);'.format(message_typename))
            lines.append('    return false;')
            lines.append('  }')
            for i, dv in enumerate(default_vals):
                lines.append('  msg->{}.value[{}] = {};'.format(
                    member.name, i, value_to_c(vt, dv)))
            lines.append('}')

    lines.append('')

for line in lines:
    print('  ' + line)
}@
  return true;
}

bool
@(message_typename)__init(@(message_typename) * msg)
{
  return @(message_typename)__init_with_allocator(msg, NULL);
}
@#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

@#<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
@# __init_from_storage
bool
@(message_typename)__init_from_storage(
  @(message_typename) * msg,
  const @(message_typename)__ExternalStorage * storage)
{
  if (!msg || !storage) {
    return false;
  }
@{
lines = []
for member in message.structure.members:
    field_tn = experimental_field_typename(message_typename, member.name)
    type_ = member.type

    lines.append('// ' + member.name)

    if isinstance(type_, BasicType):
        lines.append('if (!{}__init_from_memory(&msg->{}, storage->members.{})) {{'.format(
            field_tn, member.name, member.name))
        lines.append('  {}__fini(msg);'.format(message_typename))
        lines.append('  return false;')
        lines.append('}')

    elif isinstance(type_, (AbstractString, AbstractWString)):
        lines.append('if (!{}__init_from_region(&msg->{}, storage->members.{})) {{'.format(
            field_tn, member.name, member.name))
        lines.append('  {}__fini(msg);'.format(message_typename))
        lines.append('  return false;')
        lines.append('}')

    elif isinstance(type_, NamespacedType):
        sub_tn = idl_structure_type_to_experimental_c_typename(type_)
        lines.append('if (!{}__init_from_storage(&msg->{}, &storage->members.{})) {{'.format(
            sub_tn, member.name, member.name))
        lines.append('  {}__fini(msg);'.format(message_typename))
        lines.append('  return false;')
        lines.append('}')

    elif isinstance(type_, Array):
        vt = type_.value_type
        if isinstance(vt, BasicType):
            lines.append('if (!{}__init_from_region(&msg->{}, storage->members.{})) {{'.format(
                field_tn, member.name, member.name))
            lines.append('  {}__fini(msg);'.format(message_typename))
            lines.append('  return false;')
            lines.append('}')
        else:
            # ARRAY: per-element external storage.
            # String/WString elements expose __init_from_region(elem, region).
            # NamespacedType elements expose __init_from_storage(elem, storage).
            lines.append('for (size_t i = 0U; i < {}U; ++i) {{'.format(type_.size))
            if isinstance(vt, NamespacedType):
                sub_tn = idl_structure_type_to_experimental_c_typename(vt)
                lines.append('  if (!{}__init_from_storage(&msg->{}.value->data[i],'.format(
                    sub_tn, member.name))
                lines.append('      &storage->members.{}[i])) {{'.format(member.name))
            else:
                elem_type = experimental_element_c_type(message_typename, member.name, vt)
                lines.append('  if (!{}__init_from_region(&msg->{}.value->data[i],'.format(
                    elem_type, member.name))
                lines.append('      storage->members.{}[i])) {{'.format(member.name))
            lines.append('    {}__fini(msg);'.format(message_typename))
            lines.append('    return false;')
            lines.append('  }')
            lines.append('}')

    elif isinstance(type_, AbstractSequence):
        vt = type_.value_type
        if isinstance(vt, BasicType):
            # Primitive sequence: just use the region
            lines.append('if (!{}__init_from_region(&msg->{}, storage->members.{})) {{'.format(
                field_tn, member.name, member.name))
            lines.append('  {}__fini(msg);'.format(message_typename))
            lines.append('  return false;')
            lines.append('}')
        else:
            # Complex element sequence: use region + element storage pool
            lines.append('if (!{}__init_region_storage(&msg->{},'.format(field_tn, member.name))
            lines.append('    storage->members.{}.region,'.format(member.name))
            lines.append('    storage->members.{}.element_storage_pool,'.format(member.name))
            lines.append('    storage->members.{}.element_storage_pool_size)) {{'.format(member.name))
            lines.append('  {}__fini(msg);'.format(message_typename))
            lines.append('  return false;')
            lines.append('}')

    lines.append('')

for line in lines:
    print('  ' + line)
}@
  return true;
}
@#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

@#<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
@# __fini
void
@(message_typename)__fini(@(message_typename) * msg)
{
  if (!msg) {
    return;
  }
@{
lines = []
for member in message.structure.members:
    field_tn = experimental_field_typename(message_typename, member.name)
    type_ = member.type

    lines.append('// ' + member.name)
    if isinstance(type_, BasicType):
        lines.append('{}__fini(&msg->{});'.format(field_tn, member.name))
    elif isinstance(type_, (AbstractString, AbstractWString)):
        lines.append('{}__fini(&msg->{});'.format(field_tn, member.name))
    elif isinstance(type_, NamespacedType):
        sub_tn = idl_structure_type_to_experimental_c_typename(type_)
        lines.append('{}__fini(&msg->{});'.format(sub_tn, member.name))
    elif isinstance(type_, Array):
        lines.append('{}__fini(&msg->{});'.format(field_tn, member.name))
    elif isinstance(type_, AbstractSequence):
        lines.append('{}__fini(&msg->{});'.format(field_tn, member.name))
    lines.append('')

for line in lines:
    print('  ' + line)
}@
}
@#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

@#<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
@# __create / __destroy
@(message_typename) *
@(message_typename)__create()
{
  rcutils_allocator_t alloc = rcutils_get_default_allocator();
  @(message_typename) * msg =
    (@(message_typename) *)alloc.zero_allocate(1U, sizeof(@(message_typename)), alloc.state);
  if (!msg) {
    return NULL;
  }
  if (!@(message_typename)__init_with_allocator(msg, &alloc)) {
    alloc.deallocate(msg, alloc.state);
    return NULL;
  }
  return msg;
}

void
@(message_typename)__destroy(@(message_typename) * msg)
{
  if (msg) {
    @(message_typename)__fini(msg);
  }
  rcutils_allocator_t alloc = rcutils_get_default_allocator();
  alloc.deallocate(msg, alloc.state);
}
@#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

@#<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
@# __are_equal
bool
@(message_typename)__are_equal(
  const @(message_typename) * lhs,
  const @(message_typename) * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
@[for member in message.structure.members]@
@{
field_tn = experimental_field_typename(message_typename, member.name)
type_ = member.type
}@
  // @(member.name)
@[  if isinstance(type_, BasicType)]@
  if (lhs->@(member.name).value->data != rhs->@(member.name).value->data) {
    return false;
  }
@[  elif isinstance(type_, (AbstractString, AbstractWString, AbstractSequence))]@
  if (!@(field_tn)__are_equal(&lhs->@(member.name), &rhs->@(member.name))) {
    return false;
  }
@[  elif isinstance(type_, NamespacedType)]@
@{sub_tn = idl_structure_type_to_experimental_c_typename(type_)}@
  if (!@(sub_tn)__are_equal(&lhs->@(member.name), &rhs->@(member.name))) {
    return false;
  }
@[  else]@
  if (!@(field_tn)__are_equal(&lhs->@(member.name), &rhs->@(member.name))) {
    return false;
  }
@[  end if]@
@[end for]@
  return true;
}
@#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

@#<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
@# __copy
bool
@(message_typename)__copy(
  const @(message_typename) * input,
  @(message_typename) * output)
{
  if (!input || !output) {
    return false;
  }
@[for member in message.structure.members]@
@{
field_tn = experimental_field_typename(message_typename, member.name)
type_ = member.type
}@
  // @(member.name)
@[  if isinstance(type_, BasicType)]@
  output->@(member.name).value->data = input->@(member.name).value->data;
@[  elif isinstance(type_, (AbstractString, AbstractWString, AbstractSequence))]@
  if (!@(field_tn)__copy(&input->@(member.name), &output->@(member.name))) {
    return false;
  }
@[  elif isinstance(type_, NamespacedType)]@
@{sub_tn = idl_structure_type_to_experimental_c_typename(type_)}@
  if (!@(sub_tn)__copy(&input->@(member.name), &output->@(member.name))) {
    return false;
  }
@[  else]@
  if (!@(field_tn)__copy(&input->@(member.name), &output->@(member.name))) {
    return false;
  }
@[  end if]@
@[end for]@
  return true;
}
@#>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
