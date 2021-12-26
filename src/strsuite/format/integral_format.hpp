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
#include <array>
#include <strsuite/encmetric/base.hpp>
#include <strsuite/io/string_stream.hpp>

namespace sts{

    struct Standard_number_chars{
        static constexpr size_t max_base = 36;
        static constexpr std::array<unicode, max_base> all_chars{'0'_uni, '1'_uni, '2'_uni, '3'_uni, '4'_uni, '5'_uni, '6'_uni, '7'_uni, '8'_uni, '9'_uni, 'a'_uni, 'b'_uni, 'c'_uni, 'd'_uni, 'e'_uni, 'f'_uni, 'g'_uni, 'h'_uni, 'i'_uni, 'j'_uni, 'k'_uni, 'l'_uni, 'm'_uni, 'n'_uni, 'o'_uni, 'p'_uni, 'q'_uni, 'r'_uni, 's'_uni, 't'_uni, 'u'_uni, 'v'_uni, 'w'_uni, 'x'_uni, 'y'_uni, 'z'_uni};
    };

    struct Int_opts{
        uint base;
        bool plus;
        Int_opts() : base{10}, plus{false} {}
        Int_opts(uint b) : base{b}, plus{false} {}
        Int_opts(uint b, bool p) : base{b}, plus{p} {}

        template<std::unsigned_integral I, typename T = Standard_number_chars>
        constexpr unicode convert_unit(I i) const{
            I j = i % base;
            return T::all_chars.at(j);
        }

        template<std::unsigned_integral I, typename T = Standard_number_chars>
        constexpr I get_number(unicode c) const{
            size_t dec = T::max_base;
            for(size_t i=0; i<T::max_base; i++){
                if(c == T::all_chars[i]){
                    dec = i;
                    break;
                }
            }
            if(dec >= base)
                throw out_of_range{"Invalid number letter"};
            return dec;
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
