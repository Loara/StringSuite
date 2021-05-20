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

/*
    Base definitions
*/
#include <concepts>
#include <cstdint>
#include <strsuite/encmetric/byte_tools.hpp>
#include <type_traits>

namespace sts{
using std::size_t;
enum unicode : std::uint_least32_t {};

inline unicode read_unicode(byte b){
	return unicode{static_cast<std::uint_least32_t>(b)};
}

inline constexpr unicode BOM{0xFEFF};

template<typename T>
struct conditional_result{
    bool success;
    T data;
    explicit operator bool() const noexcept{ return success;}
    T get() const{ return data;}
};

using index_result=conditional_result<size_t>;
using validation_result=conditional_result<uint>;

template<std::unsigned_integral S, std::unsigned_integral T>
inline constexpr bool no_overflow_sum(S s, T t)noexcept{
    return (s + t) >= s; //only one
}

}

