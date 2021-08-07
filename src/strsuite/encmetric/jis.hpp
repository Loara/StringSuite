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
*/
}
