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
#include <strsuite/format/format_exc.hpp>

namespace sts{

    struct Int_opts{
        uint base;
        bool plus, MAI;
        Int_opts() : base{10}, plus{false}, MAI{false} {}
        Int_opts(uint b) : base{b}, plus{false}, MAI{false} {}
        Int_opts(uint b, bool p, bool M) : base{b}, plus{p}, MAI{M} {}

        template<std::unsigned_integral I>
        constexpr unicode convert_unit(I i) const{
            if(base > 36)
                throw out_of_range{"Invalid base"};
            I j = i % base;
            if(j < 10)
                return static_cast<unicode>(j + '0');
            else if(MAI)
                return static_cast<unicode>((j - 10u) + 'A');
            else
                return static_cast<unicode>((j - 10u) + 'a');
        }
    };

    template<std::integral I>
    constexpr conditional_result<I> get_number(unicode c, uint base){
        if(base < 2 || base > 36)
            throw out_of_range{"Invalid base"};//Must throw in this case since is an option error
        I zer = static_cast<I>('0');
        I A = static_cast<I>('A');
        I a = static_cast<I>('a');
        size_t val = static_cast<size_t>(c);
        I ret;
        if(val >= zer && val < zer + 10u)
            ret = val - zer;
        else if(val >= A && val < A + 26u)
            ret = val - A + 10u;
        else if(val >= a && val < a + 26u)
            ret = val - a + 10u;
        else
            return conditional_result{false, I(0)};

        if(ret >= base)
            return conditional_result{false, I(0)};
        return conditional_result{true, ret};
    }

    template<typename Stream, std::integral I>
    void write_integer(Stream &out, I val, const Int_opts &opt);
    template<typename Stream, std::integral I>
    void write_integer(Stream &out, I val){
        write_integer(out, val, Int_opts{});
    }

    template<typename Stream, std::integral I>
    void read_integer(Stream &in, I &val, uint base =10);

    struct IntFormatter{
        template<general_enctype T, std::integral I>
        void format(string_stream<T> &str, I i, const adv_string_view<T> &){
            write_integer(str, i, Int_opts{});
        }
    };

#include <strsuite/format/integral_format.tpp>
}
