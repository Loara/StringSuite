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
#include <concepts>
#include <utility>
#include <bit>
#include <strsuite/encmetric/encoding.hpp>
#include <strsuite/encmetric/byte_tools.hpp>

namespace sts{
/*
 * An endianess class is just an std::index_sequence<n0, n1, n2, ... n(K-1)> sequence of integers from 0 to K-1
 * with element ni saying the (i+1)-th byte in the encoded sequence must be multiplied by 2^(8 * i) in order to decode it
 *
 * little endian -> std::index_sequence<0, 1, 2, .., K-1>
 * big endian    -> std::index_sequence<K-1, K-2, ..., 0>
 */

template<typename T, size_t... base, size_t... inc>
constexpr void decode_help(T *uni, const byte *b, std::index_sequence<base...>, std::index_sequence<inc...>) noexcept{
    *uni = T{0};
    ((*uni = *uni | static_cast<T>(static_cast<size_t>(b[base]) << (8 * inc))), ...);
}

template<typename T, size_t... base, size_t... inc>
constexpr void encode_help(const T &uni, byte *b, std::index_sequence<base...>, std::index_sequence<inc...>) noexcept{
    ((b[base] = static_cast<byte>(uni >> (8 * inc))), ...);
}

template<typename T, unsigned int N, typename Seq>
class Endian_enc_size;

template<typename T, unsigned int N, typename Seq> requires std::unsigned_integral<T>
class Endian_enc_size<T, N, Seq>{
public:
    static_assert(is_index_seq_of_len<Seq, N>, "Invalid endianess type");
    using ctype=T;
    static consteval uint min_bytes() noexcept{ return N;}
    static consteval uint max_bytes() noexcept{ return N;}
	static uint chLen(const byte *, size_t) {return N;}
	static validation_result validChar(const byte *, size_t l) noexcept {return validation_result{l >= static_cast<size_t>(N), N};}
	static tuple_ret<T> decode(const byte *by, size_t l){
        if(l < N)
            throw buffer_small{N - static_cast<uint>(l)};
        T uni{};
        decode_help(&uni, by, std::make_index_sequence<N>{}, Seq{});
        return tuple_ret<T>{N, uni};
    }
    static T decode_direct(const byte *by, size_t l){
        return std::get<1>(decode(by, l));
    }
	static uint encode(const T &uni, byte *by, size_t l){
        if(l < N)
            throw buffer_small{N - static_cast<uint>(l)};
        encode_help(uni, by, std::make_index_sequence<N>{}, Seq{});
        return N;
    }
};

template<size_t N>
struct u_make_size;

template<>
struct u_make_size<1>{
    using type = std::uint8_t;
    using stype = std::int8_t;
};

template<>
struct u_make_size<2>{
    using type = std::uint16_t;
    using stype = std::int16_t;
};

template<>
struct u_make_size<4>{
    using type = std::uint32_t;
    using stype = std::int32_t;
};

template<>
struct u_make_size<8>{
    using type = std::uint64_t;
    using stype = std::int64_t;
};

template<size_t N>
using u_make_size_t = typename u_make_size<N>::type;
template<size_t N>
using s_make_size_t = typename u_make_size<N>::stype;

template<typename T>
using u_same_size_t = u_make_size_t<sizeof(T)>;

template<typename T>
constexpr u_same_size_t<T> to_unsigned(const T &t) noexcept{
    return std::bit_cast<u_same_size_t<T>>(t);
}

template<typename T>
constexpr T from_unsigned(const u_same_size_t<T> &t) noexcept{
    return std::bit_cast<T>(t);
}

/*
 * Encoding for all types that are convertible to and from unsigned integers
 *
 * Notice for signed integers: when sizeof(T) > N once you decode a number you should extend its sign
 * with 'constexpr void sts::extends_sign(Int &val) noexcept' function
 */
template<typename T, unsigned int N, typename Seq> requires std::is_scalar_v<T> && (!std::unsigned_integral<T>)
class Endian_enc_size<T, N, Seq>{
public:
    static_assert(is_index_seq_of_len<Seq, N>, "Invalid endianess type");
    using ctype=T;
    using unsigned_ctype=u_same_size_t<T>;
    static constexpr uint min_bytes() noexcept{ return N;}
    static constexpr uint max_bytes() noexcept{ return N;}
	static uint chLen(const byte *, size_t siz) {return N;}
	static validation_result validChar(const byte *, size_t l) noexcept {return validation_result{l >= static_cast<size_t>(N), N};}
	static tuple_ret<T> decode(const byte *by, size_t l){
        unsigned_ctype au = Endian_enc_size<unsigned_ctype, N, Seq>::decode_direct(by, l);
        return tuple_ret<T>{N, from_unsigned<T>(au)};
    }
    static T decode_direct(const byte *by, size_t l){
        return std::get<1>(decode(by, l));
    }
	static uint encode(const T &uni, byte *by, size_t l){
        return Endian_enc_size<unsigned_ctype, N, Seq>::encode(to_unsigned(uni), by, l);
    }
};

template<bool be, unsigned int N>
using BLE_end = std::conditional_t<be, sts::make_rev_index_sequence<N>, std::make_index_sequence<N>>;

template<unsigned int N>
struct PDP_end_h{
    static_assert(N % 2 == 0, "Odd length for a PDP encoding");
    using end = typename push_value<typename push_value<typename PDP_end_h<N-2>::end>::apply_f<N-1>>::apply_f<N-2>;
};

template<>
struct PDP_end_h<0>{
    using end=std::index_sequence<>;
};
template<>
struct PDP_end_h<1>{
    using end=std::index_sequence<0>;
};

template<unsigned int N>
using PDP_end = PDP_end_h<N>::end;

template<unsigned int N>
using BE_end = BLE_end<true, N>;
template<unsigned int N>
using LE_end = BLE_end<false, N>;

template<bool be, typename T, unsigned int N = sizeof(T)>
using Endian_enc = Endian_enc_size<T, N, BLE_end<be, N>>;

}
