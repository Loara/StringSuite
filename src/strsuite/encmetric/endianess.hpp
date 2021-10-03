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

template<bool be, typename T, unsigned int N>
class Endian_enc_size;

template<bool be, typename T, unsigned int N > requires std::unsigned_integral<T>
class Endian_enc_size{
public:
    using ctype=T;
    static constexpr uint min_bytes() noexcept{ return N;}
    static constexpr bool has_max() noexcept {return true;}
    static constexpr uint max_bytes() noexcept{ return N;}
	static uint chLen(const byte *, size_t) {return N;}
	static validation_result validChar(const byte *, size_t l) noexcept {return validation_result{l >= static_cast<size_t>(N), N};}
	static uint decode(T *uni, const byte *by, size_t l){
        if(l < N)
            throw buffer_small{N - static_cast<uint>(l)};
        *uni = T{0};
        if constexpr(be){
            for(uint i=0; i < N; i++){
                *uni = (*uni * 0x100u) + static_cast<T>(by[i]);
            }
            return N;
        }
        else{
            for(uint i=0; i < N ; i++){
                *uni = (*uni * 0x100u) + static_cast<T>(by[N - 1 - i]);
            }
            return N;
        }
    }
	static uint encode(const T &uni, byte *by, size_t l){
        if(l < N)
            throw buffer_small{N - static_cast<uint>(l)};
        T temp = uni;
        if constexpr(!be){
            for(uint i=0; i < N; i++){
                by[i] = static_cast<byte>(temp & 0xffu);
                temp /= 0x100u;
            }
            return N;
        }
        else{
            for(uint i=0; i < N; i++){
                by[N - 1 - i] = static_cast<byte>(temp & 0xffu);
                temp /= 0x100u;
            }
            return N;
        }
    }
};

template<bool be, typename T, unsigned int N > requires std::signed_integral<T>
class Endian_enc_size<be, T, N>{
public:
    using ctype=T;
    using unsigned_ctype=std::make_unsigned_t<T>;
    static constexpr uint min_bytes() noexcept{ return N;}
    static constexpr bool has_max() noexcept {return true;}
    static constexpr uint max_bytes() noexcept{ return N;}
	static uint chLen(const byte *, size_t siz) {return N;}
	static validation_result validChar(const byte *, size_t l) noexcept {return validation_result{l >= static_cast<size_t>(N), N};}
	static uint decode(T *uni, const byte *by, size_t l){
        unsigned_ctype iv;
        uint ret = Endian_enc_size<be, unsigned_ctype, N>::decode(&iv, by, l);
        *uni = static_cast<T>(iv);
        return ret;
    }
	static uint encode(const T &uni, byte *by, size_t l){
        return Endian_enc_size<be, unsigned_ctype, N>::encode(static_cast<unsigned_ctype>(uni), by, l);
    }
};

template<bool be, typename T>
using Endian_enc = Endian_enc_size<be, T, sizeof(T)>;

}
