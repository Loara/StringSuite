#pragma once
/*
    This file is part of Encmetric.
    Copyright (C) 2021 Paolo De Donato.

    Encmetric is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Encmetric is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Encmetric. If not, see <http://www.gnu.org/licenses/>.
*/
/*
    Some usebul bitwise operations using bitmasks that can be built at compile-time
*/
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <stdexcept>

namespace sts{
using uint = unsigned int;
/*
    Needed an 8-bit byte in order to work with encodings
*/
using std::uint8_t;
//enum class byte : uint8_t {};
using std::byte;
static_assert(sizeof(uint8_t) == sizeof(byte), "byte has not 8 bits");

inline bool compare(const byte *a, const byte *b, int nsiz) noexcept{
	return std::memcmp(a, b, nsiz) == 0;
}

/*
    Create a bit mask of type RET with ones at positions oth.

    For example compose_bit_mask<int>(1, 4, 3) returns 26=11010b as an int 
*/
template<typename RET, typename IntegerType>
constexpr RET compose_bit_mask(IntegerType bit){
	static_assert(std::is_integral_v<IntegerType>, "Not integral type");
	if(bit < 0 || static_cast<size_t>(bit) >= (sizeof(RET)*8))
		throw std::out_of_range("Invalid bit position");
	return RET{1} << bit;
}

template<typename RET, typename IntegerType, typename... T>
constexpr RET compose_bit_mask(IntegerType bit, T... oth){
	return compose_bit_mask<RET>(bit) | compose_bit_mask<RET>(oth...);
}

/*
    Bit testing operations using bitmasks:
     - bit_zero return true when test has all zeroes at the positions all 
     - bit_one return true when test has all ones at the positions all 
*/
template<typename B, typename... T>
constexpr bool bit_zero(B test, T... all){
	return (test & compose_bit_mask<B>(all...)) == B{0};
}

template<typename B, typename... T>
constexpr bool bit_one(B test, T... all){
	B mask = compose_bit_mask<B>(all...);
	return (test & mask) == mask;
}

/*
    Set/reset the specified bits:
     - set_bits sets these bits
     - reset_bits resets these bits
     - leave_bits resets all the remaining bits leaving the remaining unchanged
     - swap_bits swaps these bits
*/
template<typename B, typename... T>
constexpr void set_bits(B &mask, T... all){
	mask |= compose_bit_mask<B>(all...);
}

template<typename B, typename... T>
constexpr void reset_bits(B &mask, T... all){
	mask &= ~compose_bit_mask<B>(all...);
}

template<typename B, typename... T>
constexpr void leave_bits(B &mask, T... all){
	mask &= compose_bit_mask<B>(all...);
}

template<typename B, typename... T>
constexpr void swap_bits(B &mask, T... all){
	mask ^= compose_bit_mask<B>(all...);
}


template<typename B, typename... T>
constexpr B set_b(B mask, T... all){
	return mask | compose_bit_mask<B>(all...);
}

template<typename B, typename... T>
constexpr B reset_b(B mask, T... all){
	return mask & ~compose_bit_mask<B>(all...);
}

template<typename B, typename... T>
constexpr B leave_b(B mask, T... all){
	return mask & compose_bit_mask<B>(all...);
}

template<typename B, typename... T>
constexpr B swap_b(B mask, T... all){
	return mask ^ compose_bit_mask<B>(all...);
}

/*
    access an endianess-dependend arrays as a big endian array

    bool be = true if big endian
    dim = dimension of object in byte
    i = i-th byte you access normally if vour object is stored BE
*/

inline constexpr int acc(bool be, int dim, int i){
	return be ? i : dim-1-i;
}

template<typename T>
constexpr T& access(T *by, bool be, int dim, int i){ return by[acc(be, dim, i)];}

inline constexpr bool byte_less(byte a, byte b) noexcept{
    return static_cast<uint>(a) < static_cast<uint>(b);
}

inline constexpr byte byte_plus(byte a, byte b) noexcept{
    return static_cast<byte>(static_cast<uint>(a) + static_cast<uint>(b));
}
inline constexpr byte byte_minus(byte a, byte b) noexcept{
    return static_cast<byte>(static_cast<uint>(a) - static_cast<uint>(b));
}
}


