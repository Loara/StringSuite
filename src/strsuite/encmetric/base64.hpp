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

#include <strsuite/encmetric/encoding.hpp>
#include <strsuite/encmetric/enc_string.hpp>

namespace sts{

struct three_byte{
	byte *bytes;
	uint nbyte;
};

class Base64_padding{
	public:
		using ctype=three_byte;
		static constexpr unsigned char recode[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
		static int find(byte b) noexcept{
			if(std::to_integer<unsigned char>(b) == '=')
				return 64;		
			for(int i=0; i<64; i++){
				if(std::to_integer<unsigned char>(b) == recode[i])
					return i;
			}
			return -1;
		}
		static constexpr uint min_bytes() noexcept {return 4;}
		static constexpr bool has_max() noexcept {return true;}
		static constexpr uint max_bytes() noexcept {return 4;}
		static constexpr uint chLen(const byte *, size_t) {return 4;}
		static bool validChar(const byte *, uint &) noexcept;
		static uint decode(three_byte *uni, const byte *by, size_t l);
		static uint encode(const three_byte &uni, byte *by, size_t l);
};

void base64_encode(const byte *from, byte *to, size_t siz);
void base64_decode(const byte *from, byte *to, size_t siz);

}
