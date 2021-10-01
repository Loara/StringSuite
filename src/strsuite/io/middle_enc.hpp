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

#include <strsuite/io/string_stream.hpp>

namespace sts{

template<typename T>
class c_buffer;

template<general_enctype T> requires feat::has_max<T>::value
class c_buffer<T>{
private:
    byte mem[feat::has_max<T>::get_max()];
    tchar_pt<T> data;
public:
    c_buffer(EncMetric_info<T>, [[maybe_unused]] std::pmr::memory_resource * r=std::pmr::get_default_resource()) : data{mem} {}
    template<general_enctype S>
    void char_write(CharOStream<S> &out, const typename T::ctype &ev){
        data.encode(ev, feat::has_max<T>::get_max());
        out.char_write(data, feat::has_max<T>::get_max());
    }
};

template<general_enctype T> requires (! feat::has_max<T>::value)
class c_buffer<T>{
private:
    string_stream<T> data;
public:
    c_buffer(EncMetric_info<T> i, std::pmr::memory_resource *res=std::pmr::get_default_resource()) : data{i, res} {}
    template<general_enctype S>
    void char_write(CharOStream<S> &out, const typename T::ctype &ev){
        data.char_write(ev);
        data.put_char(out);
    }
};


}
