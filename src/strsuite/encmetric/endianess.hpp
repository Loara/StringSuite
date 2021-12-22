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
#include <strsuite/encmetric/encoding.hpp>

namespace sts{

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
    static constexpr uint min_bytes() noexcept{ return N;}
    static constexpr bool has_max() noexcept {return true;}
    static constexpr uint max_bytes() noexcept{ return N;}
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

template<typename T, unsigned int N, typename Seq> requires std::signed_integral<T>
class Endian_enc_size<T, N, Seq>{
public:
    static_assert(is_index_seq_of_len<Seq, N>, "Invalid endianess type");
    using ctype=T;
    using unsigned_ctype=std::make_unsigned_t<T>;
    static constexpr uint min_bytes() noexcept{ return N;}
    static constexpr bool has_max() noexcept {return true;}
    static constexpr uint max_bytes() noexcept{ return N;}
	static uint chLen(const byte *, size_t siz) {return N;}
	static validation_result validChar(const byte *, size_t l) noexcept {return validation_result{l >= static_cast<size_t>(N), N};}
	static tuple_ret<T> decode(const byte *by, size_t l){
        unsigned_ctype iv;
        uint n;
        std::tie(n, iv) = Endian_enc_size<unsigned_ctype, N, Seq>::decode(by, l);
        return tuple_ret<T>{n, static_cast<T>(iv)};
    }
    static T decode_direct(const byte *by, size_t l){
        return std::get<1>(decode(by, l));
    }
	static uint encode(const T &uni, byte *by, size_t l){
        return Endian_enc_size<unsigned_ctype, N, Seq>::encode(static_cast<unsigned_ctype>(uni), by, l);
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
