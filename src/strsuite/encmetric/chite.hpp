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
#include <strsuite/encmetric/encoding.hpp>

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
		EncMetric_info<T> ei;
		explicit base_tchar_pt(data_type_0 *b, EncMetric_info<T> f) : ptr{b}, ei{f} {}
	public:
        using data_type = data_type_0;
		using ctype = typename EncMetric_info<T>::ctype;
		using static_enc = T;
		/*
		    Raw pointer
		*/
		data_type *data() const {return ptr;}
		EncMetric_info<T> raw_format() const noexcept{ return ei;}
		const EncMetric<ctype> &format() const noexcept {return ei.format();}
		/*
		    Informations about relative EncMetric
		*/
		uint unity() const noexcept {return ei.unity();}
		bool has_max() const noexcept {return ei.has_max();}
		uint max_bytes() const noexcept {return ei.max_bytes();}
		bool is_fixed() const noexcept {return ei.is_fixed();}
		uint chLen() const {return ei.chLen(ptr);}
		bool validChar(uint &l) const noexcept {return ei.validChar(ptr, l);}
		uint decode(ctype *uni, size_t l) const {return ei.decode(uni, ptr, l);}

		std::type_index index() const noexcept {return ei.index();}
		/*
		    Step the pointer by 1 character, returns the number og bytes skipped
		*/
		uint next(){
			if constexpr(fixed_size<T>){
				ptr += T::unity();
				return T::unity();
			}
			else{
				uint add = chLen();
				ptr += add;
				return add;
			}
		}
		/*
		    Validate the first character, then skip it

		    rsiz is the number of bytes of ptr, it will be updated
		*/
		bool valid_next(size_t &rsiz) noexcept{
			if(unity() > rsiz)
				return false;
			uint dec;
			if(!validChar(dec))
				return false;
			if(dec > rsiz)
				return false;
			rsiz -= dec;
			ptr += dec;
			return true;
		}

		uint decode_next(ctype *uni, size_t l){
            uint ret = ei.decode(uni, ptr, l);
            ptr += ret;
            return ret;
        }
		/*
		    Test if is a null string
		*/
		bool isNull() const {return ptr == nullptr;}
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
		explicit wbase_tchar_pt(byte *b, EncMetric_info<T> f) : base_tchar_pt<T, U, byte>{b, f} {}
	public:
		uint encode(const typename base_tchar_pt<T, U, byte>::ctype &uni, size_t l) const {return this->ei.encode(uni, this->ptr, l);}
		uint encode_next(const typename base_tchar_pt<T, U, byte>::ctype &uni, size_t l) {
            uint ret = this->ei.encode(uni, this->ptr, l);
            this->ptr += ret;
            return ret;
        }
};

/*
    Standard implementations
*/
template<general_enctype T>
class const_tchar_pt : public base_tchar_pt<T, const_tchar_pt<T>, byte const>{
	public:
		explicit const_tchar_pt(const byte *c, EncMetric_info<T> f) : base_tchar_pt<T, const_tchar_pt<T>, byte const>{c, f} {}

        #if costructors_concepts
		explicit const_tchar_pt(const byte *b) requires not_widenc<T> : const_tchar_pt{b, EncMetric_info<T>{}} {}
		explicit const_tchar_pt(const char *b) requires not_widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{}} {}
		explicit const_tchar_pt(const char8_t *b) requires not_widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{}} {}
		explicit const_tchar_pt(const char16_t *b) requires not_widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{}} {}
		explicit const_tchar_pt(const char32_t *b) requires not_widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{}} {}
		explicit const_tchar_pt(std::nullptr_t) requires not_widenc<T> : const_tchar_pt{static_cast<const byte *>(nullptr), EncMetric_info<T>{}} {}

		explicit const_tchar_pt(const byte *b, const EncMetric<typename T::ctype> *f) requires widenc<T> : const_tchar_pt{b, EncMetric_info<T>{f}} {}
		explicit const_tchar_pt(const char *b, const EncMetric<typename T::ctype> *f) requires widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{f}} {}
		explicit const_tchar_pt(const char8_t *b, const EncMetric<typename T::ctype> *f) requires widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{f}} {}
		explicit const_tchar_pt(const char16_t *b, const EncMetric<typename T::ctype> *f) requires widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{f}} {}
		explicit const_tchar_pt(const char32_t *b, const EncMetric<typename T::ctype> *f) requires widenc<T> : const_tchar_pt{reinterpret_cast<const byte *>(b), EncMetric_info<T>{f}} {}
		explicit const_tchar_pt(std::nullptr_t, const EncMetric<typename T::ctype> *f) requires widenc<T> : const_tchar_pt{static_cast<const byte *>(nullptr), EncMetric_info<T>{f}} {}
        #endif

		const_tchar_pt new_instance(const byte *c) const{return const_tchar_pt<T>{c, this->ei};}
		const_tchar_pt new_instance(const char *c) const{return const_tchar_pt<T>{reinterpret_cast<const byte *>(c), this->ei};}
};

/*
 * Additional costructors
 */

template<strong_enctype T>
const_tchar_pt<T> new_const_pt(const byte *b){
    return const_tchar_pt<T>{b, EncMetric_info<T>{}};
}
template<strong_enctype T>
const_tchar_pt<T> new_const_pt(const char *b){
    return const_tchar_pt<T>{reinterpret_cast<const byte *>(b), EncMetric_info<T>{}};
}
template<strong_enctype T>
const_tchar_pt<T> new_const_pt(){
    return const_tchar_pt<T>{nullptr, EncMetric_info<T>{}};
}

template<widenc T>
const_tchar_pt<T> new_const_pt(const byte *b, const EncMetric<typename T::ctype> *f){
    return const_tchar_pt<T>{b, EncMetric_info<T>{f}};
}
template<widenc T>
const_tchar_pt<T> new_const_pt(const char *b, const EncMetric<typename T::ctype> *f){
    return const_tchar_pt<T>{reinterpret_cast<const byte *>(b), EncMetric_info<T>{f}};
}
template<widenc T>
const_tchar_pt<T> new_const_pt(const EncMetric<typename T::ctype> *f){
    return const_tchar_pt<T>{nullptr, EncMetric_info<T>{f}};
}

template<general_enctype T>
class tchar_pt : public wbase_tchar_pt<T, tchar_pt<T>>{
	public:
		explicit tchar_pt(byte *c, EncMetric_info<T> f) : wbase_tchar_pt<T, tchar_pt<T>>{c, f} {}

        #if costructors_concepts
		explicit tchar_pt(byte *b) requires not_widenc<T> : tchar_pt{b, EncMetric_info<T>{}} {}
		explicit tchar_pt(char *b) requires not_widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{}} {}
		explicit tchar_pt(char8_t *b) requires not_widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{}} {}
		explicit tchar_pt(char16_t *b) requires not_widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{}} {}
		explicit tchar_pt(char32_t *b) requires not_widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{}} {}
		explicit tchar_pt(std::nullptr_t) requires not_widenc<T> : tchar_pt{static_cast<byte *>(nullptr), EncMetric_info<T>{}} {}


		explicit tchar_pt(byte *b, const EncMetric<typename T::ctype> *f) requires widenc<T> : tchar_pt{b, EncMetric_info<T>{f}} {}
		explicit tchar_pt(char *b, const EncMetric<typename T::ctype> *f) requires widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{f}} {}
		explicit tchar_pt(char8_t *b, const EncMetric<typename T::ctype> *f) requires widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{f}} {}
		explicit tchar_pt(char16_t *b, const EncMetric<typename T::ctype> *f) requires widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{f}} {}
		explicit tchar_pt(char32_t *b, const EncMetric<typename T::ctype> *f) requires widenc<T> : tchar_pt{reinterpret_cast<byte *>(b), EncMetric_info<T>{f}} {}
		explicit tchar_pt(std::nullptr_t, EncMetric<typename T::ctype> *f) requires widenc<T> : tchar_pt{static_cast<byte *>(nullptr), EncMetric_info<T>{f}} {}
        #endif

		const_tchar_pt<T> cast() const noexcept{ return const_tchar_pt<T>{this->ptr, this->ei};}
		operator const_tchar_pt<T>() const noexcept{ return cast();}

		tchar_pt new_instance(byte *c) const{return tchar_pt<T>{c, this->ei};}
		tchar_pt new_instance(char *c) const{return tchar_pt<T>{reinterpret_cast<byte *>(c), this->ei};}
};

template<strong_enctype T>
tchar_pt<T> new_pt(byte *b){
    return tchar_pt<T>{b, EncMetric_info<T>{}};
}
template<strong_enctype T>
tchar_pt<T> new_pt(char *b){
    return tchar_pt<T>{reinterpret_cast<byte *>(b), EncMetric_info<T>{}};
}
template<strong_enctype T>
tchar_pt<T> new_pt(){
    return tchar_pt<T>{nullptr, EncMetric_info<T>{}};
}

template<widenc T>
tchar_pt<T> new_pt(byte *b, const EncMetric<typename T::ctype> *f){
    return tchar_pt<T>{b, EncMetric_info<T>{f}};
}
template<widenc T>
tchar_pt<T> new_pt(char *b, const EncMetric<typename T::ctype> *f){
    return tchar_pt<T>{reinterpret_cast<byte *>(b), EncMetric_info<T>{f}};
}
template<widenc T>
tchar_pt<T> new_pt(const EncMetric<typename T::ctype> *f){
    return tchar_pt<T>{nullptr, EncMetric_info<T>{f}};
}

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
        tchar_relative(const tchar_pt<T> &fr, std::size_t t = 0) : ptr{fr}, dif{t} {}
        tchar_relative(tchar_pt<T> &&, std::size_t = 0) =delete;

        byte *data() const {return ptr.data() + dif;}
        std::size_t difff() const noexcept {return dif;}
		EncMetric_info<T> raw_format() const noexcept{ return ptr.raw_format();}

		uint unity() const noexcept {return raw_format().unity();}
		bool has_max() const noexcept {return raw_format().has_max();}
		uint max_bytes() const noexcept {return raw_format().max_bytes();}
		bool is_fixed() const noexcept {return raw_format().is_fixed();}
		uint chLen() const {return raw_format().chLen(data());}
		bool validChar(uint &l) const noexcept {return raw_format().validChar(data(), l);}
		uint decode(ctype *uni, size_t l) const {return raw_format().decode(uni, data(), l);}
		uint encode(ctype &uni, size_t l) const {return raw_format().encode(uni, data(), l);}

		uint decode_next(ctype *uni, size_t l) {
            uint ret = raw_format().decode(uni, data(), l);
            dif += ret;
            return ret;
        }
		uint encode_next(ctype &uni, size_t l) {
            uint ret = raw_format().encode(uni, data(), l);
            dif += ret;
            return ret;
        }

		tchar_pt<T> convert() const noexcept {return ptr.new_instance(data());}
		tchar_relative operator+(std::size_t t) const{
            return tchar_relative(ptr, dif + t);
        }
		uint next(){
			if constexpr(fixed_size<T>){
				dif += T::unity();
				return T::unity();
			}
			else{
				uint add = raw_format().chLen(data());
				dif += add;
				return add;
			}
		}
};

//---------------------------------------------

template<general_enctype S, general_enctype T>
bool sameEnc(const const_tchar_pt<S> &s, const const_tchar_pt<T> &t) noexcept{
    return s.raw_format().equalTo(t.raw_format());
}

/*
	Return a new pointer pointing to the same array and with a similar, but with possible different template parameter.
*/
template<general_enctype S, general_enctype T>
bool can_reassign_to(const_tchar_pt<T> chr) noexcept {return chr.raw_format().template can_reassign_to<S>();};

template<general_enctype S, general_enctype T>
tchar_pt<S> reassign(tchar_pt<T> p);
template<general_enctype S, general_enctype T>
const_tchar_pt<S> reassign(const_tchar_pt<T> p);

/*
    Assign an encoding to a RAW character pointer
*/
template<typename tt>
inline tchar_pt<WIDE<tt>> set_encoding(tchar_pt<RAW<tt>> r, const EncMetric<tt> *f) noexcept {return tchar_pt<WIDE<tt>>{r.data(), f};}
template<typename tt>
inline const_tchar_pt<WIDE<tt>> set_encoding(const_tchar_pt<RAW<tt>> r, const EncMetric<tt> *f) noexcept {return const_tchar_pt<WIDE<tt>>{r.data(), f};}

/*
    Estimate the size of a possible string with n characters
*/
template<general_enctype T>
uint min_size_estimate(const_tchar_pt<T>, uint) noexcept;
template<general_enctype T>
uint max_size_estimate(const_tchar_pt<T>, uint);

template<general_enctype T>
bool dynamic_fixed_size(const_tchar_pt<T>) noexcept;

using c_wchar_pt = const_tchar_pt<WIDE<unicode>>;
using wchar_pt = tchar_pt<WIDE<unicode>>;

#include <strsuite/encmetric/chite.tpp>
}

