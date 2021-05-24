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
#include <vector>
#include <memory>
#include <memory_resource>
#include <stdexcept>
#include <type_traits>
#include <string>
#include <strsuite/encmetric/config.hpp>
#include <strsuite/encmetric/chite.hpp>
#include <strsuite/encmetric/basic_ptr.hpp>

namespace sts{

template<general_enctype T>
using terminate_func = std::function<bool(const byte *, const EncMetric_info<T> &, size_t)>;
/*
template<typename T>
void deduce_lens(const_tchar_pt<T>, size_t maxsiz, size_t &len, size_t &siz, const terminate_func<T> &);

template<typename T>
void deduce_lens(const_tchar_pt<T>, size_t rsiz, bool zero, size_t &len, size_t &siz);
*/

//enum class meas {size, length};

/*
 * Basic terminate function: string is terminated if and only if the encoded character is all 0 bytes
 */
template<general_enctype T>
bool zero_terminating(const byte *data, const EncMetric_info<T> &format, size_t){
    uint size = format.min_bytes();
	for(uint i=0; i<size; i++){
		if(data[i] != byte{0})
			return false;
	}
	return true;
}
/*
 * Another terminate function: terminate if and only if the encoded character is equivalent to 0
 *
 * WARNING: control up to st characters and may throw if the string is not correctly encoded
 */
template<general_enctype T>
bool encoding_terminating(const byte *data, const EncMetric_info<T> &format, size_t st){
    using ctype = typename T::ctype;
    ctype cha;
    try{
        format.decode(&cha, data, st);
    }
    catch(const buffer_small &){
        return false;
    }
    return cha == ctype{0};
}

template<general_enctype T>
class adv_string; //forward declaration

template<general_enctype T>
class adv_string_view{
	private:
		const_tchar_pt<T> ptr;
		size_t len;//character number
		size_t siz;//bytes number
	protected:
		explicit adv_string_view(size_t length, size_t size, const_tchar_pt<T> bin) noexcept : ptr{bin}, len{length}, siz{size} {}
	public:
		explicit adv_string_view(const_tchar_pt<T>, size_t maxsiz, const terminate_func<T> & = zero_terminating<T>);
		/*
		    read at least len characters and/or siz bytes
		*/
		explicit adv_string_view(const_tchar_pt<T>, size_t maxsiz, size_t maxlen);

		explicit adv_string_view(const byte *b, EncMetric_info<T> f, size_t maxsiz, const terminate_func<T> &tf = zero_terminating<T>) : adv_string_view{const_tchar_pt<T>{b, f}, maxsiz, tf} {}
		explicit adv_string_view(const byte *b, EncMetric_info<T> f, size_t maxsiz, size_t maxlen) : adv_string_view{const_tchar_pt<T>{b, f}, maxsiz, maxlen} {}

        #if costructors_concepts
		template<typename U>
		explicit adv_string_view(const U *b, size_t maxsiz, const terminate_func<T> &tf = zero_terminating<T>) requires not_widenc<T> : adv_string_view{const_tchar_pt<T>{b}, maxsiz, tf} {}
		template<typename U>
		explicit adv_string_view(const U *b, size_t siz, size_t len) requires not_widenc<T> : adv_string_view{const_tchar_pt<T>{b}, siz, len} {}

		template<typename U>
		explicit adv_string_view(const U *b, size_t maxsiz, const EncMetric<typename T::ctype> *f, const terminate_func<T> &tf = zero_terminating<T>) requires widenc<T> : adv_string_view{const_tchar_pt<T>{b, f}, maxsiz, tf} {}
		template<typename U>
		explicit adv_string_view(const U *b, const EncMetric<typename T::ctype> *f, size_t siz, size_t len) requires widenc<T> : adv_string_view{const_tchar_pt<T>{b, f}, siz, len} {}
        #endif

		virtual ~adv_string_view() {}
		/*
		    Verify the string is correctly encoded
		*/
		void verify() const;
		bool verify_safe() const noexcept;
		
		adv_string_view<T> substring(size_t b, size_t e, bool endstr) const;
		adv_string_view<T> substring(size_t b, size_t e) const {return substring(b, e, false);}
		adv_string_view<T> substring(size_t b) const {return substring(b, 0, true);}
		size_t length() const noexcept {return len;}
		size_t size() const noexcept {return siz;}
		size_t size(size_t a, size_t n) const;//bytes of first n character starting from the (a+1)-st character
		size_t size(size_t n) const {return size(0, n);}
		EncMetric_info<T> raw_format() const noexcept{ return ptr.raw_format();}

		template<general_enctype S>
		bool equal_to(const adv_string_view<S> &, size_t n) const;//compare only the first n character

		template<general_enctype S>
		bool operator==(const adv_string_view<S> &) const;

		template<general_enctype S>
		bool operator!=(const adv_string_view<S> &bin) const {return !(*this == bin);}

		template<general_enctype S>
		index_result bytesOf(const adv_string_view<S> &) const;

		template<general_enctype S>
		index_result indexOf(const adv_string_view<S> &) const;

		template<general_enctype S>
		bool containsChar(const adv_string_view<S> &) const;

		template<general_enctype S>
		bool startsWith(const adv_string_view<S> &) const;

		template<general_enctype S>
		bool endsWith(const adv_string_view<S> &) const;

		const byte *data() const noexcept {return ptr.data();}
		const char *raw() const noexcept {return (const char *)(ptr.data());}
		std::string toString() const noexcept {return std::string{(const char *)(ptr.data()), siz};}
		/*
			Mustn't throw any exception if 0 <= a <= len
		*/
		const_tchar_pt<T> at(size_t chr) const;
		const_tchar_pt<T> begin() const noexcept {return at(0);}
		const_tchar_pt<T> end() const noexcept {return at(len);}

		template<general_enctype S>
		adv_string<T> concatenate(const adv_string_view<S> &, std::pmr::memory_resource * = std::pmr::get_default_resource()) const;

	friend adv_string_view<T> direct_build(const_tchar_pt<T> ptr, size_t len, size_t siz) noexcept;
};

/*
 * Doesn't perform any control about dimensions
 *
 * Use with care
 */
template<general_enctype T>
adv_string_view<T> direct_build(const_tchar_pt<T> ptr, size_t len, size_t siz) noexcept{
    return adv_string_view<T>{len, siz, ptr};
}


template<strong_enctype T, typename U>
adv_string_view<T> new_string_view(const U *b, size_t maxsiz, const terminate_func<T> &t = zero_terminating<T>){
        return adv_string_view{new_const_pt<T>(b), maxsiz, t};
}

template<strong_enctype T, typename U>
adv_string_view<T> new_string_view(const U *b, size_t siz, size_t len){
        return adv_string_view{new_const_pt<T>(b), siz, len};
}

template<widenc T, typename U>
adv_string_view<T> new_string_view(const U *b, const EncMetric<typename T::ctype> *f, size_t maxsiz, const terminate_func<T> &t = zero_terminating<T>){
        return adv_string_view{new_const_pt<T>(b, f), maxsiz, t};
}

template<widenc T, typename U>
adv_string_view<T> new_string_view(const U *b, const EncMetric<typename T::ctype> *f, size_t siz, size_t len){
        return adv_string_view{new_const_pt<T>(b, f), siz, len};
}

template<typename T, typename S>
bool sameEnc(const adv_string_view<T> &a, const adv_string_view<S> &b) noexcept{
	return sameEnc(a.begin(), b.begin());
}


template<general_enctype T>
class adv_string_buf{
	private:
		basic_ptr buffer;
		EncMetric_info<T> ei;
		size_t siz, len;
	public:
		adv_string_buf(EncMetric_info<T> f, std::pmr::memory_resource *alloc=std::pmr::get_default_resource()) : buffer{alloc}, ei{f}, siz{0}, len{0} {}
		adv_string_buf(EncMetric_info<T> f, size_t siz, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()) : buffer{siz, alloc}, ei{f}, siz{0}, len{0} {}
		adv_string_buf(const adv_string_view<T> &str, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()) : adv_string_buf{str.begin().raw_format(), str.size(), alloc} {
			append_string(str);
		}

		size_t size() const noexcept { return siz;}
		size_t length() const noexcept {return len;}
		const byte *raw() {return buffer.memory;}

		/*
         * Only if encoding is the same
         */
		template<general_enctype S>
		size_t append_string(const adv_string_view<S> &);
        template<general_enctype S>
		adv_string_buf &operator<<(const adv_string_view<S> &p){
			append_string(p);
			return *this;
		}

		/*
		    Validate the character(s) before adding to buffer
		*/
		bool append_chr_v(const_tchar_pt<T>, size_t siz);
		bool append_chrs_v(const_tchar_pt<T>, size_t siz, size_t nchr);

		/*
		    Convert the string before adding it
		*/
		template<general_enctype S>
		size_t append_string_c(const adv_string_view<S> &);

		void clear() noexcept;
		adv_string_view<T> view() const noexcept;
		adv_string<T> move();
		adv_string<T> allocate(std::pmr::memory_resource *all = std::pmr::get_default_resource()) const;
};

template<strong_enctype T>
adv_string_buf<T> new_str_buf(std::pmr::memory_resource *alloc = std::pmr::get_default_resource()){
    return adv_string_buf<T>{EncMetric_info<T>{}, alloc};
}
template<strong_enctype T>
adv_string_buf<T> new_str_buf(size_t siz, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()){
    return adv_string_buf<T>{EncMetric_info<T>{}, siz, alloc};
}

template<widenc T>
adv_string_buf<T> new_str_buf(const EncMetric<typename T::ctype> *format, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()){
    return adv_string_buf<T>{EncMetric_info<T>{format}, alloc};
}
template<widenc T>
adv_string_buf<T> new_str_buf(const EncMetric<typename T::ctype> *format, size_t siz, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()){
    return adv_string_buf<T>{EncMetric_info<T>{format}, siz, alloc};
}

template<typename T>
adv_string_buf<T> new_str_buf(adv_string_view<T> str, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()){
    return adv_string_buf<T>{str, alloc};
}


template<general_enctype T>
class adv_string : public adv_string_view<T>{
	private:
		basic_ptr bind;

		adv_string(const_tchar_pt<T>, size_t, size_t, basic_ptr);

		/*
			USE WITH EXTREME CARE
			init with memory pointed by data and ignore ptr, use it only to detect encoding
			ignore is ignored
		*/
		adv_string(const_tchar_pt<T> ptr, size_t len, size_t siz, basic_ptr data, int ignore);
	public:
		adv_string(const adv_string_view<T> &, std::pmr::memory_resource *alloc);
		adv_string(const adv_string<T> &me) : adv_string{static_cast<const adv_string_view<T> &>(me), me.get_allocator()} {}
		adv_string(adv_string &&st) noexcept =default;

		std::pmr::memory_resource *get_allocator() const noexcept{return bind.get_allocator();}
		std::size_t capacity() const noexcept{ return bind.dimension;}

	template<general_enctype S>
	friend class adv_string_view;
	template<general_enctype S>
	friend class adv_string_buf;
};


template<strong_enctype T, typename U>
adv_string<T> alloc_string(const U *b, size_t maxsiz, const terminate_func<T> &t, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()){
        return adv_string<T>{adv_string_view<T>{new_const_pt<T>(b), maxsiz, t}, alloc};
}

template<strong_enctype T, typename U>
adv_string<T> alloc_string(const U *b, size_t maxsiz, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()){
        return adv_string<T>{adv_string_view<T>{new_const_pt<T>(b), maxsiz, zero_terminating<T>}, alloc};
}

template<strong_enctype T, typename U>
adv_string<T> alloc_string(const U *b, size_t siz, size_t len, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()){
        return adv_string<T>{adv_string_view<T>{new_const_pt<T>(b), siz, len}, alloc};
}

template<widenc T, typename U>
adv_string<T> alloc_string(const U *b, const EncMetric<typename T::ctype> *f, size_t maxsiz, const terminate_func<T> &t, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()){
        return adv_string<T>{adv_string_view<T>{new_const_pt<T>(b, f), maxsiz, t}, alloc};
}

template<widenc T, typename U>
adv_string<T> alloc_string(const U *b, const EncMetric<typename T::ctype> *f, size_t maxsiz, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()){
        return adv_string<T>{adv_string_view<T>{new_const_pt<T>(b, f), maxsiz, zero_terminating<T>}, alloc};
}

template<widenc T, typename U>
adv_string<T> alloc_string(const U *b, const EncMetric<typename T::ctype> *f, size_t siz, size_t len, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()){
        return adv_string<T>{adv_string_view<T>{new_const_pt<T>(b, f), siz, len}, alloc};
}

template<general_enctype S, general_enctype T>
adv_string<S> operator+(const adv_string<S> &a, const adv_string<T> &b){
	return a.template concatenate<T>(b, a.get_allocator());
}

using wstr_view = adv_string_view<WIDE<unicode>>;

using wstr = adv_string<WIDE<unicode>>;


#include <strsuite/encmetric/enc_string.tpp>
}


