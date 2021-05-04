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
};

/*
    Standard implementations
*/
template<general_enctype T>
class const_tchar_pt : public base_tchar_pt<T, const_tchar_pt<T>, byte const>{
	public:
		explicit const_tchar_pt(const byte *c, EncMetric_info<T> f) : base_tchar_pt<T, const_tchar_pt<T>, byte const>{c, f} {}

		//bool terminate() const {return is_all_zero(this->ptr, this->ei.unity());}
        //No more required

		const_tchar_pt new_instance(const byte *c) const{return const_tchar_pt<T>{c, this->ei};}
		const_tchar_pt new_instance(const char *c) const{return const_tchar_pt<T>{reinterpret_cast<const byte *>(c), this->ei};}
};

template<typename T, enable_not_wide_t<T, int> =0>
const_tchar_pt<T> new_const_tchar_pt(const byte *b)noexcept{
    return const_tchar_pt<T>(b, EncMetric_info<T>{});
}
template<typename T, enable_not_wide_t<T, int> =0>
const_tchar_pt<T> new_const_tchar_pt(const char *b)noexcept{
    return const_tchar_pt<T>(reinterpret_cast<const byte *>(b), EncMetric_info<T>{});
}
template<typename T, enable_not_wide_t<T, int> =0>
const_tchar_pt<T> new_const_tchar_pt(const char16_t *b)noexcept{
    return const_tchar_pt<T>(reinterpret_cast<const byte *>(b), EncMetric_info<T>{});
}
template<typename T, enable_not_wide_t<T, int> =0>
const_tchar_pt<T> new_const_tchar_pt(const char32_t *b)noexcept{
    return const_tchar_pt<T>(reinterpret_cast<const byte *>(b), EncMetric_info<T>{});
}
template<typename T, enable_not_wide_t<T, int> =0>
const_tchar_pt<T> new_const_tchar_pt()noexcept{
    return const_tchar_pt<T>(nullptr, EncMetric_info<T>{});
}

template<typename T, enable_wide_t<T, int> =0>
const_tchar_pt<T> new_const_tchar_pt(const byte *b, const EncMetric<typename T::ctype> *f)noexcept{
    return const_tchar_pt<T>(b, EncMetric_info<T>{f});
}
template<typename T, enable_wide_t<T, int> =0>
const_tchar_pt<T> new_const_tchar_pt(const char *b, const EncMetric<typename T::ctype> *f)noexcept{
    return const_tchar_pt<T>(reinterpret_cast<const byte *>(b), EncMetric_info<T>{f});
}
template<typename T, enable_wide_t<T, int> =0>
const_tchar_pt<T> new_const_tchar_pt(const char16_t *b, const EncMetric<typename T::ctype> *f)noexcept{
    return const_tchar_pt<T>(reinterpret_cast<const byte *>(b), EncMetric_info<T>{f});
}
template<typename T, enable_wide_t<T, int> =0>
const_tchar_pt<T> new_const_tchar_pt(const char32_t *b, const EncMetric<typename T::ctype> *f)noexcept{
    return const_tchar_pt<T>(reinterpret_cast<const byte *>(b), EncMetric_info<T>{f});
}
template<typename T, enable_wide_t<T, int> =0>
const_tchar_pt<T> new_const_tchar_pt(const EncMetric<typename T::ctype> *f)noexcept{
    return const_tchar_pt<T>(nullptr, EncMetric_info<T>{f});
}
/*
template<typename tt>
class const_tchar_pt<WIDE<tt>> : public base_tchar_pt<WIDE<tt>, const_tchar_pt<WIDE<tt>>, byte const>{
	public:
		explicit const_tchar_pt(const byte *c, EncMetric_info<WIDE<tt>> f) : base_tchar_pt<WIDE<tt>, const_tchar_pt<WIDE<tt>>, byte const>{c, f} {}

		explicit const_tchar_pt(const byte *c, const EncMetric<tt> *format) : const_tchar_pt{c, EncMetric_info<WIDE<tt>>{format}} {}
		explicit const_tchar_pt(const char *c, const EncMetric<tt> *format) : const_tchar_pt{reinterpret_cast<const byte *>(c), EncMetric_info<WIDE<tt>>{format}} {}
		explicit const_tchar_pt(const char16_t *c, const EncMetric<tt> *format) : const_tchar_pt{reinterpret_cast<const byte *>(c), EncMetric_info<WIDE<tt>>{format}} {}
		explicit const_tchar_pt(const char32_t *c, const EncMetric<tt> *format) : const_tchar_pt{reinterpret_cast<const byte *>(c), EncMetric_info<WIDE<tt>>{format}} {}
		explicit const_tchar_pt(const EncMetric<tt> *format) : const_tchar_pt{static_cast<const byte *>(nullptr), EncMetric_info<WIDE<tt>>{format}} {}

		//bool terminate() const {return is_all_zero(this->ptr, this->ei.unity());}
        //No more required

		const_tchar_pt new_instance(const byte *c) const{return const_tchar_pt<WIDE<tt>>{c, this->ei};}
		const_tchar_pt new_instance(const char *c) const{return const_tchar_pt<WIDE<tt>>{reinterpret_cast<const byte*>(c), this->ei};}
};
*/

template<general_enctype T>
class tchar_pt : public wbase_tchar_pt<T, tchar_pt<T>>{
	public:
		explicit tchar_pt(byte *c, EncMetric_info<T> f) : wbase_tchar_pt<T, tchar_pt<T>>{c, f} {}

		//bool terminate() const {return is_all_zero(this->ptr, this->ei.unity());}
		const_tchar_pt<T> cast() const noexcept{ return const_tchar_pt<T>{this->ptr, this->ei};}
		operator const_tchar_pt<T>() const noexcept{ return cast();}

		tchar_pt new_instance(byte *c) const{return tchar_pt<T>{c, this->ei};}
		tchar_pt new_instance(char *c) const{return tchar_pt<T>{reinterpret_cast<byte *>(c), this->ei};}
};



template<typename T, enable_not_wide_t<T, int> =0>
tchar_pt<T> new_tchar_pt(byte *b)noexcept{
    return tchar_pt<T>(b, EncMetric_info<T>{});
}
template<typename T, enable_not_wide_t<T, int> =0>
tchar_pt<T> new_tchar_pt(char *b)noexcept{
    return tchar_pt<T>(reinterpret_cast<byte *>(b), EncMetric_info<T>{});
}
template<typename T, enable_not_wide_t<T, int> =0>
tchar_pt<T> new_tchar_pt(char16_t *b)noexcept{
    return tchar_pt<T>(reinterpret_cast<byte *>(b), EncMetric_info<T>{});
}
template<typename T, enable_not_wide_t<T, int> =0>
tchar_pt<T> new_tchar_pt(char32_t *b)noexcept{
    return tchar_pt<T>(reinterpret_cast<byte *>(b), EncMetric_info<T>{});
}
template<typename T, enable_not_wide_t<T, int> =0>
tchar_pt<T> new_tchar_pt()noexcept{
    return tchar_pt<T>(nullptr, EncMetric_info<T>{});
}

template<typename T, enable_wide_t<T, int> =0>
tchar_pt<T> new_tchar_pt(byte *b, const EncMetric<typename T::ctype> *f)noexcept{
    return tchar_pt<T>(b, EncMetric_info<T>{f});
}
template<typename T, enable_wide_t<T, int> =0>
tchar_pt<T> new_tchar_pt(char *b, const EncMetric<typename T::ctype> *f)noexcept{
    return tchar_pt<T>(reinterpret_cast<byte *>(b), EncMetric_info<T>{f});
}
template<typename T, enable_wide_t<T, int> =0>
tchar_pt<T> new_tchar_pt(char16_t *b, const EncMetric<typename T::ctype> *f)noexcept{
    return tchar_pt<T>(reinterpret_cast<byte *>(b), EncMetric_info<T>{f});
}
template<typename T, enable_wide_t<T, int> =0>
tchar_pt<T> new_tchar_pt(char32_t *b, const EncMetric<typename T::ctype> *f)noexcept{
    return tchar_pt<T>(reinterpret_cast<byte *>(b), EncMetric_info<T>{f});
}
template<typename T, enable_wide_t<T, int> =0>
tchar_pt<T> new_tchar_pt(const EncMetric<typename T::ctype> *f)noexcept{
    return tchar_pt<T>(nullptr, EncMetric_info<T>{f});
}
/*
template<typename tt>
class tchar_pt<WIDE<tt>> : public wbase_tchar_pt<WIDE<tt>, tchar_pt<WIDE<tt>>>{
	public:
		explicit tchar_pt(byte *c, EncMetric_info<WIDE<tt>> f) : wbase_tchar_pt<WIDE<tt>, tchar_pt<WIDE<tt>>>{c, f} {}

		explicit tchar_pt(byte *c, const EncMetric<tt> *format) : wbase_tchar_pt<WIDE<tt>, tchar_pt<WIDE<tt>>>{c, EncMetric_info<WIDE<tt>>{format}} {}
		explicit tchar_pt(char *c, const EncMetric<tt> *format) : tchar_pt{reinterpret_cast<byte *>(c), EncMetric_info<WIDE<tt>>{format}} {}
		explicit tchar_pt(char16_t *c, const EncMetric<tt> *format) : tchar_pt{reinterpret_cast<byte *>(c), EncMetric_info<WIDE<tt>>{format}} {}
		explicit tchar_pt(char32_t *c, const EncMetric<tt> *format) : tchar_pt{reinterpret_cast<byte *>(c), EncMetric_info<WIDE<tt>>{format}} {}

		explicit tchar_pt(const EncMetric<tt> *form) : tchar_pt{static_cast<byte *>(nullptr), EncMetric_info<WIDE<tt>>{form}} {}

		//bool terminate() const {return is_all_zero(this->ptr, this->ei.unity());}
		const_tchar_pt<WIDE<tt>> cast() const noexcept{ return const_tchar_pt<WIDE<tt>>{this->ptr, this->ei};}

		tchar_pt new_instance(byte *c) const{return tchar_pt<WIDE<tt>>{c, this->ei};}
		tchar_pt new_instance(char *c) const{return tchar_pt<WIDE<tt>>{reinterpret_cast<byte *>(c), this->ei};}
};
*/
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

template<typename S, typename T>
bool sameEnc(const const_tchar_pt<S> &, const const_tchar_pt<T> &) noexcept;
template<typename T1, typename T2>
bool sameEnc(const T1 &arg1, const T2 &arg2) noexcept{
	return sameEnc(const_tchar_pt<typename T1::static_enc>{arg1}, const_tchar_pt<typename T2::static_enc>{arg2});
}
template<typename S, typename T, typename... Rarg>
bool sameEnc(const S &f1, const T &f2, const Rarg&... tre) noexcept{
	return sameEnc(f2, tre...) && sameEnc(f1, f2);
}
/*
	T not wide
*/
template<typename S, typename T>
bool sameEnc(const const_tchar_pt<T> &) noexcept;

/*
	Different from sameEnc, since it can handle also WIDE encodings and also allow
	conversion to any RAW encoded string, also with different ctype
*/
template<typename S, typename T>
bool can_reassign(const const_tchar_pt<T> &) noexcept;
/*
	Return a new pointer pointing to the same array and with a similar, but with possible different template parameter.
*/
template<typename S, typename T>
tchar_pt<S> reassign(tchar_pt<T> p);
template<typename S, typename T>
const_tchar_pt<S> reassign(const_tchar_pt<T> p);

/*
    Assign an encoding to a RAW character pointer
*/
template<typename tt>
inline tchar_pt<WIDE<tt>> set_encoding(tchar_pt<RAW<tt>> r, const EncMetric<tt> &f) noexcept {return tchar_pt<WIDE<tt>>{r.data(), f};}
template<typename tt>
inline const_tchar_pt<WIDE<tt>> set_encoding(const_tchar_pt<RAW<tt>> r, const EncMetric<tt> &f) noexcept {return const_tchar_pt<WIDE<tt>>{r.data(), f};}

/*
    Make an encoding conversion between Unicode-compatible encodings using from_unicode and to_unicode functions.
    Note: convert only the first character
*/
template<typename S, typename T, enable_same_data_t<S, T, int> =0>
void basic_encoding_conversion(const_tchar_pt<T> in, uint inlen, tchar_pt<S> out, uint oulen);
template<typename S, typename T, enable_same_data_t<S, T, int> =0>
void basic_encoding_conversion(const_tchar_pt<T> in, uint inlen, tchar_pt<S> out, uint oulen, uint &inread, uint &outwrite);

/*
    Estimate the size of a possible string with n characters
*/
template<typename T>
uint min_size_estimate(const_tchar_pt<T>, uint) noexcept;
template<typename T>
uint max_size_estimate(const_tchar_pt<T>, uint);

template<typename T>
bool dynamic_fixed_size(const_tchar_pt<T>) noexcept;

using c_wchar_pt = const_tchar_pt<WIDE<unicode>>;
using wchar_pt = tchar_pt<WIDE<unicode>>;

#include <strsuite/encmetric/chite.tpp>
}

