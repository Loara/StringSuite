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

template<typename T>
concept widenc = is_wide<T>::value;
template<typename T>
concept not_widenc = strong_enctype<T> && !is_wide<T>::value;

template<typename T>
concept general_enctype = widenc<T> || strong_enctype<T>;

/*
    index_traits control if encoding class ovverides the index
*/
template<typename T>
concept default_aliases = not_widenc<T> && !requires {typename T::compare_enc;} && !requires {typename T::equivalent_enc;};
template<typename T>
concept compare_aliases = not_widenc<T> && requires {typename T::compare_enc;};
template<typename T>
concept equivalence_aliases = not_widenc<T> && !requires {typename T::compare_enc;} && requires {typename T::equivalent_enc;};

template<typename T>
struct index_traits_0;

template<default_aliases T>
struct index_traits_0<T>{
	using type_enc=T;
};

template<compare_aliases T>
struct index_traits_0<T>{
    static_assert(!is_wide<typename T::compare_enc>::value, "Cannot define aliases to WIDE encodings");
	using type_enc=typename T::compare_enc;
};

template<equivalence_aliases T>
struct index_traits_0<T>{
    static_assert(not_widenc<typename T::equivalence_enc>, "Cannot define aliases to WIDE encodings");
	using type_enc=typename index_traits_0<typename T::equivalence_enc>::type_enc;
};

template<not_widenc T>
struct index_traits : public index_traits_0<T> {
	static std::type_index index() noexcept {return std::type_index{typeid(typename index_traits_0<T>::type_enc)};}
};

/*
    Test if types refer to the same encoding
*/

template<typename S, typename T>
concept same_enc = not_widenc<S> && not_widenc<T> && std::same_as<typename index_traits<S>::type_enc, typename index_traits<T>::type_enc>;

template<typename T>
concept enc_raw = same_enc<T, RAW<byte>>;

/*
 * Test if both encodings work on the same data type
 */

template<typename S, typename T>
concept same_data = general_enctype<S> && general_enctype<T> && (enc_raw<S> || enc_raw<T> || std::same_as<typename S::ctype, typename T::ctype>);

template<typename T>
concept safe_hasmax = not_widenc<T> && T::has_max();

template<typename T>
concept fixed_size = not_widenc<T> && T::has_max() && (T::min_bytes() == T::max_bytes());

/*
 * Tests if a pointer with encoding S can be assigned to a pointer with encoding T
 *
 * Notice in particular their behavior when:
 * 1 - T is raw
 * Both returns true automatically, since a RAW encoding means undefined encoding, so can accept any encoding class also with a different ctype. Clearly
 * a RAW encoding cannot be easily converted to another encoding
 * 2 - T is wide
 * Both returns true when T and S have the same ctype and false otherwise: in fact when T=WIDE<tt> the dynamically defined encoding is taken from S (that can
 * be both wide or not) so the only limitation is due to their underlying ctype
 * 3 - S is wide
 * in this case weak_assign returns true if S has the same ctype of T, whereas strong_assign returns true only when S is equal to T (in particuar have same
 * ctype) or T is raw.
 * This different behaviour is due to different goals: weak_assign will only test statically the types before performing runtime checks; instead strong_assign
 * have to determine at compile-time if a string can be reassigned to a new class encoding.
 */
template<typename S, typename T>
concept weak_assign = enc_raw<T> || (same_data<S, T> && (widenc<S> || widenc<T> || same_enc<S, T>));

template<typename S, typename T>
concept strong_assign = enc_raw<T> || (same_data<S, T> && (widenc<T> || same_enc<S, T>));


template<strong_enctype T>
constexpr int min_length(int nchr) noexcept{
	return T::min_bytes() * nchr;
}
template<typename tt>
int min_length(int nchr, const EncMetric<tt> &format) noexcept{
	return format.d_min_bytes() * nchr;
}

template<strong_enctype T>
constexpr int max_length(uint nchr){
	static_assert(safe_hasmax<T>, "This encoding has no superior limit");
	return T::max_bytes() * nchr;
}
template<typename tt>
int max_length(uint nchr, const EncMetric<tt> &format){
	if(format.d_has_max())
		return format.d_max_bytes() * nchr;
	else
		throw encoding_error{"This encoding has no superior limit"};
}

template<strong_enctype T>
constexpr void assert_raw(){static_assert(!enc_raw<T>, "Using RAW format");}

template<typename tt>
void assert_raw(const EncMetric<tt> &f){
	if(f.index() == index_traits<RAW<tt>>::index())
		throw encoding_error("Using RAW format");
}

/*
    Wrapper of an encoding T in order to save it in a class field of WIDENC classes
*/
template<strong_enctype T>
class DynEncoding : public EncMetric<typename T::ctype>{
	private:		
		DynEncoding() {}
	public:
		using static_enc = T;

		~DynEncoding() {}

		uint d_min_bytes() const noexcept {return static_enc::min_bytes();}
		bool d_has_max() const noexcept {return static_enc::has_max();}
		uint d_max_bytes() const {return static_enc::max_bytes();}
		uint d_chLen(const byte *b, size_t siz) const {return static_enc::chLen(b, siz);}
		validation_result d_validChar(const byte *b, size_t siz) const noexcept {return static_enc::validChar(b, siz);}
		std::type_index index() const noexcept {return index_traits<T>::index();}

		uint d_decode(typename T::ctype *uni, const byte *by, size_t l) const {return static_enc::decode(uni, by, l);}
		uint d_encode(const typename T::ctype &uni, byte *by, size_t l) const {return static_enc::encode(uni, by, l);}

		bool d_fixed_size() const noexcept {return fixed_size<T>;}

		static const EncMetric<typename T::ctype> *instance() noexcept{
			static DynEncoding<T> t{};
			return &t;
		}
};

/*
    Store information about used encoding
*/
template<general_enctype T>
class EncMetric_info{
	public:
		using ctype=typename T::ctype;
		EncMetric_info(const EncMetric_info<T> &) noexcept {}
		EncMetric_info() {}
		const EncMetric<ctype> *format() const noexcept {return DynEncoding<T>::instance();}

		constexpr uint min_bytes() const noexcept {return T::min_bytes();}
		constexpr bool has_max() const noexcept {return T::has_max();}
		constexpr uint max_bytes() const noexcept {return T::max_bytes();}
		constexpr bool is_fixed() const noexcept {return fixed_size<T>;}
		uint chLen(const byte *b, size_t siz) const {return T::chLen(b, siz);}
		validation_result validChar(const byte *b, size_t l) const noexcept {return T::validChar(b, l);}
		uint decode(ctype *uni, const byte *by, size_t l) const {return T::decode(uni, by, l);}
		uint encode(const ctype &uni, byte *by, size_t l) const {return T::encode(uni, by, l);}
		std::type_index index() const noexcept {return index_traits<T>::index();}

		template<general_enctype S>
		bool equalTo(EncMetric_info<S> o) const noexcept{
            if constexpr(not_widenc<S>)
                return same_enc<S, T>;
            else
                return index() == o.index();
        }
        template<general_enctype S>
        bool can_reassign_to() const noexcept{
            if constexpr(enc_raw<S>)
                return true;
            else if constexpr(widenc<S>)
                return same_data<T, S>;
            else
                return same_enc<T, S>;
        }
        template<general_enctype S>
        EncMetric_info<S> reassign() const{
            static_assert(weak_assign<T, S>, "Cannot reassign these encodings");
            if(!can_reassign_to<S>())
                throw encoding_error{"Cannot reassign these encodings"};
            if constexpr(widenc<S>)
                return EncMetric_info<S>{DynEncoding<T>::instance()};
            else
                return EncMetric_info<S>{};
        }
};

template<typename tt>
class EncMetric_info<WIDE<tt>>{
	private:
		const EncMetric<tt> *f;
	public:
		using ctype=tt;
		EncMetric_info(const EncMetric<tt> *format) : f{format} {}
		EncMetric_info(const EncMetric_info &info) : f{info.f} {}
		const EncMetric<tt> *format() const noexcept {return f;}

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
        bool can_reassign_to() const noexcept{
            if constexpr(enc_raw<S>)
                return true;
            else if constexpr(widenc<S>)
                return same_data<WIDE<tt>, S>;
            else
                return index() == index_traits<S>::index();
        }
        template<general_enctype S>
        EncMetric_info<S> reassign() const{
            static_assert(weak_assign<WIDE<tt>, S>, "Cannot reassign these encodings");
            if(!can_reassign_to<S>())
                throw encoding_error{"Cannot reassign these encodings"};
            if constexpr(widenc<S>)
                return EncMetric_info<S>{f};
            else
                return EncMetric_info<S>{};
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
		static uint chLen(const byte *, size_t siz) {return 1;}
		static validation_result validChar(const byte *, size_t) noexcept;
		static uint decode(unicode *uni, const byte *by, size_t l);
		static uint encode(const unicode &uni, byte *by, size_t l);
};

class Latin1{
	public:
		using ctype=unicode;
		static constexpr uint min_bytes() noexcept {return 1;}
		static constexpr bool has_max() noexcept {return true;}
		static constexpr uint max_bytes() noexcept {return 1;}
		static uint chLen(const byte *, size_t siz) {return 1;}
		static validation_result validChar(const byte *, size_t) noexcept;
		static uint decode(unicode *uni, const byte *by, size_t l);
		static uint encode(const unicode &uni, byte *by, size_t l);
};

}


