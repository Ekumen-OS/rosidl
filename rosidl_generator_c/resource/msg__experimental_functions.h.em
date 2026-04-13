@# Included from rosidl_generator_c/resource/idl__experimental_functions.h.em
@{
from rosidl_generator_c import interface_path_to_string
from rosidl_generator_c.experimental import idl_structure_type_to_experimental_c_typename

message_typename = idl_structure_type_to_experimental_c_typename(
    message.structure.namespaced_type)
}@
// Experimental message functions for @(interface_path_to_string(interface_path)) in @(package_name).

/// Initialize @(interface_path_to_string(interface_path)) experimental message.
/**
 * All fields are set to their default zero-initialized state using the
 * default allocator.
 * If called twice without an intervening @(message_typename)__fini()
 * the previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * \return true if initialization was successful, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_@(package_name)
bool
@(message_typename)__init(@(message_typename) * msg);

/// Initialize @(interface_path_to_string(interface_path)) experimental message with allocator.
/**
 * All fields are set to their default zero-initialized state using the
 * provided allocator (or the default allocator when NULL).
 * If called twice without an intervening @(message_typename)__fini()
 * the previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * \param[in] allocator Allocator for managed field storage; NULL for default.
 * \return true if initialization was successful, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_@(package_name)
bool
@(message_typename)__init_with_allocator(
  @(message_typename) * msg,
  const rcutils_allocator_t * allocator);

/// Initialize @(interface_path_to_string(interface_path)) experimental message from external storage.
/**
 * Each field is initialized to point into the memory regions described by
 * \p storage.  All regions in \p storage must be valid (non-NULL address and
 * sufficient size); the call fails if any region is insufficient.
 * \param[in,out] msg The previously allocated message pointer.
 * \param[in] storage Fully populated external storage descriptor.
 * \return true if initialization was successful, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_@(package_name)
bool
@(message_typename)__init_from_storage(
  @(message_typename) * msg,
  const @(message_typename)__ExternalStorage * storage);

/// Finalize @(interface_path_to_string(interface_path)) experimental message.
/**
 * Releases all heap-allocated field memory.  The message pointer itself is
 * not freed; use @(message_typename)__destroy() for that.
 * \param[in,out] msg The initialized message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_@(package_name)
void
@(message_typename)__fini(@(message_typename) * msg);

/// Create @(interface_path_to_string(interface_path)) experimental message.
/**
 * Allocates and zero-initializes the message, then calls
 * @(message_typename)__init().
 * \return Pointer to the initialized message, or NULL on failure.
 */
ROSIDL_GENERATOR_C_PUBLIC_@(package_name)
@(message_typename) *
@(message_typename)__create();

/// Destroy @(interface_path_to_string(interface_path)) experimental message.
/**
 * Calls @(message_typename)__fini() and then frees the message memory using
 * the default allocator.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_@(package_name)
void
@(message_typename)__destroy(@(message_typename) * msg);

/// Check equality of two @(interface_path_to_string(interface_path)) experimental messages.
/**
 * \param[in] lhs Left-hand side message pointer.
 * \param[in] rhs Right-hand side message pointer.
 * \return true if the messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_@(package_name)
bool
@(message_typename)__are_equal(
  const @(message_typename) * lhs,
  const @(message_typename) * rhs);

/// Deep-copy a @(interface_path_to_string(interface_path)) experimental message.
/**
 * \param[in]  input  Source message pointer.
 * \param[out] output Destination message pointer (must already be initialized).
 * \return true if successful, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_@(package_name)
bool
@(message_typename)__copy(
  const @(message_typename) * input,
  @(message_typename) * output);