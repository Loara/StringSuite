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

template class sts::tchar_pt<sts::ASCII>;
template class sts::const_tchar_pt<sts::ASCII>;
template class sts::adv_string_view<sts::ASCII>;
template class sts::adv_string<sts::ASCII>;

template class sts::tchar_pt<sts::UTF8>;
template class sts::const_tchar_pt<sts::UTF8>;
template class sts::adv_string_view<sts::UTF8>;
template class sts::adv_string<sts::UTF8>;

template class sts::tchar_pt<sts::UTF16SYS>;
template class sts::const_tchar_pt<sts::UTF16SYS>;
template class sts::adv_string_view<sts::UTF16SYS>;
template class sts::adv_string<sts::UTF16SYS>;

template class sts::tchar_pt<sts::UTF32SYS>;
template class sts::const_tchar_pt<sts::UTF32SYS>;
template class sts::adv_string_view<sts::UTF32SYS>;
template class sts::adv_string<sts::UTF32SYS>;
