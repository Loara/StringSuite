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
    Pointers to encoded string types.

    We don't use a generic const byte * to point the string, instead we use an
    encoding-dependent class in order to simplify the char-length operations
*/
/*
    Remember that
    - length = number of characters
    - size = number of bytes
*/
#include <type_traits>
#include <typeindex>
#include <cstddef>
#include <functional>
#include <strsuite/encmetric/config.hpp>
#include <strsuite/encmetric/encmetric.hpp>

namespace sts{

/*
    Base class of all pointer-type operations
*/
template<general_enctype T, typename U, typename B>
class base_tchar_pt{
	private:
		using data_type_0 = B;
		/*
		    Static cast operations
		*/
		U* mycast() noexcept {return static_cast<U*>(this);}
		U& instance() noexcept {return *(mycast());}
		const U* mycast() const noexcept {return static_cast<const U*>(this);}
		const U& instance() const noexcept {return *(mycast());}
		U p_new_instance(data_type_0 *bin) const noexcept {return mycast()->new_instance(bin);}

	protected:
		data_type_0 *ptr;
		[[no_unique_address]] EncMetric_info<T> ei;
		explicit constexpr base_tchar_pt(data_type_0 *b, EncMetric_info<T> f) noexcept : ptr{b}, ei{f} {}
	public:
        using data_type = data_type_0;

		using ctype = typename EncMetric_info<T>::ctype;
		using static_enc = T;
		/*
		    Raw pointer
		*/
		constexpr data_type *data() const {return ptr;}
		constexpr EncMetric_info<T> raw_format() const noexcept{ return ei;}
		const EncMetric<ctype> *format() const noexcept {return ei.format();}

		constexpr bool isNull() const noexcept{return ptr == nullptr;}
		void rebase(data_type *p) noexcept{ptr = p;}
		constexpr uint min_bytes() const noexcept {return ei.min_bytes();}
		constexpr bool has_max() const noexcept {return ei.has_max();}
		constexpr uint max_bytes() const noexcept {return ei.max_bytes();}
		bool is_fixed() const noexcept {return ei.is_fixed();}
		uint chLen(size_t siz) const {return ei.chLen(ptr, siz);}
		validation_result validChar(size_t l) const noexcept {return ei.validChar(ptr, l);}
		std::tuple<uint, ctype> decode(size_t l) const {return ei.decode(ptr, l);}

		std::type_index index() const noexcept {return ei.index();}
		/*
		    Step the pointer by 1 character, returns the number og bytes skipped
		*/
		uint next(size_t siz);
		uint next_update(size_t &siz);
		validation_result valid_next(size_t siz) noexcept;
		validation_result valid_next_update(size_t &siz) noexcept;

		tuple_ret<ctype> decode_next(size_t l);
		tuple_ret<ctype> decode_next_update(size_t &l);
		/*
		    Access ptr as a byte array
		*/
		data_type &operator[](uint i) const {return ptr[i];}
		/*
		    Increase ptr by i bytes (not characters)
		*/
		U operator+(std::ptrdiff_t i) const{
			if(i <= 0)
				return instance();
			else
				return p_new_instance(ptr + i);
		}
		U &operator+=(std::ptrdiff_t i){
			if(i > 0)
				ptr += i;
			return instance();
		}
		/*
		    Difference of pointers in bytes
		*/
		std::ptrdiff_t operator-(const U &oth) const noexcept{
			return ptr - oth.data();
		}
		/*
		    Compairson
		*/
		bool operator==(const U &it) const {return ptr == it.data();}
		bool operator!=(const U &oth) const {return ptr != oth.data();}		
};

/*
    Adds the funstion wrapper from_unicode for rw pointers
*/
template<general_enctype T, typename U>
class wbase_tchar_pt : public base_tchar_pt<T, U, byte>{
	protected:
		constexpr explicit wbase_tchar_pt(byte *b, EncMetric_info<T> f) noexcept : base_tchar_pt<T, U, byte>{b, f} {}
	public:
		uint encode(const typename base_tchar_pt<T, U, byte>::ctype &uni, size_t l) const {return this->ei.encode(uni, this->ptr, l);}
		uint encode_next(const typename base_tchar_pt<T, U, byte>::ctype &uni, size_t l);
		uint encode_next_update(const typename base_tchar_pt<T, U, byte>::ctype &uni, size_t &l);
};

/*
    Standard implementations
*/
template<general_enctype T>
class const_tchar_pt : public base_tchar_pt<T, const_tchar_pt<T>, byte const>{
	public:
		constexpr explicit const_tchar_pt(const byte *c, EncMetric_info<T> f) noexcept : base_tchar_pt<T, const_tchar_pt<T>, byte const>{c, f} {}

		constexpr explicit const_tchar_pt(const byte *b) noexcept requires not_widenc<T> : const_tchar_pt{b, EncMetric_info<T>{}} {}
		constexpr explicit const_tchar_pt(const char *b)  noexcept requires not_widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{}} {}
		constexpr explicit const_tchar_pt(const char8_t *b) noexcept requires not_widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{}} {}
		constexpr explicit const_tchar_pt(const char16_t *b) noexcept requires not_widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{}} {}
		constexpr explicit const_tchar_pt(const char32_t *b) noexcept requires not_widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{}} {}
		constexpr explicit const_tchar_pt(std::nullptr_t) noexcept requires not_widenc<T> : const_tchar_pt{static_cast<const byte *>(nullptr), EncMetric_info<T>{}} {}

		constexpr explicit const_tchar_pt(const byte *b, const EncMetric<typename T::ctype> *f) noexcept requires widenc<T> : const_tchar_pt{b, EncMetric_info<T>{f}} {}
		constexpr explicit const_tchar_pt(const char *b, const EncMetric<typename T::ctype> *f) noexcept requires widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{f}} {}
		constexpr explicit const_tchar_pt(const char8_t *b, const EncMetric<typename T::ctype> *f) noexcept requires widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{f}} {}
		constexpr explicit const_tchar_pt(const char16_t *b, const EncMetric<typename T::ctype> *f) noexcept requires widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{f}} {}
		constexpr explicit const_tchar_pt(const char32_t *b, const EncMetric<typename T::ctype> *f) noexcept requires widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{f}} {}
		constexpr explicit const_tchar_pt(std::nullptr_t, const EncMetric<typename T::ctype> *f) noexcept requires widenc<T> : const_tchar_pt{static_cast<const byte *>(nullptr), EncMetric_info<T>{f}} {}

		const_tchar_pt new_instance(const byte *c) const noexcept{return const_tchar_pt<T>{c, this->ei};}
};

template<general_enctype T>
class tchar_pt : public wbase_tchar_pt<T, tchar_pt<T>>{
	public:
		constexpr explicit tchar_pt(byte *c, EncMetric_info<T> f) noexcept : wbase_tchar_pt<T, tchar_pt<T>>{c, f} {}

		constexpr explicit tchar_pt(byte *b) noexcept requires not_widenc<T> : tchar_pt{b, EncMetric_info<T>{}} {}
		constexpr explicit tchar_pt(char *b) noexcept requires not_widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{}} {}
		constexpr explicit tchar_pt(char8_t *b) noexcept requires not_widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{}} {}
		constexpr explicit tchar_pt(char16_t *b) noexcept requires not_widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{}} {}
		constexpr explicit tchar_pt(char32_t *b) noexcept requires not_widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{}} {}
		constexpr explicit tchar_pt(std::nullptr_t) noexcept requires not_widenc<T> : tchar_pt{static_cast<byte *>(nullptr), EncMetric_info<T>{}} {}


		constexpr explicit tchar_pt(byte *b, const EncMetric<typename T::ctype> *f) noexcept requires widenc<T> : tchar_pt{b, EncMetric_info<T>{f}} {}
		constexpr explicit tchar_pt(char *b, const EncMetric<typename T::ctype> *f) noexcept requires widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{f}} {}
		constexpr explicit tchar_pt(char8_t *b, const EncMetric<typename T::ctype> *f) noexcept requires widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{f}} {}
		constexpr explicit tchar_pt(char16_t *b, const EncMetric<typename T::ctype> *f) noexcept requires widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{f}} {}
		constexpr explicit tchar_pt(char32_t *b, const EncMetric<typename T::ctype> *f) noexcept requires widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{f}} {}
		constexpr explicit tchar_pt(std::nullptr_t, EncMetric<typename T::ctype> *f) noexcept requires widenc<T> : tchar_pt{static_cast<byte *>(nullptr), EncMetric_info<T>{f}} {}

		constexpr const_tchar_pt<T> cast() const noexcept{ return const_tchar_pt<T>{this->ptr, this->ei};}
		constexpr operator const_tchar_pt<T>() const noexcept{ return cast();}

		tchar_pt new_instance(byte *c) const noexcept{return tchar_pt<T>{c, this->ei};}
};

//---------------------------------------------

/*
 * Relative pointer type. Useful for reallocations of principal pointer
 */
template<general_enctype T>
class tchar_relative{
    private:
        const tchar_pt<T> &ptr;
        std::size_t dif;
    public:
        using ctype = typename T::ctype;
        constexpr tchar_relative(const tchar_pt<T> &fr, std::size_t t = 0) noexcept : ptr{fr}, dif{t} {}
        constexpr tchar_relative(tchar_pt<T> &&, std::size_t = 0) =delete;

        constexpr tchar_relative(const tchar_relative &e) noexcept : ptr{e.ptr}, dif{e.dif} {}

        tchar_relative &operator=(const tchar_relative &)=delete;
        tchar_relative &operator=(tchar_relative &&)=delete;

        constexpr byte *data() const {return ptr.data() + dif;}
        constexpr std::size_t difff() const noexcept {return dif;}
        void set_diff(std::size_t ns) noexcept {dif = ns;}
		constexpr EncMetric_info<T> raw_format() const noexcept{ return ptr.raw_format();}

		constexpr uint min_bytes() const noexcept {return raw_format().min_bytes();}
		constexpr bool has_max() const noexcept {return raw_format().has_max();}
		constexpr uint max_bytes() const noexcept {return raw_format().max_bytes();}
		constexpr bool is_fixed() const noexcept {return raw_format().is_fixed();}

		uint chLen(size_t siz) const {return raw_format().chLen(data(), siz);}
		validation_result validChar(size_t l) const noexcept {return raw_format().validChar(data(), l);}
		tuple_ret<ctype> decode(size_t l) const {return raw_format().decode(data(), l);}
		uint encode(const ctype &uni, size_t l) const {return raw_format().encode(uni, data(), l);}

		uint next(size_t siz);
		uint next_update(size_t &siz);

		tuple_ret<ctype> decode_next(size_t l);
		uint encode_next(const ctype &uni, size_t l);
		tuple_ret<ctype> decode_next_update(size_t &l);
		uint encode_next_update(const ctype &uni, size_t &l);

		tchar_pt<T> convert() const noexcept {return ptr.new_instance(data());}
		tchar_relative operator+(std::size_t t) const{
            return tchar_relative(ptr, dif + t);
        }
        tchar_relative &operator+=(std::size_t t){
            dif += t;
            return *this;
        }
};

//---------------------------------------------

template<general_enctype S, general_enctype T>
bool sameEnc(const const_tchar_pt<S> &s, const const_tchar_pt<T> &t) noexcept{
    return s.raw_format().equalTo(t.raw_format());
}

/*
    Assign an encoding to a RAW character pointer
*/
template<typename tt>
inline tchar_pt<WIDE<tt>> set_encoding(tchar_pt<RAW<tt>> r, const EncMetric<tt> *f) noexcept {return tchar_pt<WIDE<tt>>{r.data(), f};}
template<typename tt>
inline const_tchar_pt<WIDE<tt>> set_encoding(const_tchar_pt<RAW<tt>> r, const EncMetric<tt> *f) noexcept {return const_tchar_pt<WIDE<tt>>{r.data(), f};}

/*
 * S should be a base for T
 */
template<general_enctype T, general_enctype S>
bool can_rebase_pointer(const_tchar_pt<S> from, EncMetric_info<T> f) noexcept{
    return from.raw_format().base_for(f);
}
template<general_enctype T, general_enctype S>
void assert_rebase_pointer(const_tchar_pt<S> from, EncMetric_info<T> f){
    from.raw_format().assert_base_for(f);
}
template<general_enctype T, general_enctype S>
const_tchar_pt<T> rebase_pointer(const_tchar_pt<S> from, EncMetric_info<T> f){
    from.raw_format().assert_base_for(f);
    return const_tchar_pt<T>{from.data(), f};
}

template<general_enctype T, general_enctype S>
tchar_pt<T> rebase_pointer(tchar_pt<S> from, EncMetric_info<T> f){
    from.raw_format().assert_base_for(f);
    return tchar_pt<T>{from.data(), f};
}

/*
 * Use this method only if you want to write characters onto from
 * so if you want to write an ASCII character on tchar_pt<UTF8> use this function
 * to convert it
 */
template<general_enctype T, general_enctype S>
void assert_inv_rebase_pointer(tchar_pt<S> to, EncMetric_info<T> f){
    f.assert_base_for(to.raw_format());
}
template<general_enctype T, general_enctype S>
tchar_pt<T> inv_rebase_pointer(tchar_pt<S> to, EncMetric_info<T> f){
    f.assert_base_for(to.raw_format());
    return tchar_pt<T>{to.data(), f};
}

/*
    Estimate the size of a possible string with n characters
*/
template<general_enctype T>
uint min_size_estimate(const_tchar_pt<T>, uint) noexcept;
template<general_enctype T>
uint max_size_estimate(const_tchar_pt<T>, uint);

using c_wchar_pt = const_tchar_pt<WIDE<unicode>>;
using wchar_pt = tchar_pt<WIDE<unicode>>;

#include <strsuite/encmetric/chite.tpp>
}

