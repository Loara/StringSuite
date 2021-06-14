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
#include <strsuite/encmetric/dynstring.hpp>
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
/*
 * Default wchar_t encoding
 */
using WCENC = std::conditional_t<is_windows(), UTF16LE, UTF32SYS>;

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

}
