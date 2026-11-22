// generated from rosidl_generator_c/resource/idl__views.h.em
// with input from @(package_name):@(interface_path)
// generated code does not contain a copyright notice
@{
from rosidl_generator_c import basetype_to_c, idl_type_to_c, idl_structure_type_to_c_typename
from rosidl_parser.definition import AbstractSequence, AbstractString, AbstractWString
from rosidl_parser.definition import Array, BasicType, Message, NamespacedType, Service
from rosidl_pycommon import convert_camel_case_to_lower_case_underscore

parts = [package_name] + list(interface_path.parent.parts)
stem = convert_camel_case_to_lower_case_underscore(interface_path.stem)
guard = '__'.join(part.upper() for part in parts + ['views', stem]) + '_H_'
messages = list(content.get_elements_of_type(Message))
for service in content.get_elements_of_type(Service):
    messages.extend([service.request_message, service.response_message])
nested_types = set()
for message in messages:
    for member in message.structure.members:
        t = member.type.value_type if isinstance(member.type, (Array, AbstractSequence)) else member.type
        if isinstance(t, NamespacedType):
            nested_types.add(idl_structure_type_to_c_typename(t))
region_types = sorted(nested_types | {
    idl_structure_type_to_c_typename(message.structure.namespaced_type) for message in messages})
}@
#ifndef @(guard)
#define @(guard)

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "rcutils/error_handling.h"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_runtime_c/service_type_support_struct.h"
#include "@('/'.join(parts))/@(stem).h"

#ifdef __cplusplus
extern "C"
{
#endif

/// Opaque borrowed region views and constraint handles.
/// Views and returned field pointers expire with the loan; resizing a container
/// invalidates its element views. Read-only loans must use ConstRegionView.
@[for type_name in region_types]@
typedef struct @(type_name)__RegionView @(type_name)__RegionView;
typedef struct @(type_name)__ConstRegionView @(type_name)__ConstRegionView;
typedef struct @(type_name)__RegionConstraints @(type_name)__RegionConstraints;
@[end for]@

@[for message in messages]@
@{name = idl_structure_type_to_c_typename(message.structure.namespaced_type)}@
const rosidl_message_type_support_t *
@(name)__RegionView__get_type_support(void);

/// Destroy only handles returned by create(), not borrowed nested handles.
/// Constraints must outlive every context that refers to their data.
@(name)__RegionConstraints * @(name)__RegionConstraints__create(void);
void @(name)__RegionConstraints__destroy(@(name)__RegionConstraints * constraints);
const void * @(name)__RegionConstraints__data(const @(name)__RegionConstraints * constraints);

static inline @(name)__RegionView *
@(name)__RegionView__from_loan(void * loan)
{
  if (!loan) {
    RCUTILS_SET_ERROR_MSG("Region loan is null");
    return NULL;
  }
  return (@(name)__RegionView *)loan;
}

static inline const @(name)__ConstRegionView *
@(name)__RegionView__as_const(const @(name)__RegionView * view)
{
  if (!view) {
    RCUTILS_SET_ERROR_MSG("Region view is null");
    return NULL;
  }
  return (const @(name)__ConstRegionView *)view;
}

static inline const @(name)__ConstRegionView *
@(name)__ConstRegionView__from_loan(const void * loan)
{
  if (!loan) {
    RCUTILS_SET_ERROR_MSG("Region loan is null");
    return NULL;
  }
  return (const @(name)__ConstRegionView *)loan;
}

@[for member in message.structure.members]@
@{
t = member.type
container = isinstance(t, (Array, AbstractSequence))
elem = t.value_type if container else t
index = ', size_t index' if container else ''
prefix = name + '__RegionView__' + member.name
const_prefix = name + '__ConstRegionView__' + member.name
}@
@[if isinstance(t, AbstractSequence)]@
bool @(name)__RegionConstraints__@(member.name)__set_size(
  @(name)__RegionConstraints * constraints, size_t size);
@[end if]@
@[if isinstance(elem, (AbstractString, AbstractWString))]@
bool @(name)__RegionConstraints__@(member.name)__set_string_size(
  @(name)__RegionConstraints * constraints, size_t size);
@[elif isinstance(elem, NamespacedType)]@
@(idl_structure_type_to_c_typename(elem))__RegionConstraints *
@(name)__RegionConstraints__@(member.name)__get(@(name)__RegionConstraints * constraints);
@[end if]@
@[if container]@
bool @(const_prefix)__size(const @(name)__ConstRegionView * view, size_t * size);
@[if isinstance(t, AbstractSequence)]@
bool @(prefix)__resize(@(name)__RegionView * view, size_t size);
@[end if]@
@[end if]@
@[if isinstance(elem, BasicType)]@
bool @(const_prefix)__get(
  const @(name)__ConstRegionView * view@(index), @(basetype_to_c(elem)) * value);
bool @(prefix)__set(@(name)__RegionView * view@(index), @(basetype_to_c(elem)) value);
@[elif isinstance(elem, NamespacedType)]@
const @(idl_structure_type_to_c_typename(elem))__ConstRegionView *
@(const_prefix)__get(const @(name)__ConstRegionView * view@(index));
@(idl_structure_type_to_c_typename(elem))__RegionView *
@(prefix)__get(@(name)__RegionView * view@(index));
@[elif isinstance(elem, AbstractString)]@
bool @(const_prefix)__get(
  const @(name)__ConstRegionView * view@(index), const char ** data, size_t * size);
bool @(prefix)__set(
  @(name)__RegionView * view@(index), const char * data, size_t size);
@[elif isinstance(elem, AbstractWString)]@
bool @(const_prefix)__copy(
  const @(name)__ConstRegionView * view@(index), uint16_t * data, size_t capacity, size_t * size);
bool @(prefix)__set(
  @(name)__RegionView * view@(index), const uint16_t * data, size_t size);
@[end if]@

@[end for]@
/// Plain-message view. Initialize with {0}; do not copy an initialized view.
typedef struct @(name)__View
{
  @(name) * message;
  bool owns_message;
} @(name)__View;

/// Const-qualified plain-message view. The message must outlive the view and its references.
/// C const is shallow: pointers inside strings, sequences, and nested messages remain mutable.
typedef struct @(name)__ConstView
{
  const @(name) * message;
} @(name)__ConstView;

static inline bool
@(name)__View__init_owned(@(name)__View * view)
{
  if (!view || view->message) {
    RCUTILS_SET_ERROR_MSG("View must be non-null and uninitialized");
    return false;
  }
  view->message = @(name)__create();
  if (!view->message) {
    RCUTILS_SET_ERROR_MSG("Failed to create view message");
    return false;
  }
  view->owns_message = true;
  return true;
}

static inline bool
@(name)__View__init_borrowed(@(name)__View * view, @(name) * message)
{
  if (!view || view->message || !message) {
    RCUTILS_SET_ERROR_MSG("View and message must be valid and view uninitialized");
    return false;
  }
  view->message = message;
  view->owns_message = false;
  return true;
}

static inline bool
@(name)__ConstView__init_borrowed(@(name)__ConstView * view, const @(name) * message)
{
  if (!view || view->message || !message) {
    RCUTILS_SET_ERROR_MSG("View and message must be valid and view uninitialized");
    return false;
  }
  view->message = message;
  return true;
}

static inline bool
@(name)__View__fini(@(name)__View * view)
{
  if (!view || !view->message) {
    RCUTILS_SET_ERROR_MSG("View is not initialized");
    return false;
  }
  if (view->owns_message) {
    @(name)__destroy(view->message);
  }
  view->message = NULL;
  view->owns_message = false;
  return true;
}

static inline bool
@(name)__ConstView__fini(@(name)__ConstView * view)
{
  if (!view || !view->message) {
    RCUTILS_SET_ERROR_MSG("View is not initialized");
    return false;
  }
  view->message = NULL;
  return true;
}

static inline bool
@(name)__ConstView__release(@(name)__ConstView * view, const @(name) ** message)
{
  if (!view || !view->message || !message) {
    RCUTILS_SET_ERROR_MSG("View and output pointer must be valid");
    return false;
  }
  *message = view->message;
  view->message = NULL;
  return true;
}

/// Consume the view, returning the underlying message and its ownership status.
static inline bool
@(name)__View__release(
  @(name)__View * view, @(name) ** message, bool * was_owned)
{
  if (!view || !view->message || !message || !was_owned) {
    RCUTILS_SET_ERROR_MSG("View and output pointers must be valid");
    return false;
  }
  *message = view->message;
  *was_owned = view->owns_message;
  view->message = NULL;
  view->owns_message = false;
  return true;
}

@[for member in message.structure.members]@
@{
type_ = member.type
field_type = basetype_to_c(type_.value_type) if isinstance(type_, Array) else idl_type_to_c(type_)
}@
static inline @(field_type) *
@(name)__View__get__@(member.name)(@(name)__View * view)
{
  if (!view || !view->message) {
    RCUTILS_SET_ERROR_MSG("View is not initialized");
    return NULL;
  }
  return @('view->message->' + member.name if isinstance(type_, Array) else '&view->message->' + member.name);
}

static inline const @(field_type) *
@(name)__ConstView__get__@(member.name)(const @(name)__ConstView * view)
{
  if (!view || !view->message) {
    RCUTILS_SET_ERROR_MSG("View is not initialized");
    return NULL;
  }
  return @('view->message->' + member.name if isinstance(type_, Array) else '&view->message->' + member.name);
}

@[end for]@
@[end for]@
@[for service in content.get_elements_of_type(Service)]@
@{service_name = idl_structure_type_to_c_typename(service.namespaced_type)}@
const rosidl_service_type_support_t *
@(service_name)__RegionView__get_type_support(void);
@[end for]@
#ifdef __cplusplus
}
#endif

#endif  // @(guard)
