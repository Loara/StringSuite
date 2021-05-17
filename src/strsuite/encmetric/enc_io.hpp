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
#include <strsuite/encmetric/enc_string.hpp>
#include <strsuite/encmetric/all_enc.hpp>
#include <strsuite/encmetric/config.hpp>
#include <strsuite/encmetric/enc_io_core.hpp>
#include <type_traits>

namespace sts{

//System encoding for IO operations
using IOenc = std::conditional_t<is_windows(), UTF16<false>, UTF8>;

//explicit declaration of template - for compilation improvment
using iochar_pt = tchar_pt<IOenc>;
using c_iochar_pt = const_tchar_pt<IOenc>;
using iostr_view = adv_string_view<IOenc>;
using iostr = adv_string<IOenc>;

/*
 * Read/write up to n characters (not n bytes)
 *
 * stdin_getChrs_verify verify each character and throws an incorrect_encoding exception if a character is not valid
 */
size_t stdin_getChrs(iochar_pt, size_t);
size_t stdin_getChrs_validate(iochar_pt, size_t);
size_t stdout_putChrs(c_iochar_pt, size_t);
size_t stderr_putChrs(c_iochar_pt, size_t);
inline size_t stdout_putStr(iostr_view str) {return stdout_putChrs(str.begin(), str.length());}
inline size_t stderr_putStr(iostr_view str) {return stderr_putChrs(str.begin(), str.length());}

inline size_t stdout_endline(){return raw_newline();};

}




