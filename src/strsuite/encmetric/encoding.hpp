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
    Encoding Metrics classes, with some useful base classes.

    Any implementation of an Encoding Metric must have a typedef 'ctype' represents the type of data you want to encode/decode
    (for character encodings you can use Unicode). Also it must have these function member declared as 
    static:

     - constexpr unsigned int min_bytes() noexcept  => minimum number of bytes needed to detect the length of a character
     - unsigned int chLen(const byte *, size_t)  => the length in bytes of the first character pointed by (can throw
        an encoding_error if the length can't be recognized). The first purpouse of this function
        is only to calculate le length of a character, not to verify it. The second character is the size of your byte array
     - validation_result validChar(const byte *, size_t) noexcept  => Test is the first character is valid with
        respect to this encoding. This function returns also the character length that can be retrived with get() function member.
     - std::tuple<unsigned int, T> decode(const byte *, size_t)  =>  returns the number of bytes read and the decoded character.
       If there aren't enough bytes it must throw buffer_small
     - unsigned int encode(const T &, byte *, size_t)  => encode the Unicode character and writes it in the memory pointed
        and returns the number of bytes written. If there isn't enough space it must throw buffer_small

        If the array size is too small in validChar function it must return false, in other functions a buffer_small exception must be thrown
        Notice also that chLen should throw a buffer_small exception if and only if it's not possible to detect the length of the ipothetical
        character, without testing if that character is entirely encoded. For this reason chLen mustn't throw anyway if all characters have the same length
*/
#include <typeindex>
#include <type_traits>
#include <cstring>
#include <concepts>
#include <tuple>
#include <strsuite/encmetric/base.hpp>
#include <strsuite/encmetric/exceptions.hpp>

namespace sts{

inline void copyN(const byte *src, byte *des, size_t l) {std::memcpy(des, src, l);}

template<typename ctype>
using tuple_ret = std::tuple<uint, ctype>;

/*
 * Use these instead of std::get in order to hol portability across different implementations
 */
template<typename ctype>
constexpr uint get_len_el(const tuple_ret<ctype> &tupl) noexcept{
    return std::get<0>(tupl);
}
template<typename ctype>
constexpr ctype get_chr_el(const tuple_ret<ctype> &tupl) noexcept{
    return std::get<1>(tupl);
}

/*
    Placeholder: the encoding is determined at runtime
*/
template<typename tt>
class WIDE{
	public:
		using ctype=tt;
};

using WIDEchr=WIDE<unicode>;

/*
    Use when you don't know actual encoding of a string. It assumes size == length and doesn't allow any decode/encode
*/
template<typename tt>
class RAW{
	public:
		using ctype=tt;
		static consteval uint min_bytes() noexcept {return 1;}
		static consteval uint max_bytes() {return 1;}
		static constexpr uint chLen(const byte *, size_t) {return 1;}
		static constexpr validation_result validChar(const byte *, size_t i) noexcept{
			return validation_result{i >= 1, 1};
		}
		static tuple_ret<tt> decode(const byte *, size_t) {throw raw_error{};}
		static uint encode(const tt &, byte *, size_t) {throw raw_error{};}
};

using RAWchr=RAW<unicode>;

/*
 * Concepts
 */

template<typename T>
concept strong_enctype = requires {typename T::ctype;} && requires(const byte * const a, byte * const b, const typename T::ctype &tu, const size_t sz){
        typename std::integral_constant<uint, T::min_bytes()>;
        {T::chLen(a, sz)}->std::convertible_to<uint>;
        {T::validChar(a, sz)}noexcept->std::same_as<validation_result>;
        {T::decode(a, sz)}->std::same_as<tuple_ret<typename T::ctype>>;
        {T::encode(tu, b, sz)}->std::convertible_to<uint>;
    };

template<typename U>
struct is_wide : public std::false_type {};
template<typename tt>
struct is_wide<WIDE<tt>> : public std::true_type {};

template<typename U>
struct is_raw : public std::false_type {};
template<typename tt>
struct is_raw<RAW<tt>> : public std::true_type {};

template<typename T>
concept widenc = is_wide<T>::value;
template<typename T>
concept not_widenc = strong_enctype<T> && !is_wide<T>::value;
template<typename S, typename T>
concept not_widenc_both = not_widenc<S> && not_widenc<T>;

template<typename T>
concept enc_raw = is_raw<T>::value;

template<typename T>
concept general_enctype = widenc<T> || strong_enctype<T>;

template<typename T, typename tt>
concept strong_enctype_of = strong_enctype<T> && std::same_as<typename T::ctype, tt>;

template<typename T, typename tt>
concept general_enctype_of = general_enctype<T> && std::same_as<typename T::ctype, tt>;

/*
    Test if types refer to the same encoding
*/

template<typename S, typename T>
concept same_enc = not_widenc<S> && not_widenc<T> && std::same_as<S, T>;

/*
 * Test if both encodings work on the same data type
 */

template<typename S, typename T>
concept same_data = general_enctype<S> && general_enctype<T> && (enc_raw<S> || enc_raw<T> || std::same_as<typename S::ctype, typename T::ctype>);

/*
    Removing alias mechanism, substituted with assigment rules.

    We say an encoding A is a BASE for B if and only if any A encoded string is also a B valid encoded string
    with the same meaning. A well-known application on unicode strings is when A is ASCII and B is UTF8, Latin1, ISO8859, Win codepages, ...

    In order to make A a BASE for B you need to include inside B an "enc_base" class typedef of A. Look Latin1 class declaration.

    You should not use aliases to RAW encodingd, and you mustn't create crossed aliases in order to make two encodings equivalent.
*/
template<typename T>
concept has_alias = strong_enctype<T> && requires {typename T::enc_base;} && strong_enctype_of<typename T::enc_base, typename T::ctype>;

template<typename T>
concept has_not_alias = strong_enctype<T> && !requires {typename T::enc_base;};

template<typename A, typename B>
struct base_ale_0;

template<strong_enctype A, has_not_alias B>
struct base_ale_0<A, B> : public std::bool_constant<std::same_as<A, B>> {};

template<strong_enctype A, has_alias B>
struct base_ale_0<A, B> : public std::bool_constant<std::same_as<A, B> || base_ale_0<A, typename B::enc_base>::value> {};

template<typename A, typename B>
concept is_base_for = strong_enctype<A> && strong_enctype<B> && std::same_as<typename A::ctype, typename B::ctype> && base_ale_0<A, B>::value;

template<typename A, typename B>
concept is_extension_for = is_base_for<B, A>;

template<strong_enctype T>
constexpr int min_length(int nchr) noexcept{
	return T::min_bytes() * nchr;
}

template<strong_enctype T>
constexpr void assert_raw(){static_assert(!enc_raw<T>, "Using RAW format");}

/*
 * Encoding optional features
 */
namespace feat{
    template<typename T>
    class has_max : public std::false_type{};

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

//---------------------------------------------------

/*
    Some basic encodings
*/
class ASCII{
	public:
		using ctype=unicode;
		static consteval uint min_bytes() noexcept {return 1;}
		static consteval uint max_bytes() noexcept {return 1;}
		static uint chLen(const byte *, size_t) {return 1;}
		static validation_result validChar(const byte *, size_t) noexcept;
		static tuple_ret<unicode> decode(const byte *by, size_t l);
		static uint encode(const unicode &uni, byte *by, size_t l);
};

class Latin1{
	public:
		using ctype=unicode;
        using enc_base=ASCII;
		static consteval uint min_bytes() noexcept {return 1;}
		static consteval uint max_bytes() noexcept {return 1;}
		static uint chLen(const byte *, size_t) {return 1;}
		static validation_result validChar(const byte *, size_t) noexcept;
		static tuple_ret<unicode> decode(const byte *by, size_t l);
		static uint encode(const unicode &uni, byte *by, size_t l);
};

}


