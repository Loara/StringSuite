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

#include <strsuite/encmetric/encoding.hpp>

namespace sts{

/*
 * Encoding optional features
 */
namespace feat{
    template<typename T>
    class has_max : public std::false_type{

    };

    template<typename T> requires requires(){typename std::integral_constant<uint, T::max_bytes()>;}
    class has_max<T> : public std::true_type{
    public:
        static constexpr uint get_max() noexcept{
            return T::max_bytes();
        }
    };

    template<typename T>
    concept safe_hasmax = strong_enctype<T> && has_max<T>::value;

    template<typename T>
    class fixed_size : public std::false_type{};

    template<typename T> requires has_max<T>::value
    class fixed_size<T> : public std::bool_constant<T::min_bytes() == T::max_bytes()> {};

    /*
     * Decoding object without retrieving also its size. Redefine only if you can provide a more optimized algorithm instead of usual decoding one
     */

    template<typename Enc>
    struct decode_direct_test : public std::false_type{
        static_assert(strong_enctype<Enc>, "Not an encoding class");
        using ctype = typename Enc::ctype;

        static ctype decode(const byte *b, size_t l){
            return get_chr_el(Enc::decode(b, l));
        }
    };

    template<typename Enc> requires strong_enctype<Enc> && requires(const byte * const b, const size_t s){
        {Enc::decode_direct(b, s)}->std::same_as<typename Enc::ctype> ;}
    struct decode_direct_test<Enc> : public std::true_type{
        using ctype = typename Enc::ctype;

        static ctype decode(const byte *b, size_t l){
            return Enc::decode_direct(b, l);
        }
    };

    /*
     * An encoding is opt_find if and only if there exists N>0 integer such that every encoded character can be divided in two consecutive regions:
     *  - a region A of length exactly N bytes
     *  - a region B of length N * k bytes with k>=0 integer
     * and every possible subsequence of N bytes in B must be different from every possible values of A
     *
     * This property allows easier finding algorithm. UTF8 and UTF16 both posseses this property with all fixed size encodings.
     */
    template<typename T>
    class opt_head : public std::false_type{};

    template<typename T> requires fixed_size<T>::value || requires(){typename std::integral_constant<uint, T::fixed_head()>;}
    class opt_head<T> : public std::true_type{
    public:
        static constexpr uint get_head() noexcept{
            if constexpr(fixed_size<T>::value)
                return T::min_bytes();
            else
                return T::fixed_head();
        }
    };

    /*
     * Proxy decoding
     * If the original bytestream is not going to be deallocated then you would sometimes not decode completely
     * a type but instead only do a "light decoding" by using a pointer to the encoded data
     *
     * This is useful if ctype object should use dynamically allocated memory that can be avoided if the original stream will not be released
     */

    template<typename T>
    concept has_proxy_type = strong_enctype<T> && requires (const typename T::proxy_ctype &re, byte * const buff, const size_t siz){
        {T::encode(re, buff, siz)}->std::convertible_to<uint>;
    };

    template<typename T>
    concept has_not_proxy_type = strong_enctype<T> && !requires(){typename T::proxy_ctype;};

    template<typename T>
    struct Proxy_wrapper{
        static_assert(strong_enctype<T>, "Not a encoding type");

        using proxy_ctype = typename T::ctype;
		static tuple_ret<proxy_ctype> light_decode(const byte *by, size_t l){
            return T::decode(by, l);
        }
    };

    template<typename T> requires has_proxy_type<T>
    struct Proxy_wrapper<T>{
        using proxy_ctype = typename T::proxy_ctype;

		static tuple_ret<proxy_ctype> light_decode( const byte *by, size_t l)
        {
            return T::light_decode(by, l);
        }
    };

    template<typename T>
    using Proxy_wrapper_ctype = typename Proxy_wrapper<T>::proxy_ctype;
}



template<strong_enctype T>
constexpr int max_length(uint nchr){
	static_assert(feat::has_max<T>::value, "This encoding has no superior limit");
	return T::max_bytes() * nchr;
}

}
