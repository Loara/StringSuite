
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

template<strong_enctype S, general_enctype T>
bool sameEnc(const const_tchar_pt<T> &a) noexcept{
	if constexpr(not_widenc<T>)
		return same_enc<S, T>;
	else{
		const EncMetric<typename T::ctype> &f = a.format();
		return f.index() == index_traits<S>::index();
	}
}

template<general_enctype S, general_enctype T>
bool sameEnc(const const_tchar_pt<S> &pa, const const_tchar_pt<T> &pb) noexcept{
	if constexpr(widenc<S>){
        if constexpr(widenc<T>){
            auto fa = pa.format();
            auto fb = pb.format();
            return fa.index() == fb.index();
        }
        else
            return sameEnc<T>(pa);
    }
    else{
        return sameEnc<S>(pb);
    }
}

template<typename S, general_enctype T>
bool can_reassign(const_tchar_pt<T> ptr) noexcept{
    bool b = weak_assign<T, S>;
    if(!b)
        return false;
    if constexpr(widenc<T>){
        if constexpr(enc_raw<S> || (widenc<S> && same_data<S, T>))
            return true;
        else if constexpr(not_widenc<S>)
            return sameEnc<S>(ptr);
        else return false;
    }
    else return true;
}

template<general_enctype S, general_enctype T>
tchar_pt<S> reassign(tchar_pt<T> p){
    if constexpr (widenc<S>){
        static_assert(same_data<S, T>, "Not same data encodings");
        return tchar_pt<S>{p.data(), p.format()};
    }
    else{
        if(!can_reassign<S>(p))
            throw encoding_error{"Cannot convert these strings"};
        return tchar_pt<S>{p.data()};
    }
}

template<general_enctype S, general_enctype T>
const_tchar_pt<S> reassign(const_tchar_pt<T> p){
    if constexpr (widenc<S>){
        static_assert(same_data<S, T>, "Not same data encodings");
        return const_tchar_pt<S>{p.data(), p.format()};
    }
    else{
        if(!can_reassign<S>(p))
            throw encoding_error{"Cannot convert these strings"};
        return const_tchar_pt<S>{p.data()};
    }
}


template<general_enctype S, general_enctype T>  requires same_data<S, T>
void basic_encoding_conversion(const_tchar_pt<T> in, uint inlen, tchar_pt<S> out, uint oulen){
	typename S::ctype bias;
	in.decode(&bias, inlen);
	out.encode(bias, oulen);
}

template<general_enctype S, general_enctype T>  requires same_data<S, T>
void basic_encoding_conversion(const_tchar_pt<T> in, uint inlen, tchar_pt<S> out, uint oulen, uint &inread, uint &outread){
	typename S::ctype bias;
	inread = in.decode(&bias, inlen);
	outread = out.encode(bias, oulen);
}

template<general_enctype T>
uint min_size_estimate(const_tchar_pt<T> ptr, uint nchr) noexcept{
	if constexpr(not_widenc<T>)
		return min_length<T>(nchr);
	else
		return min_length(nchr, ptr.format());
}
template<general_enctype T>
uint max_size_estimate(const_tchar_pt<T> ptr, uint nchr){
	if constexpr(not_widenc<T>)
		return max_length<T>(nchr);
	else
		return max_length(nchr, ptr.format());
}

template<general_enctype T>
bool dynamic_fixed_size(const_tchar_pt<T> ptr) noexcept{
	if constexpr(widenc<T>)
		return ptr.format().d_fixed_size();
	else
		return fixed_size<T>;
}



