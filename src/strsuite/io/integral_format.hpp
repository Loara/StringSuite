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
#include <strsuite/io/string_stream.hpp>
namespace sts{

    struct Int_opts{
        uint base;
        bool plus;
        Int_opts() : base{10}, plus{false} {}
    };

    template<safe_hasmax T, std::integral I>
    void write_integer(CharOStream<T> &, I, const Int_opts &, std::pmr::memory_resource * =std::pmr::get_default_resource());

#include <strsuite/io/integral_format.tpp>
}
