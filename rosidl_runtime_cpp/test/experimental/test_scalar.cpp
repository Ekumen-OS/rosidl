// Copyright 2026 Ekumen, Inc.
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

#include <gtest/gtest.h>

#include <cmath>
#include <iostream>
#include <sstream>
#include <type_traits>

#include "rosidl_runtime_cpp/experimental/scalar.hpp"

TEST(rosidl_runtime_cpp_experimental_scalar, inline_storage_roundtrip)
{
  rosidl_runtime_cpp::Scalar<int> scalar(42);
  EXPECT_EQ(scalar.get(), 42);

  scalar = 7;
  EXPECT_EQ(static_cast<int>(scalar), 7);
}

TEST(rosidl_runtime_cpp_experimental_scalar, external_storage_aliasing)
{
  int value = 3;
  rosidl_memory_t memory;
  memory.address = &value;
  memory.attributes = 0;

  rosidl_runtime_cpp::Scalar<int> scalar(memory);
  EXPECT_EQ(scalar.get(), 3);

  scalar = 11;
  EXPECT_EQ(value, 11);

  value = 17;
  EXPECT_EQ(scalar.get(), 17);
}

// ===========================================================================
// Arithmetic — binary, Scalar op Scalar → Scalar
// ===========================================================================

TEST(rosidl_runtime_cpp_experimental_scalar, add_scalar_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  rosidl_runtime_cpp::Scalar<int> b(3);
  auto r = a + b;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar + Scalar must return Scalar");
  EXPECT_EQ(r.get(), 8);
}

TEST(rosidl_runtime_cpp_experimental_scalar, sub_scalar_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  rosidl_runtime_cpp::Scalar<int> b(3);
  auto r = a - b;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar - Scalar must return Scalar");
  EXPECT_EQ(r.get(), 2);
}

TEST(rosidl_runtime_cpp_experimental_scalar, mul_scalar_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  rosidl_runtime_cpp::Scalar<int> b(3);
  auto r = a * b;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar * Scalar must return Scalar");
  EXPECT_EQ(r.get(), 15);
}

TEST(rosidl_runtime_cpp_experimental_scalar, div_scalar_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(7);
  rosidl_runtime_cpp::Scalar<int> b(2);
  auto r = a / b;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar / Scalar must return Scalar");
  EXPECT_EQ(r.get(), 3);  // C++ int division truncates
}

TEST(rosidl_runtime_cpp_experimental_scalar, mod_scalar_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(7);
  rosidl_runtime_cpp::Scalar<int> b(3);
  auto r = a % b;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar % Scalar must return Scalar");
  EXPECT_EQ(r.get(), 1);
}

// ===========================================================================
// Arithmetic — binary, Scalar op builtin → Scalar
// ===========================================================================

TEST(rosidl_runtime_cpp_experimental_scalar, add_scalar_builtin)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  auto r = a + 3;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar + builtin must return Scalar");
  EXPECT_EQ(r.get(), 8);
}

TEST(rosidl_runtime_cpp_experimental_scalar, sub_scalar_builtin)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  auto r = a - 3;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar - builtin must return Scalar");
  EXPECT_EQ(r.get(), 2);
}

TEST(rosidl_runtime_cpp_experimental_scalar, mul_scalar_builtin)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  auto r = a * 3;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar * builtin must return Scalar");
  EXPECT_EQ(r.get(), 15);
}

TEST(rosidl_runtime_cpp_experimental_scalar, div_scalar_builtin)
{
  rosidl_runtime_cpp::Scalar<int> a(7);
  auto r = a / 2;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar / builtin must return Scalar");
  EXPECT_EQ(r.get(), 3);
}

TEST(rosidl_runtime_cpp_experimental_scalar, mod_scalar_builtin)
{
  rosidl_runtime_cpp::Scalar<int> a(7);
  auto r = a % 3;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar % builtin must return Scalar");
  EXPECT_EQ(r.get(), 1);
}

// ===========================================================================
// Arithmetic — binary, builtin op Scalar → Scalar
// ===========================================================================

TEST(rosidl_runtime_cpp_experimental_scalar, add_builtin_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  auto r = 3 + a;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "builtin + Scalar must return Scalar");
  EXPECT_EQ(r.get(), 8);
}

TEST(rosidl_runtime_cpp_experimental_scalar, sub_builtin_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  auto r = 3 - a;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "builtin - Scalar must return Scalar");
  EXPECT_EQ(r.get(), -2);
}

TEST(rosidl_runtime_cpp_experimental_scalar, mul_builtin_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  auto r = 3 * a;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "builtin * Scalar must return Scalar");
  EXPECT_EQ(r.get(), 15);
}

TEST(rosidl_runtime_cpp_experimental_scalar, div_builtin_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(2);
  auto r = 7 / a;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "builtin / Scalar must return Scalar");
  EXPECT_EQ(r.get(), 3);
}

TEST(rosidl_runtime_cpp_experimental_scalar, mod_builtin_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(3);
  auto r = 7 % a;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "builtin % Scalar must return Scalar");
  EXPECT_EQ(r.get(), 1);
}

// ===========================================================================
// Arithmetic — bitwise
// ===========================================================================

TEST(rosidl_runtime_cpp_experimental_scalar, bitwise_and)
{
  rosidl_runtime_cpp::Scalar<int> a(12);
  auto r = a & 10;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar & builtin must return Scalar");
  EXPECT_EQ(r.get(), 8);
}

TEST(rosidl_runtime_cpp_experimental_scalar, bitwise_or)
{
  rosidl_runtime_cpp::Scalar<int> a(12);
  auto r = a | 3;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar | builtin must return Scalar");
  EXPECT_EQ(r.get(), 15);
}

TEST(rosidl_runtime_cpp_experimental_scalar, bitwise_xor)
{
  rosidl_runtime_cpp::Scalar<int> a(12);
  auto r = a ^ 10;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar ^ builtin must return Scalar");
  EXPECT_EQ(r.get(), 6);
}

TEST(rosidl_runtime_cpp_experimental_scalar, bitwise_lshift)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  auto r = a << 2;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar << builtin must return Scalar");
  EXPECT_EQ(r.get(), 20);
}

TEST(rosidl_runtime_cpp_experimental_scalar, bitwise_rshift)
{
  rosidl_runtime_cpp::Scalar<int> a(20);
  auto r = a >> 2;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar >> builtin must return Scalar");
  EXPECT_EQ(r.get(), 5);
}

TEST(rosidl_runtime_cpp_experimental_scalar, bitwise_and_scalar_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(12);
  rosidl_runtime_cpp::Scalar<int> b(10);
  auto r = a & b;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar & Scalar must return Scalar");
  EXPECT_EQ(r.get(), 8);
}

// ===========================================================================
// Arithmetic — unary
// ===========================================================================

TEST(rosidl_runtime_cpp_experimental_scalar, unary_neg)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  auto r = -a;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "-Scalar must return Scalar");
  EXPECT_EQ(r.get(), -5);
}

TEST(rosidl_runtime_cpp_experimental_scalar, unary_pos)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  auto r = +a;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "+Scalar must return Scalar");
  EXPECT_EQ(r.get(), 5);
}

TEST(rosidl_runtime_cpp_experimental_scalar, unary_invert)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  auto r = ~a;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "~Scalar must return Scalar");
  EXPECT_EQ(r.get(), -6);
}

TEST(rosidl_runtime_cpp_experimental_scalar, unary_abs)
{
  rosidl_runtime_cpp::Scalar<int> a(-5);
  auto r = abs(a);
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "abs(Scalar) must return Scalar");
  EXPECT_EQ(r.get(), 5);
}

// ===========================================================================
// Arithmetic — in-place
// ===========================================================================

TEST(rosidl_runtime_cpp_experimental_scalar, iadd)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  auto & r = (a += 3);
  EXPECT_EQ(&r, &a);  // in-place returns *this
  EXPECT_EQ(a.get(), 8);
}

TEST(rosidl_runtime_cpp_experimental_scalar, isub)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  a -= 3;
  EXPECT_EQ(a.get(), 2);
}

TEST(rosidl_runtime_cpp_experimental_scalar, imul)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  a *= 3;
  EXPECT_EQ(a.get(), 15);
}

TEST(rosidl_runtime_cpp_experimental_scalar, idiv)
{
  rosidl_runtime_cpp::Scalar<int> a(7);
  a /= 2;
  EXPECT_EQ(a.get(), 3);
}

TEST(rosidl_runtime_cpp_experimental_scalar, imod)
{
  rosidl_runtime_cpp::Scalar<int> a(7);
  a %= 3;
  EXPECT_EQ(a.get(), 1);
}

TEST(rosidl_runtime_cpp_experimental_scalar, iand)
{
  rosidl_runtime_cpp::Scalar<int> a(12);
  a &= 10;
  EXPECT_EQ(a.get(), 8);
}

TEST(rosidl_runtime_cpp_experimental_scalar, ior)
{
  rosidl_runtime_cpp::Scalar<int> a(12);
  a |= 3;
  EXPECT_EQ(a.get(), 15);
}

TEST(rosidl_runtime_cpp_experimental_scalar, ixor)
{
  rosidl_runtime_cpp::Scalar<int> a(12);
  a ^= 10;
  EXPECT_EQ(a.get(), 6);
}

TEST(rosidl_runtime_cpp_experimental_scalar, ilshift)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  a <<= 2;
  EXPECT_EQ(a.get(), 20);
}

TEST(rosidl_runtime_cpp_experimental_scalar, irshift)
{
  rosidl_runtime_cpp::Scalar<int> a(20);
  a >>= 2;
  EXPECT_EQ(a.get(), 5);
}

// ===========================================================================
// Comparison — ordering (only == and != exist today)
// ===========================================================================

TEST(rosidl_runtime_cpp_experimental_scalar, less_than)
{
  rosidl_runtime_cpp::Scalar<int> a(3);
  rosidl_runtime_cpp::Scalar<int> b(5);
  EXPECT_LT(a, b);
  EXPECT_GT(b, a);
}

TEST(rosidl_runtime_cpp_experimental_scalar, less_equal)
{
  rosidl_runtime_cpp::Scalar<int> a(3);
  rosidl_runtime_cpp::Scalar<int> b(3);
  EXPECT_LE(a, b);
  EXPECT_LE(a, 5);
}

TEST(rosidl_runtime_cpp_experimental_scalar, greater_than)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  rosidl_runtime_cpp::Scalar<int> b(3);
  EXPECT_GT(a, b);
  EXPECT_GT(a, 3);
}

TEST(rosidl_runtime_cpp_experimental_scalar, greater_equal)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  rosidl_runtime_cpp::Scalar<int> b(5);
  EXPECT_GE(a, b);
  EXPECT_GE(a, 3);
}

// ===========================================================================
// Arithmetic — floating point
// ===========================================================================

TEST(rosidl_runtime_cpp_experimental_scalar, float_add)
{
  rosidl_runtime_cpp::Scalar<double> a(2.5);
  auto r = a + 1.5;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<double>>,
    "Scalar<double> + builtin must return Scalar<double>");
  EXPECT_DOUBLE_EQ(r.get(), 4.0);
}

TEST(rosidl_runtime_cpp_experimental_scalar, float_div)
{
  rosidl_runtime_cpp::Scalar<double> a(5.0);
  auto r = a / 2.0;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<double>>,
    "Scalar<double> / builtin must return Scalar<double>");
  EXPECT_DOUBLE_EQ(r.get(), 2.5);
}

TEST(rosidl_runtime_cpp_experimental_scalar, float_abs)
{
  rosidl_runtime_cpp::Scalar<double> a(-2.5);
  auto r = abs(a);
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<double>>,
    "abs(Scalar<double>) must return Scalar<double>");
  EXPECT_DOUBLE_EQ(r.get(), 2.5);
}

// ===========================================================================
// Arithmetic — external storage
// ===========================================================================

TEST(rosidl_runtime_cpp_experimental_scalar, external_storage_arithmetic)
{
  int value = 5;
  rosidl_memory_t memory;
  memory.address = &value;
  memory.attributes = 0;

  rosidl_runtime_cpp::Scalar<int> scalar(memory);
  auto r = scalar + 3;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "externally-backed Scalar + builtin must return Scalar");
  EXPECT_EQ(r.get(), 8);
  EXPECT_EQ(value, 5);  // binary op must not mutate the backing memory

  scalar += 3;
  EXPECT_EQ(value, 8);  // in-place op writes through to the backing memory
}

// ===========================================================================
// Arithmetic — mixed operand types (C++ usual arithmetic, narrowed to T)
// ===========================================================================

TEST(rosidl_runtime_cpp_experimental_scalar, mixed_scalar_double_times_int)
{
  rosidl_runtime_cpp::Scalar<double> a(2.5);
  auto r = a * 2;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<double>>,
    "Scalar<double> * int must return Scalar<double>");
  EXPECT_DOUBLE_EQ(r.get(), 5.0);
}

TEST(rosidl_runtime_cpp_experimental_scalar, mixed_int_times_scalar_double)
{
  rosidl_runtime_cpp::Scalar<double> a(2.5);
  auto r = 2 * a;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<double>>,
    "int * Scalar<double> must return Scalar<double>");
  EXPECT_DOUBLE_EQ(r.get(), 5.0);
}

TEST(rosidl_runtime_cpp_experimental_scalar, mixed_scalar_int_plus_double_narrows)
{
  // C++ semantics: int + double computes as double, then narrows back to T.
  rosidl_runtime_cpp::Scalar<int> a(5);
  auto r = a + 3.5;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar<int> + double must return Scalar<int>");
  EXPECT_EQ(r.get(), 8);
}

TEST(rosidl_runtime_cpp_experimental_scalar, mixed_scalar_double_plus_int)
{
  rosidl_runtime_cpp::Scalar<double> a(2.5);
  auto r = a + 1;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<double>>,
    "Scalar<double> + int must return Scalar<double>");
  EXPECT_DOUBLE_EQ(r.get(), 3.5);
}

// ===========================================================================
// Arithmetic — additional bitwise combinations
// ===========================================================================

TEST(rosidl_runtime_cpp_experimental_scalar, bitwise_or_scalar_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(12);
  rosidl_runtime_cpp::Scalar<int> b(3);
  auto r = a | b;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar | Scalar must return Scalar");
  EXPECT_EQ(r.get(), 15);
}

TEST(rosidl_runtime_cpp_experimental_scalar, bitwise_xor_scalar_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(12);
  rosidl_runtime_cpp::Scalar<int> b(10);
  auto r = a ^ b;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar ^ Scalar must return Scalar");
  EXPECT_EQ(r.get(), 6);
}

TEST(rosidl_runtime_cpp_experimental_scalar, bitwise_lshift_scalar_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  rosidl_runtime_cpp::Scalar<int> b(2);
  auto r = a << b;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar << Scalar must return Scalar");
  EXPECT_EQ(r.get(), 20);
}

TEST(rosidl_runtime_cpp_experimental_scalar, bitwise_rshift_scalar_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(20);
  rosidl_runtime_cpp::Scalar<int> b(2);
  auto r = a >> b;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "Scalar >> Scalar must return Scalar");
  EXPECT_EQ(r.get(), 5);
}

TEST(rosidl_runtime_cpp_experimental_scalar, bitwise_builtin_scalar)
{
  rosidl_runtime_cpp::Scalar<int> a(12);
  auto r = 10 & a;
  static_assert(
    std::is_same_v<decltype(r), rosidl_runtime_cpp::Scalar<int>>,
    "builtin & Scalar must return Scalar");
  EXPECT_EQ(r.get(), 8);
}

// ===========================================================================
// Arithmetic — in-place with Scalar rhs and external storage
// ===========================================================================

TEST(rosidl_runtime_cpp_experimental_scalar, iadd_scalar_rhs)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  rosidl_runtime_cpp::Scalar<int> b(3);
  a += b;  // Scalar rhs converts via operator T()
  EXPECT_EQ(a.get(), 8);
}

TEST(rosidl_runtime_cpp_experimental_scalar, external_storage_unary)
{
  int value = 5;
  rosidl_memory_t memory;
  memory.address = &value;
  memory.attributes = 0;

  rosidl_runtime_cpp::Scalar<int> scalar(memory);
  auto neg = -scalar;
  static_assert(
    std::is_same_v<decltype(neg), rosidl_runtime_cpp::Scalar<int>>,
    "-externally-backed Scalar must return Scalar");
  EXPECT_EQ(neg.get(), -5);
  EXPECT_EQ(value, 5);  // unary op must not mutate the backing memory
}

TEST(rosidl_runtime_cpp_experimental_scalar, external_storage_inplace_all)
{
  int value = 10;
  rosidl_memory_t memory;
  memory.address = &value;
  memory.attributes = 0;

  rosidl_runtime_cpp::Scalar<int> scalar(memory);
  scalar -= 3;
  EXPECT_EQ(value, 7);
  scalar *= 2;
  EXPECT_EQ(value, 14);
  scalar /= 3;
  EXPECT_EQ(value, 4);
  scalar %= 3;
  EXPECT_EQ(value, 1);
  scalar |= 4;
  EXPECT_EQ(value, 5);
  scalar &= 3;
  EXPECT_EQ(value, 1);
  scalar ^= 2;
  EXPECT_EQ(value, 3);
  scalar <<= 2;
  EXPECT_EQ(value, 12);
  scalar >>= 1;
  EXPECT_EQ(value, 6);
}

// ===========================================================================
// Equality with builtins (via implicit conversion)
// ===========================================================================

TEST(rosidl_runtime_cpp_experimental_scalar, eq_builtin)
{
  rosidl_runtime_cpp::Scalar<int> a(5);
  EXPECT_EQ(a, 5);
  EXPECT_EQ(5, a);
  EXPECT_NE(a, 6);
}

// ===========================================================================
// Stream insertion (operator<< shift must not conflict with iostream)
// ===========================================================================

TEST(rosidl_runtime_cpp_experimental_scalar, stream_insertion)
{
  rosidl_runtime_cpp::Scalar<int> a(42);
  std::ostringstream oss;
  oss << a;  // must resolve to the stream operator via operator T()
  EXPECT_EQ(oss.str(), "42");
}
