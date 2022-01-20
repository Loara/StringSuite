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
#include <strsuite/io/enc_io_core.hpp>
#include <strsuite/io/cio_stream.hpp>
#include <strsuite/io/tokens.hpp>

namespace sts{

//explicit declaration of template - for compilation improvment
using iochar_pt = tchar_pt<IOenc>;
using c_iochar_pt = const_tchar_pt<IOenc>;
using iostr_view = adv_string_view<IOenc>;
using iostr = adv_string<IOenc>;

/*
 * in order to define IOenc string literals you should use STS_IO_asv macro, for example
 * iostr_view v = STS_IO_asv("Hello")
 *initializes a system-aware "Hello" string
 */

}




