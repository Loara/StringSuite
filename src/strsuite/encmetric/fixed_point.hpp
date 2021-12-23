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

template<typename T, uint M, typename Seq, uint N, typename Seq2>
struct Fixed_point_size{
    static_assert(is_index_seq_of_len<Seq, M>, "Invalid endianess type");
    static_assert(is_index_seq_of_len<Seq2, N>, "Invalid endianess type");
    static_assert(std::floating_point<T>, "Not a floating point type");
    using ctype = T;
    static consteval uint min_bytes() noexcept { return M+N;}
    static consteval uint max_bytes() noexcept { return M+N;}
	static uint chLen(const byte *, size_t) {return M+N;}
	static validation_result validChar(const byte *, size_t l) noexcept {return validation_result{l >= static_cast<size_t>(M+N), M+N};}

	static void disassemble(const T &ev, std::int_fast64_t &int_p, std::uint_fast64_t &fra_p) noexcept{
        T i_p;
        T f_p = std::modf(ev, &i_p);
        if(f_p < 0)
            f_p = -f_p;
        int_p = static_cast<std::int_fast64_t>(i_p);
        fra_p = static_cast<std::uint_fast64_t>(std::floor(f_p * (1 << (8 * N))));
    }

    static void assemble(T &ev, const std::int_fast64_t &int_p, const std::uint_fast64_t &fra_p) noexcept{
        ev = static_cast<T>(int_p) + (int_p < 0 ? -1 : 1) * static_cast<T>(fra_p) / (1 << (8 * N));
    }

	static tuple_ret<T> decode(const byte *b, size_t s){
        if(s < M + N)
            throw buffer_small{M + N - static_cast<uint>(s)};
        auto i = Endian_enc_size<std::int_fast64_t, M, Seq>::decode_direct(b, M);
        auto f = Endian_enc_size<std::uint_fast64_t, N, Seq2>::decode_direct(b + M, N);
        T ret;
        assemble(ret, i, f);
        return tuple_ret<T>{M + N, ret};
    }

    static uint encode(const T &val, byte *out, size_t s){
        if(s < M + N)
            throw buffer_small{M + N - static_cast<uint>(s)};
        std::int_fast64_t i;
        std::uint_fast64_t f;
        disassemble(val, i, f);
        Endian_enc_size<std::int_fast64_t, M, Seq>::encode(i, out, M);
        Endian_enc_size<std::uint_fast64_t, M, Seq>::encode(f, out + M, N);
        return M + N;
    }
};

template<bool be, typename T, uint M, uint N>
using Fixed_point = Fixed_point_size<T, M, BLE_end<be, M>, N, BLE_end<be, N>>;

}
