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
#include <strsuite/encmetric/endianess.hpp>
#include <cmath>

namespace sts{

    /*
     * Store floating point numbers as fixed point
     * M = integer part size in bytes
     * N = fractional part in bytes
     */
template<typename T, uint M, uint N, typename Seq>
struct Fixed_point_size{
    static_assert(is_index_seq_of_len<Seq, M + N>, "Invalid endianess type");
    static_assert(std::floating_point<T>, "Not a floating point type");
    using ctype = T;
    using integer_type = s_make_size_t<M + N>;
    static consteval uint min_bytes() noexcept { return M + N;}
    static consteval uint max_bytes() noexcept { return M + N;}
	static uint chLen(const byte *, size_t) {return M+N;}
	static validation_result validChar(const byte *, size_t l) noexcept {return validation_result{l >= static_cast<size_t>(M+N), M+N};}

	static tuple_ret<T> decode(const byte *b, size_t s){
        if(s < M + N)
            throw buffer_small{M + N - static_cast<uint>(s)};
        integer_type i = Endian_enc_size<integer_type, M + N, Seq>::decode_direct(b, s);
        extends_sign<M + N>(i);
        return tuple_ret<T>{M + N, std::ldexp(static_cast<T>(i), -(8 * N))};
    }

    static uint encode(const T &val, byte *out, size_t s){
        if(s < M + N)
            throw buffer_small{M + N - static_cast<uint>(s)};
        integer_type i = static_cast<integer_type>(std::floor(std::ldexp(val, 8 * N)));
        return Endian_enc_size<integer_type, M+N, Seq>::encode(i, out, s);
    }
};

template<bool be, typename T, uint M, uint N>
using Fixed_point = Fixed_point_size<T, M, N, BLE_end<be, M + N>>;

template<bool be, typename T = float>
using Fix16 = Fixed_point<be, T, 2, 2>;

}
