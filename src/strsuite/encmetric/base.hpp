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
#include <utility>

namespace sts{
using std::size_t;
enum unicode : std::uint_fast32_t {};

inline unicode read_unicode(byte b){
	return unicode{static_cast<std::uint_least32_t>(b)};
}

inline constexpr unicode BOM{0xFEFF};

inline namespace literals{
inline constexpr unicode operator"" _uni(char32_t chr) noexcept{
    return unicode{chr};
}
/*
 * Only ASCII
 */
inline constexpr unicode operator"" _uni(char chr) noexcept{
    return chr >= 0 ?  static_cast<unicode>(chr) : static_cast<unicode>(-chr);
}
}

template<typename T>
struct conditional_result{
    T data;
    bool success;
    conditional_result(bool s, const T &d) : data{d}, success{s} {}
    explicit operator bool() const noexcept{ return success;}
    T get() const{ return data;}
};

using index_result=conditional_result<size_t>;
using validation_result=conditional_result<uint>;

struct dimensions{
    size_t len;
    size_t siz;
    dimensions() : len{0}, siz{0} {}
};

template<std::unsigned_integral S, std::unsigned_integral T>
inline constexpr bool no_overflow_sum(S s, T t)noexcept{
    return (s + t) >= s; //only one
}

/*
 * std::index_sequence helper functions
 */
 template<typename T, size_t N>
 struct is_index_h : public std::false_type{};

 template<size_t... I, size_t N>
 struct is_index_h<std::index_sequence<I...>, N> : public std::bool_constant<N == sizeof...(I)> {};

 template<typename T, size_t N>
 concept is_index_seq_of_len = is_index_h<T, N>::value;

 template<typename> struct push_value;

 template<size_t... prq>
 struct push_value<std::index_sequence<prq...>>{
     template<size_t N>
     using apply_f = std::index_sequence<N, prq...>;
     template<size_t N>
     using apply_b = std::index_sequence<prq..., N>;
 };

 template<size_t N>
 struct generate_rev{
     using apply = push_value<typename generate_rev<N-1>::apply>:: template apply_f<N-1>;
 };

 template<>
 struct generate_rev<0>{
     using apply = std::index_sequence<>;
 };

 template<size_t N>
 using make_rev_index_sequence = generate_rev<N>::apply;

}

