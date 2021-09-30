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
#include <strsuite/encmetric/byte_tools.hpp>

namespace sts{

struct jisx_213{
    // 1 <= row, col <= 94
    //if row=0 then col is jisx 201 (ASCII + half katakana)
    byte row;
    byte col;
    enum PLANE{PLANE_1, PLANE_2};
    PLANE plane;
};

class EUC_JP{
	public:
		using ctype=jisx_213;
		static constexpr uint min_bytes() noexcept {return 1;}
		static constexpr bool has_max() noexcept {return true;}
		static constexpr uint max_bytes() noexcept {return 3;}
		static uint chLen(const byte *, size_t);
		static validation_result validChar(const byte *, size_t) noexcept;
		static uint decode(jisx_213 *uni, const byte *by, size_t l);
		static uint encode(const jisx_213 &uni, byte *by, size_t l);
};
/*
 *
 * s1 = [ (r + 0x101) / 2       if  p=1, 1<=r<=0x3e         0x81-0x9f
 *        (r + 0x181) / 2       if  p=1, 0x3f<=r<=0x5e      0xe0-0xef
 *        (r + 0x1df) / 2       if  p=2, r=1,3,4,5          0xf0, 0xf1, 0xf1, 0xf2 (A)
 *        (r + 0x1df) / 2 - 3   if  p=2, r=8,12,13,14,15    0xf0, 0xf2, 0xf3, 0xf3, 0xf4 (B)
 *        (r + 0x19b) / 2       if  p=2, 0x4e<=r<=0x5e]     0xf4-0xfc
 *
 * s2 = [ c + 0x3f              if r odd, 0x1<=c<=0x3f      0x40-0x7e
 *        c + 0x40              if r odd, 0x40<=c<=0x5e     0x80-0x9e
 *        c + 0x9e              if r even]                  0x9f-0xfc
 *
 * Tra A e B si potrebbe pensare a delle collisioni, per esempio decodificare 0xf1 porterebbe
 * 1) r = 3, 4
 * 2) r = 9, 10
 * ma dato che 9, 10 non appartengono alla lista non vi sono ambiguità,
 * eventuali collisioni poi sarebbero evitate grazie alla differente parità
 */
class SHIFT_JIS{
	public:
		using ctype=jisx_213;
		static constexpr uint min_bytes() noexcept {return 1;}
		static constexpr bool has_max() noexcept {return true;}
		static constexpr uint max_bytes() noexcept {return 2;}
		static uint chLen(const byte *, size_t);
		static validation_result validChar(const byte *, size_t) noexcept;
		static uint decode(jisx_213 *uni, const byte *by, size_t l);
		static uint encode(const jisx_213 &uni, byte *by, size_t l);
};

}
