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
#include <strsuite/encmetric/enc_string.hpp>
#include <strsuite/encmetric/raw_buffer.hpp>

namespace sts{

template<general_enctype T>
class adv_string : public adv_string_view<T>{
	private:
		basic_ptr bind;

		adv_string(const_tchar_pt<T>, size_t, size_t, basic_ptr);

		/*
			USE WITH EXTREME CARE
		*/
		adv_string(EncMetric_info<T>, size_t len, size_t siz, basic_ptr data);
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
    friend adv_string<T> direct_build_dyn<T>(basic_ptr, size_t , size_t, EncMetric_info<T>);
};

template<general_enctype T>
adv_string<T> direct_build_dyn(basic_ptr data, size_t len, size_t siz, EncMetric_info<T> enc){
    return adv_string<T>{enc, len, siz, std::move(data)};
}

template<general_enctype T>
class adv_string_buf : private raw_buf{
	private:
		EncMetric_info<T> ei;
	public:
		adv_string_buf(EncMetric_info<T> f, std::pmr::memory_resource *alloc=std::pmr::get_default_resource()) : raw_buf{alloc}, ei{f} {}
		adv_string_buf(EncMetric_info<T> f, size_t dim, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()) : raw_buf{dim, alloc}, ei{f} {}

		adv_string_buf(std::pmr::memory_resource *alloc=std::pmr::get_default_resource()) requires strong_enctype<T> : adv_string_buf{EncMetric_info<T>{}, alloc} {}
		adv_string_buf(size_t dim, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()) requires strong_enctype<T> : adv_string_buf{EncMetric_info<T>{}, dim, alloc} {}

		adv_string_buf(const EncMetric<typename T::ctype> *f, std::pmr::memory_resource *alloc=std::pmr::get_default_resource()) requires widenc<T> : adv_string_buf{EncMetric_info<T>{f}, alloc} {}
		adv_string_buf(const EncMetric<typename T::ctype> *f, size_t dim, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()) requires widenc<T> : adv_string_buf{EncMetric_info<T>{f}, dim, alloc} {}


		adv_string_buf(const adv_string_view<T> &str, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()) : adv_string_buf{str.begin().raw_format(), str.size(), alloc} {
			append_string(str);
		}

		size_t size() const noexcept { return siz;}
		size_t length() const noexcept {return len;}
		size_t capacity() const noexcept {return raw_capacity();}
		const byte *raw() {return raw_first();}

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

//---------------------
/*
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
*/
//------------------------

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

using wstr = adv_string<WIDEchr>;
using wstr_buf = adv_string_buf<WIDEchr>;

#include <strsuite/encmetric/stringbuild.tpp>
}


