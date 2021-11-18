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
 * You should never include directly this header, but instead include always utf16_enc.hpp
 * */
#include <strsuite/encmetric/byte_tools.hpp>
#include <strsuite/encmetric/encoding.hpp>
#include <strsuite/encmetric/endianess.hpp>

namespace sts{

template<typename Seq>
class UTF16{
    static_assert(is_index_seq_of_len<Seq, 2>, "Invalid endianess type");
	public:
		using ctype=unicode;
		static consteval uint min_bytes() noexcept {return 2;}
		static consteval uint max_bytes() noexcept {return 4;}
		static consteval uint fixed_head() noexcept {return 2;}
		static uint chLen(const byte *, size_t);
		static validation_result validChar(const byte *, size_t) noexcept;
		static uint decode(unicode *uni, const byte *by, size_t l);
		static uint encode(const unicode &uni, byte *by, size_t l);
};
using UTF16LE = UTF16<LE_end<2>>;
using UTF16BE = UTF16<BE_end<2>>;

}
