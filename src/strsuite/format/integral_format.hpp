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
#include <concepts>
#include <forward_list>
#include <strsuite/encmetric/base.hpp>

namespace sts{

    struct Int_opts{
        uint base;
        bool plus;
        Int_opts() : base{10}, plus{false} {}
        Int_opts(uint b) : base{b}, plus{false} {}
        Int_opts(uint b, bool p) : base{b}, plus{p} {}

        template<std::unsigned_integral I>
        constexpr unicode convert_unit(I i) const noexcept{
            I j = i % base;
            if(j < 10)
                return unicode{0x30u + j};
            else
                return unicode{0x61u + (j - 10u)};
        }
    };

    template<typename Stream, std::integral I>
    void write_integer(Stream &out, I val, const Int_opts &opt);

    struct IntFormatter{
        template<general_enctype T, std::integral I>
        void format(string_stream<T> &str, I i, const adv_string_view<T> &){
            write_integer(str, i, Int_opts{});
        }
    };

#include <strsuite/format/integral_format.tpp>
}
