// Copyright 2020 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gtest/gtest.h"
#include "rosidl_runtime_c/message_type_support_struct.h"

const rosidl_message_type_support_t * dummy_message_typesupport_handle_function(
  const rosidl_message_type_support_t * handle, const char *) {return handle;}

TEST(message_typesupport, get_message_typesupport_handle) {
  rosidl_message_type_support_t message_typesupport;

  constexpr char identifier[] = "identifier";
  message_typesupport.typesupport_identifier = &identifier[0];
  message_typesupport.func = dummy_message_typesupport_handle_function;

  EXPECT_EQ(
    get_message_typesupport_handle(
      &message_typesupport,
      &identifier[0]), &message_typesupport);
  EXPECT_EQ(
    get_message_typesupport_handle_function(
      &message_typesupport,
      &identifier[0]), &message_typesupport);
  EXPECT_EQ(
    get_message_typesupport_handle_function(
      &message_typesupport,
      "different identifier"), nullptr);
}

TEST(message_typesupport, identifier_matches) {
  // Exact match (no trailing '*').
  EXPECT_TRUE(rosidl_runtime_c_typesupport_identifier_matches(
      "rosidl_typesupport_xcdr_cpython", "rosidl_typesupport_xcdr_cpython"));
  EXPECT_FALSE(rosidl_runtime_c_typesupport_identifier_matches(
      "rosidl_typesupport_xcdr_cpython", "rosidl_typesupport_xcdr_cpp"));

  // Trailing '*' is a prefix match.
  EXPECT_TRUE(rosidl_runtime_c_typesupport_identifier_matches(
      "rosidl_typesupport_xcdr_cpython", "rosidl_typesupport_xcdr*"));
  EXPECT_TRUE(rosidl_runtime_c_typesupport_identifier_matches(
      "rosidl_typesupport_xcdr_cpp", "rosidl_typesupport_xcdr*"));
  EXPECT_TRUE(rosidl_runtime_c_typesupport_identifier_matches(
      "rosidl_typesupport_xcdr_c", "rosidl_typesupport_xcdr*"));
  EXPECT_FALSE(rosidl_runtime_c_typesupport_identifier_matches(
      "rosidl_typesupport_introspection_cpp", "rosidl_typesupport_xcdr*"));
  // The family prefix itself also matches the family pattern.
  EXPECT_TRUE(rosidl_runtime_c_typesupport_identifier_matches(
      "rosidl_typesupport_xcdr", "rosidl_typesupport_xcdr*"));

  // A bare '*' matches everything.
  EXPECT_TRUE(rosidl_runtime_c_typesupport_identifier_matches(
      "anything", "*"));
}

TEST(message_typesupport, handle_function_matches_pattern) {
  rosidl_message_type_support_t message_typesupport;

  constexpr char identifier[] = "rosidl_typesupport_xcdr_cpython";
  message_typesupport.typesupport_identifier = &identifier[0];
  message_typesupport.func = dummy_message_typesupport_handle_function;

  // The family pattern resolves an already-concrete handle.
  EXPECT_EQ(
    get_message_typesupport_handle_function(
      &message_typesupport, "rosidl_typesupport_xcdr*"), &message_typesupport);
  // A non-matching family returns nullptr.
  EXPECT_EQ(
    get_message_typesupport_handle_function(
      &message_typesupport, "rosidl_typesupport_fastrtps*"), nullptr);
}
