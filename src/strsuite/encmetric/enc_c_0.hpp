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
#include <strsuite/encmetric/stringbuild.hpp>
#include <strsuite/encmetric/all_enc.hpp>
#include <strsuite/encmetric/config.hpp>
#include <type_traits>

namespace sts{

/*
 * const char * string should always contain only ASCII characters
 * if you want to use UTF8 characters use instead u8 string literals
 */
using CENC = ASCII;
using UTF16SYS = UTF16<bend>;
using UTF32SYS = UTF32<bend>;

//better names
using achar_pt = tchar_pt<CENC>;
using c_achar_pt = const_tchar_pt<CENC>;
using astr_view = adv_string_view<CENC>;
using astr = adv_string<CENC>;

using u8char_pt = tchar_pt<UTF8>;
using c_u8char_pt = const_tchar_pt<UTF8>;
using u8str_view = adv_string_view<UTF8>;
using u8str = adv_string<UTF8>;

using u16char_pt = tchar_pt<UTF16SYS>;
using c_u16char_pt = const_tchar_pt<UTF16SYS>;
using u16str_view = adv_string_view<UTF16SYS>;
using u16str = adv_string<UTF16SYS>;

using u32char_pt = tchar_pt<UTF32SYS>;
using c_u32char_pt = const_tchar_pt<UTF32SYS>;
using u32str_view = adv_string_view<UTF32SYS>;
using u32str = adv_string<UTF32SYS>;

inline astr_view getstring(const char *c, size_t maxsiz){
	return adv_string_view<CENC>{c, maxsiz};
}

inline adv_string_view<UTF8> getstring(const char8_t *c, size_t maxsiz){
	return adv_string_view<UTF8>{c, maxsiz};
}

inline adv_string_view<UTF16<bend>> getstring(const char16_t *c, size_t maxsiz){
	return adv_string_view<UTF16<bend>>{c, maxsiz};
}

inline adv_string_view<UTF32<bend>> getstring(const char32_t *c, size_t maxsiz){
	return adv_string_view<UTF32<bend>>{c, maxsiz};
}

/*
    Detect utf8, utf16 encoding from BOM, if haven't BOM then throw exception
*/
inline const EncMetric<unicode> *detect_bom(adv_string_view<RAW<unicode>> t){
	const_tchar_pt<RAW<unicode>> ptr = t.begin();
	if(t.size() < 2)
		throw encoding_error{"No BOM"};
	if(ptr[0] == byte{0xfe} && ptr[1] == byte{0xff})
		return DynEncoding<UTF16<true>>::instance();
	if(ptr[0] == byte{0xff} && ptr[1] == byte{0xfe})
		return DynEncoding<UTF16<false>>::instance();
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

}

}
