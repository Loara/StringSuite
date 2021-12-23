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

	//template<general_enctype S>
	//friend class adv_string_view;
    friend adv_string<T> direct_build_dyn<T>(basic_ptr, size_t , size_t, EncMetric_info<T>);
};

template<general_enctype T>
adv_string<T> direct_build_dyn(basic_ptr data, size_t len, size_t siz, EncMetric_info<T> enc){
    return adv_string<T>{enc, len, siz, std::move(data)};
}


//------------------------

template<strong_enctype T, typename U, typename FuncType>
adv_string<T> alloc_string(const U *b, size_t maxsiz, const FuncType &t, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()){
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

template<widenc T, typename U, typename FuncType>
adv_string<T> alloc_string(const U *b, const EncMetric<typename T::ctype> *f, size_t maxsiz, const FuncType &t, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()){
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

using wstr = adv_string<WIDEchr>;

#include <strsuite/encmetric/dynstring.tpp>
}


