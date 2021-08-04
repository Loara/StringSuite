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
     - constexpr bool has_max() noexcept => the encoding fixes the maximum number of bytes per-character
     - constexpr unsigned int max_bytes() noexcept => maximum number of bytes needed to store an entire character, undefined
       if has_max is false
     - unsigned int chLen(const byte *, size_t)  => the length in bytes of the first character pointed by (can throw
        an encoding_error if the length can't be recognized). The first purpouse of this function
        is only to calculate le length of a character, not to verify it. The second character is the size of your byte array
     - validation_result validChar(const byte *, size_t) noexcept  => Test is the first character is valid with
        respect to this encoding. This function returns also the character length that can be retrived with get() function member.
     - unsigned int decode(T *, const byte *, size_t)  => sets the Unicode code of the first encoded characters
        and returns the number of bytes read. If there aren't enough bytes it must throw buffer_small
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
#include <strsuite/encmetric/base.hpp>
#include <strsuite/encmetric/exceptions.hpp>

namespace sts{

inline void copyN(const byte *src, byte *des, size_t l) {std::memcpy(des, src, l);}

template<typename tt>
class EncMetric{
	public:
		using ctype=tt;
		virtual ~EncMetric() {}
		virtual uint d_min_bytes() const noexcept=0;
		virtual bool d_has_max() const noexcept=0;
		virtual uint d_max_bytes() const=0;
		virtual uint d_chLen(const byte *, size_t) const=0;
		virtual validation_result d_validChar(const byte *, size_t) const noexcept =0;
		virtual uint d_decode(ctype *, const byte *, size_t) const =0;
		virtual uint d_encode(const ctype &, byte *, size_t) const =0;

		virtual bool d_fixed_size() const noexcept =0;
		virtual std::type_index index() const noexcept=0;
        /*
         * If it doesn't have alias must return nullptr
         */
        virtual const EncMetric<tt> *d_alias() const noexcept =0;
};

/*
    Placeholder: the encoding is determined at runtime
*/
template<typename tt>
class WIDE{
	public:
		using ctype=tt;
};

using WIDEchr=WIDE<unicode>;

struct RAWcmp{};

/*
    No encoding provided
*/
template<typename tt>
class RAW{
	public:
		using ctype=tt;
		static constexpr uint min_bytes() noexcept {return 1;}
		static constexpr bool has_max() noexcept {return true;}
		static constexpr uint max_bytes() {return 1;}
		using compare_enc=RAWcmp;
		static uint chLen(const byte *, size_t) {return 1;}
		static validation_result validChar(const byte *, size_t i) noexcept{
			return validation_result{i >= 1, 1};
		}
		static uint decode(tt *, const byte *, size_t) {throw raw_error{};}
		static uint encode(const tt &, byte *, size_t) {throw raw_error{};}
};

using RAWchr=RAW<unicode>;

/*
 * Concepts
 */

template<typename T>
concept strong_enctype = requires {typename T::ctype;} && requires(const byte *a, byte *b, typename T::ctype tu, size_t sz){
        {T::min_bytes()} noexcept->std::convertible_to<uint>;
        {T::has_max()} noexcept->std::same_as<bool>;
        {T::max_bytes()}->std::convertible_to<uint>;
        {T::chLen(a, sz)}->std::convertible_to<uint>;
        {T::validChar(a, sz)}noexcept->std::same_as<validation_result>;
        {T::decode(&tu, a, sz)}->std::convertible_to<uint>;
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

    In order to make A a BASE for B you need to include inside B an "alias" class typedef of A. Look Latin1 class declaration.

    You should not use aliases to RAW encodingd, and you mustn't create crossed aliases in order to make two encodings equivalent.
*/
template<typename T>
concept has_alias = strong_enctype<T> && requires {typename T::alias;} && strong_enctype_of<typename T::alias, typename T::ctype>;

template<typename T>
concept has_not_alias = strong_enctype<T> && !requires {typename T::alias;};

template<typename A, typename B>
struct base_ale_0;

template<strong_enctype A, has_not_alias B>
struct base_ale_0<A, B> : public std::bool_constant<std::same_as<A, B>> {};

template<strong_enctype A, has_alias B>
struct base_ale_0<A, B> : public std::bool_constant<std::same_as<A, B> || base_ale_0<A, typename B::alias>::value> {};

template<typename A, typename B>
concept is_base_for = strong_enctype<A> && strong_enctype<B> && std::same_as<typename A::ctype, typename B::ctype> && base_ale_0<A, B>::value;

template<typename tt>
bool is_base_for_d(const EncMetric<tt> *a, const EncMetric<tt> *b) noexcept{
    if(a == nullptr || b == nullptr)
        return false;
    if(a->index() == b->index())
        return true;
    return is_base_for_d(a, b->d_alias());
}

template<typename T>
concept safe_hasmax = not_widenc<T> && T::has_max();
template<typename T>
concept safe_not_hasmax = general_enctype<T> && !safe_hasmax<T>;

template<typename T>
concept fixed_size = not_widenc<T> && T::has_max() && (T::min_bytes() == T::max_bytes());
template<typename T>
concept not_fixed_size = general_enctype<T> && !fixed_size<T>;


template<strong_enctype T>
constexpr int min_length(int nchr) noexcept{
	return T::min_bytes() * nchr;
}
template<typename tt>
int min_length(int nchr, const EncMetric<tt> *format) noexcept{
	return format->d_min_bytes() * nchr;
}

template<strong_enctype T>
constexpr int max_length(uint nchr){
	static_assert(safe_hasmax<T>, "This encoding has no superior limit");
	return T::max_bytes() * nchr;
}
template<typename tt>
int max_length(uint nchr, const EncMetric<tt> *format){
	if(format->d_has_max())
		return format->d_max_bytes() * nchr;
	else
		throw encoding_error{"This encoding has no superior limit"};
}

template<strong_enctype T>
constexpr void assert_raw(){static_assert(!enc_raw<T>, "Using RAW format");}

/*
    Wrapper of an encoding T in order to save it in a class field of WIDENC classes

    WARNING: T will be never RAW
*/
template<strong_enctype T>
class DynEncoding : public EncMetric<typename T::ctype>{
	private:		
		DynEncoding() noexcept {}
	public:
        static_assert(!enc_raw<T>, "EncMetric cannot hold a RAW encoding");
		using static_enc = T;
        using ctype = typename T::ctype;

		~DynEncoding() {}

		constexpr uint d_min_bytes() const noexcept {return static_enc::min_bytes();}
		constexpr bool d_has_max() const noexcept {return static_enc::has_max();}
		constexpr uint d_max_bytes() const {return static_enc::max_bytes();}
		uint d_chLen(const byte *b, size_t siz) const {return static_enc::chLen(b, siz);}
		validation_result d_validChar(const byte *b, size_t siz) const noexcept {return static_enc::validChar(b, siz);}
		std::type_index index() const noexcept {return std::type_index{typeid(T)};}

		uint d_decode(ctype *uni, const byte *by, size_t l) const {return static_enc::decode(uni, by, l);}
		uint d_encode(const ctype &uni, byte *by, size_t l) const {return static_enc::encode(uni, by, l);}

		bool d_fixed_size() const noexcept {return fixed_size<T>;}

		static const EncMetric<ctype> *instance() noexcept{
			static DynEncoding<T> t{};
			return &t;
		}

        const EncMetric<ctype> *d_alias() const noexcept{
            if constexpr(has_alias<T>)
                return DynEncoding<typename T::alias>::instance();
            else
                return nullptr;
        }
};

/*
    Store information about used encoding
*/
template<general_enctype T>
class EncMetric_info{
	public:
		using ctype=typename T::ctype;
		constexpr EncMetric_info(const EncMetric_info<T> &) noexcept {}
		constexpr EncMetric_info() noexcept {}
		const EncMetric<ctype> *format() const noexcept {return DynEncoding<T>::instance();}

		constexpr uint min_bytes() const noexcept {return T::min_bytes();}
		constexpr bool has_max() const noexcept {return T::has_max();}
		constexpr uint max_bytes() const noexcept {return T::max_bytes();}
		constexpr bool is_fixed() const noexcept {return fixed_size<T>;}

		uint chLen(const byte *b, size_t siz) const {return T::chLen(b, siz);}
		validation_result validChar(const byte *b, size_t l) const noexcept {return T::validChar(b, l);}
		uint decode(ctype *uni, const byte *by, size_t l) const {return T::decode(uni, by, l);}
		uint encode(const ctype &uni, byte *by, size_t l) const {return T::encode(uni, by, l);}
		std::type_index index() const noexcept {return DynEncoding<T>::index();}

		template<general_enctype S>
		constexpr bool equalTo(EncMetric_info<S> o) const noexcept requires not_widenc<S>{
            if constexpr(not_widenc<S>)
                return same_enc<S, T>;
            else
                return index() == o.index();
        }
        template<general_enctype S>
        void assert_same_enc(EncMetric_info<S> o) const{
            if constexpr(strong_enctype<S>){
                static_assert(same_enc<T, S>, "Different encodings");
            }
            else{
                if(index() != o.index())
                    throw incorrect_encoding{"Different encodings"};
            }
        }

        template<general_enctype S> requires same_data<T, S> && not_widenc<S>
        constexpr bool base_for(EncMetric_info<S> b) const noexcept{
            if constexpr(not_widenc<S>)
                return is_base_for<T, S>;
            else
                return is_base_for_d(format(), b.format());
        }
        template<general_enctype S>
        void assert_base_for(EncMetric_info<S> b) const{
            if constexpr(!same_data<T, S>)
                throw incorrect_encoding{"Cannot convert these encodings"};
            else if constexpr(strong_enctype<S>){
                if constexpr(!is_base_for<T, S>)
                    throw incorrect_encoding{"Cannot convert these encodings"};
            }
            else{
                if(!is_base_for_d(format(), b.format()))
                    throw incorrect_encoding{"Cannot convert these encodings"};
            }
        }
        template<general_enctype S> requires same_data<T, S>
        bool stc_can_reassign() const noexcept{
            if constexpr(widenc<S>)
                return true;
            else
                return base_for(EncMetric_info<S>{});
        }
};

template<typename tt>
class EncMetric_info<WIDE<tt>>{
	private:
		const EncMetric<tt> *f;
	public:
		using ctype=tt;
		constexpr EncMetric_info(const EncMetric<tt> *format) noexcept : f{format} {}
		constexpr EncMetric_info(const EncMetric_info &info) noexcept : f{info.f} {}
		constexpr const EncMetric<tt> *format() const noexcept {return f;}

		uint min_bytes() const noexcept {return f->d_min_bytes();}
		bool has_max() const noexcept {return f->d_has_max();}
		uint max_bytes() const noexcept {return f->d_max_bytes();}
		bool is_fixed() const noexcept {return f->d_fixed_size();}

		uint chLen(const byte *b, size_t siz) const {return f->d_chLen(b, siz);}
		validation_result validChar(const byte *b, size_t l) const noexcept {return f->d_validChar(b, l);}
		uint decode(ctype *uni, const byte *by, size_t l) const {return f->d_decode(uni, by, l);}
		uint encode(const ctype &uni, byte *by, size_t l) const {return f->d_encode(uni, by, l);}
		std::type_index index() const noexcept {return f->index();}

		template<general_enctype S>
		bool equalTo(EncMetric_info<S> o) const noexcept{
            return index() == o.index();
        }
        template<general_enctype S>
        void assert_same_enc(EncMetric_info<S> o) const{
            if(index() != o.index())
                throw incorrect_encoding{"Different encodings"};
        }
        template<typename S> requires general_enctype_of<S, tt>
        bool base_for(EncMetric_info<S> b) const noexcept{
            return is_base_for_d(format(), b.format());
        }
        template<general_enctype S>
        void assert_base_for(EncMetric_info<S> b) const{
            if constexpr(!general_enctype_of<S, tt>)
                throw incorrect_encoding{"Cannot convert these encodings"};
            else if(!is_base_for_d(format(), b.format()))
                throw incorrect_encoding{"Cannot convert these encodings"};
        }
        template<typename S> requires general_enctype_of<S, tt>
        bool stc_can_reassign() const noexcept{
            if constexpr(widenc<S>)
                return true;
            else
                return base_for(EncMetric_info<S>{});
        }
};

/*
    Some basic encodings
*/
class ASCII{
	public:
		using ctype=unicode;
		static constexpr uint min_bytes() noexcept {return 1;}
		static constexpr bool has_max() noexcept {return true;}
		static constexpr uint max_bytes() noexcept {return 1;}
		static uint chLen(const byte *, size_t) {return 1;}
		static validation_result validChar(const byte *, size_t) noexcept;
		static uint decode(unicode *uni, const byte *by, size_t l);
		static uint encode(const unicode &uni, byte *by, size_t l);
};

class Latin1{
	public:
		using ctype=unicode;
        using alias=ASCII;
		static constexpr uint min_bytes() noexcept {return 1;}
		static constexpr bool has_max() noexcept {return true;}
		static constexpr uint max_bytes() noexcept {return 1;}
		static uint chLen(const byte *, size_t) {return 1;}
		static validation_result validChar(const byte *, size_t) noexcept;
		static uint decode(unicode *uni, const byte *by, size_t l);
		static uint encode(const unicode &uni, byte *by, size_t l);
};

}


