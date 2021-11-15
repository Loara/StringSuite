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
		virtual uint d_chLen(const byte *, size_t) const=0;
		virtual validation_result d_validChar(const byte *, size_t) const noexcept =0;
		virtual uint d_decode(ctype *, const byte *, size_t) const =0;
		virtual uint d_encode(const ctype &, byte *, size_t) const =0;

		virtual bool d_has_max() const noexcept=0;
		virtual uint d_max_bytes() const=0;

		virtual bool d_fixed_size() const noexcept =0;
		virtual std::type_index index() const noexcept=0;

        virtual bool d_has_head() const noexcept=0;
        virtual uint d_head() const=0;
        /*
         * If it doesn't have enc_base must return nullptr
         */
        virtual const EncMetric<tt> *d_enc_base() const noexcept =0;
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
		static consteval uint min_bytes() noexcept {return 1;}
		static consteval uint max_bytes() {return 1;}
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
concept strong_enctype = requires {typename T::ctype;} && requires(const byte * const a, byte * const b, typename T::ctype tu, const size_t sz){
        typename std::integral_constant<uint, T::min_bytes()>;
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

template<typename tt>
bool is_base_for_d(const EncMetric<tt> *a, const EncMetric<tt> *b) noexcept{
    if(a == nullptr || b == nullptr)
        return false;
    if(a->index() == b->index())
        return true;
    return is_base_for_d(a, b->d_enc_base());
}
template<typename tt>
bool is_extension_for_d(const EncMetric<tt> *a, const EncMetric<tt> *b) noexcept{
    return is_base_for_d(b, a);
}

template<strong_enctype T>
constexpr int min_length(int nchr) noexcept{
	return T::min_bytes() * nchr;
}
template<typename tt>
int min_length(int nchr, const EncMetric<tt> *format) noexcept{
	return format->d_min_bytes() * nchr;
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
}

template<strong_enctype T>
constexpr int max_length(uint nchr){
	static_assert(feat::has_max<T>::value, "This encoding has no superior limit");
	return T::max_bytes() * nchr;
}
template<typename tt>
int max_length(uint nchr, const EncMetric<tt> *format){
	if(format->d_has_max())
		return format->d_max_bytes() * nchr;
	else
		throw encoding_error{"This encoding has no superior limit"};
}

//---------------------------------------------------

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
		constexpr bool d_has_max() const noexcept {return feat::has_max<T>::value;}
		constexpr uint d_max_bytes() const {
            if constexpr(feat::has_max<T>::value)
                return feat::has_max<T>::get_max();
            else
                throw encoding_error{"This encoding has no superior limit"};
        }
		uint d_chLen(const byte *b, size_t siz) const {return static_enc::chLen(b, siz);}
		validation_result d_validChar(const byte *b, size_t siz) const noexcept {return static_enc::validChar(b, siz);}
		std::type_index index() const noexcept {return std::type_index{typeid(T)};}

		uint d_decode(ctype *uni, const byte *by, size_t l) const {return static_enc::decode(uni, by, l);}
		uint d_encode(const ctype &uni, byte *by, size_t l) const {return static_enc::encode(uni, by, l);}

		bool d_fixed_size() const noexcept {return feat::fixed_size<T>::value;}

        bool d_has_head() const noexcept{return feat::opt_head<T>::value;}
        uint d_head() const{
            if constexpr(feat::opt_head<T>::value)
                return feat::opt_head<T>::get_head();
            else
                throw encoding_error{"This encoding has no head/tail structure"};
        }

		static const EncMetric<ctype> *instance() noexcept{
			static DynEncoding<T> t{};
			return &t;
		}

        const EncMetric<ctype> *d_enc_base() const noexcept{
            if constexpr(has_alias<T>)
                return DynEncoding<typename T::enc_base>::instance();
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

		constexpr bool has_max() const noexcept {return feat::has_max<T>::value;}
		constexpr uint max_bytes() const noexcept requires feat::has_max<T>::value {return feat::has_max<T>::get_max();}
		/*
         * Use only for runtime erroring
         */
		uint max_bytes() const requires (!feat::has_max<T>::value) {throw encoding_error{"This encoding has no superior limit"};}

		constexpr bool has_head() const noexcept {return feat::opt_head<T>::value;}
		constexpr uint head() const noexcept requires feat::opt_head<T>::value {return feat::opt_head<T>::get_head();}
		uint head() const requires (!feat::opt_head<T>::value) {throw encoding_error{"This encoding has no head/tail structure"};}

		constexpr bool is_fixed() const noexcept {return feat::fixed_size<T>::value;}

		uint chLen(const byte *b, size_t siz) const {return T::chLen(b, siz);}
		validation_result validChar(const byte *b, size_t l) const noexcept {return T::validChar(b, l);}
		uint decode(ctype *uni, const byte *by, size_t l) const {return T::decode(uni, by, l);}
		uint encode(const ctype &uni, byte *by, size_t l) const {return T::encode(uni, by, l);}
		std::type_index index() const noexcept {return DynEncoding<T>::index();}

		template<general_enctype S>
		constexpr bool equalTo(EncMetric_info<S>) const noexcept requires not_widenc<S>{
            return same_enc<S, T>;
        }
		template<general_enctype S>
		bool equalTo(EncMetric_info<S> o) const noexcept requires widenc<S>{
            return index() == o.index();
        }

        template<general_enctype S>
        void assert_same_enc([[maybe_unused]] EncMetric_info<S> o) const{
            if constexpr(strong_enctype<S>){
                static_assert(same_enc<T, S>, "Different encodings");
            }
            else{
                if(index() != o.index())
                    throw incorrect_encoding{"Different encodings"};
            }
        }

        template<general_enctype S> requires same_data<T, S> && not_widenc<S>
        constexpr bool base_for(EncMetric_info<S>) const noexcept{
            return is_base_for<T, S>;
        }
        template<general_enctype S> requires same_data<T, S> && widenc<S>
        bool base_for(EncMetric_info<S> b) const noexcept{
            return is_base_for_d(format(), b.format());
        }

        template<general_enctype S>
        void assert_base_for([[maybe_unused]] EncMetric_info<S> b) const{
            static_assert(same_data<T, S>, "Cannot convert these encodings");
            if constexpr(strong_enctype<S>){
                static_assert(is_base_for<T, S>, "Cannot convert these encodings");
            }
            else{
                if(!is_base_for_d(format(), b.format()))
                    throw incorrect_encoding{"Cannot convert these encodings"};
            }
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
		uint max_bytes() const {return f->d_max_bytes();}

		bool is_fixed() const noexcept {return f->d_fixed_size();}

		bool has_head() const noexcept {return f->d_has_head();}
		uint head() const {return f->d_head();}

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
};

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
		static uint decode(unicode *uni, const byte *by, size_t l);
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
		static uint decode(unicode *uni, const byte *by, size_t l);
		static uint encode(const unicode &uni, byte *by, size_t l);
};

}


