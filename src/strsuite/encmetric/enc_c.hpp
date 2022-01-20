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
#include <strsuite/encmetric/enc_c_0.hpp>

namespace sts{

extern template class twrite_pt<ASCII>;
extern template class tread_pt<ASCII>;
extern template class adv_string_view<ASCII>;
extern template class adv_string<ASCII>;

extern template class twrite_pt<UTF8>;
extern template class tread_pt<UTF8>;
extern template class adv_string_view<UTF8>;
extern template class adv_string<UTF8>;

extern template class twrite_pt<UTF16SYS>;
extern template class tread_pt<UTF16SYS>;
extern template class adv_string_view<UTF16SYS>;
extern template class adv_string<UTF16SYS>;

extern template class twrite_pt<UTF32SYS>;
extern template class tread_pt<UTF32SYS>;
extern template class adv_string_view<UTF32SYS>;
extern template class adv_string<UTF32SYS>;


inline astr_view getstring(const char *c, size_t maxsiz){
	return adv_string_view<CENC>{c, maxsiz};
}

inline u8str_view getstring(const char8_t *c, size_t maxsiz){
	return adv_string_view<UTF8>{c, maxsiz};
}

inline u16str_view getstring(const char16_t *c, size_t maxsiz){
	return adv_string_view<UTF16SYS>{c, maxsiz};
}

inline u32str_view getstring(const char32_t *c, size_t maxsiz){
	return adv_string_view<UTF32SYS>{c, maxsiz};
}

/*
    Detect utf8, utf16 encoding from BOM, if haven't BOM then throw exception
*/
inline const EncMetric<unicode> *detect_bom(adv_string_view<RAW<unicode>> t){
	const_tchar_pt<RAW<unicode>> ptr = t.begin();
	if(t.size() < 2)
		throw encoding_error{"No BOM"};
	if(ptr[0] == byte{0xfe} && ptr[1] == byte{0xff})
		return DynEncoding<UTF16BE>::instance();
	if(ptr[0] == byte{0xff} && ptr[1] == byte{0xfe})
		return DynEncoding<UTF16LE>::instance();
	if(t.size() < 3)
		throw encoding_error{"No BOM"};
	if(ptr[0] == byte{0xef} && ptr[1] == byte{0xbb} && ptr[2] == byte{0xbf})
		return DynEncoding<UTF8>::instance();
	throw encoding_error{"No BOM"};

}

inline namespace literals{

inline const byte * operator""_raw(const char *c, std::size_t){
	return reinterpret_cast<const byte *>(c);
}

inline astr_view operator""_asv(const char *b, std::size_t st){
	return adv_string_view<CENC>{b, st};
}

inline adv_string_view<UTF8> operator""_asv(const char8_t *b, std::size_t st){
	return adv_string_view<UTF8>{b, st};
}

inline adv_string_view<UTF16SYS> operator""_asv(const char16_t *b, std::size_t st){
	return adv_string_view<UTF16SYS>{b, st * 2};
}

inline adv_string_view<UTF32SYS> operator""_asv(const char32_t *b, std::size_t st){
	return adv_string_view<UTF32SYS>{b, st * 4};
}

inline adv_string_view<WCENC> operator""_asv(const wchar_t *b, std::size_t st){
	return adv_string_view<WCENC>{reinterpret_cast<const byte *>(b), st * sizeof(wchar_t)};
}

}

}
