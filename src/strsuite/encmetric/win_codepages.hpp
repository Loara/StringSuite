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
#include <strsuite/encmetric/ascii_extensions.hpp>

namespace sts{
	class Win_1252 : ASCII_extension<Win_1252>{
		public:
			static const unsigned int table[];
	};
	class Win_1250 : ASCII_extension<Win_1250>{
		public:
			static const unsigned int table[];
	};

    class Win_1251{
	public:
		using ctype=unicode;
        using enc_base=ASCII;
        static const unsigned int table[];
		static constexpr uint min_bytes() noexcept {return 1;}
		static constexpr bool has_max() noexcept {return true;}
		static constexpr uint max_bytes() noexcept {return 1;}
		static constexpr uint chLen(const byte *, size_t) {
            return 1;
        }
		static validation_result validChar(const byte *b, size_t siz) noexcept {return validation_result{siz >= 1 && b[0] != byte{0x98}, 1};}
		static uint decode(unicode *uni, const byte *by, size_t l){
			if(l == 0)
				throw buffer_small{1};
			if(bit_zero(*by, 7)){
				*uni = read_unicode(by[0]);
				return 1;
			}
			else{
				int idx = std::to_integer<int>(by[0]);
				idx -= 0x80;
				*uni = unicode{Win_1251::table[idx]};
				return 1;
			}
		}
		static uint encode(const unicode &uni, byte *by, size_t l){
			if(l == 0)
				throw buffer_small{1};
			if(uni < 0x80){
				*by = byte{static_cast<uint8_t>(uni)};
				return 1;
			}
			else{
				int idx = -1;
				for(int i=0; i < 0x80; i++){
                    if(i == 0x18)
                        continue;
					if(Win_1251::table[i] == uni){
						idx = i;
						break;
					}
				}
				if(idx == -1)
					throw encoding_error("Character not included in this encoding");
				*by = byte{static_cast<uint8_t>(idx + 0x80)};
				return 1;
			}
		}
    };
}
