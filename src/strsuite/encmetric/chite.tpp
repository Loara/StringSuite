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
template<general_enctype T>
uint tread_pt<T>::next(size_t siz){
    uint add = chLen(siz);
    if(add > siz)
        throw buffer_small{add - static_cast<uint>(siz)};
    this->ptr += add;
    return add;
}

template<general_enctype T>
uint tread_pt<T>::next_update(size_t &siz){
    uint skip = next(siz);
    siz -= skip;
    return skip;
}

template<general_enctype T>
validation_result tread_pt<T>::valid_next(size_t siz) noexcept{
    validation_result ret = validChar(siz);
    if(ret)
        this->ptr += ret.get();
    return ret;
}

template<general_enctype T>
validation_result tread_pt<T>::valid_next_update(size_t &siz) noexcept{
    validation_result ret = valid_next(siz);
    if(ret)
        siz -= ret.get();
    return ret;
}

template<general_enctype T>
auto tread_pt<T>::decode_next(size_t l) -> tuple_ret<ctype>{
    tuple_ret<ctype> ret = decode(l);
    this->ptr += get_len_el(ret);
    return ret;
}

template<general_enctype T>
auto tread_pt<T>::decode_next_update(size_t &l) -> tuple_ret<ctype>{
    tuple_ret<ctype> ret = decode_next(l);
    l -= get_len_el(ret);
    return ret;
}

template<general_enctype T>
uint twrite_pt<T>::encode_next(const ctype &uni, size_t l) {
    uint ret = encode(uni, l);
    this->ptr += ret;
    return ret;
}

template<general_enctype T>
uint twrite_pt<T>::encode_next_update(const ctype &uni, size_t &l) {
    uint ret = encode_next(uni, l);
    l -= ret;
    return ret;
}

template<general_enctype T>
auto tchar_relative<T>::decode_next(size_t l) -> tuple_ret<ctype> {
    tuple_ret<ctype> ret = decode(l);
    dif += get_len_el(ret);
    return ret;
}

template<general_enctype T>
uint tchar_relative<T>::encode_next(const ctype &uni, size_t l) {
    uint ret = encode(uni, l);
    dif += ret;
    return ret;
}

template<general_enctype T>
auto tchar_relative<T>::decode_next_update(size_t &l) -> tuple_ret<ctype>{
    tuple_ret<ctype> ret = decode_next(l);
    l -= get_len_el(ret);
    return ret;
}

template<general_enctype T>
uint tchar_relative<T>::encode_next_update(const ctype &uni, size_t &l) {
    uint ret = encode_next(uni, l);
    l -= ret;
    return ret;
}

template<general_enctype T>
uint tchar_relative<T>::next(size_t siz){
    uint add = raw_format().chLen(data(), siz);
    if(add > siz)
        throw buffer_small{add};
    dif += add;
    return add;
}

template<general_enctype T>
uint tchar_relative<T>::next_update(size_t &siz){
    uint ret = next(siz);
    siz -= ret;
    return ret;
}

//-----------------------------------------

template<general_enctype T>
uint min_size_estimate(tread_pt<T> ptr, uint nchr) noexcept{
	if constexpr(not_widenc<T>)
		return min_length<T>(nchr);
	else
		return min_length(nchr, ptr.format());
}
template<general_enctype T>
uint max_size_estimate(tread_pt<T> ptr, uint nchr){
	if constexpr(not_widenc<T>)
		return max_length<T>(nchr);
	else
		return max_length(nchr, ptr.format());
}



