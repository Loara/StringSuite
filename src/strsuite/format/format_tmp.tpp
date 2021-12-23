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

template<general_enctype T, typename Formatter>
template<typename U>
adv_formatter<T, Formatter>::adv_formatter(U &&ff, EncMetric_info<T> info, std::pmr::memory_resource *alloc) : f{std::forward<U>(ff)}, stream{info, alloc} {}

template<general_enctype T, typename Formatter>
template<typename Arg, typename... Args>
void adv_formatter<T, Formatter>::dispatch_call(size_t i, const adv_string_view<T> &s, Arg && a, Args &&... args){
    if(i == 0)
        return f.format(stream, std::forward<Arg>(a), s);
    else
        return dispatch_call(i-1, s, std::forward<Args>(args)...);
}

template<general_enctype T, typename Formatter>
void adv_formatter<T, Formatter>::dispatch_call(size_t , const adv_string_view<T> &){
    throw out_of_range{"Index is too big"};
}

template<general_enctype T, typename Formatter>
template<typename placeholder>
size_t adv_formatter<T, Formatter>::get_id(placeholder &place, const adv_string_view<T> &opt){
    return static_cast<size_t>(opt.get_char(place)) - static_cast<size_t>('0');
}

template<general_enctype T, typename Formatter>
template<typename... Args>
adv_string<T> adv_formatter<T, Formatter>::format(const adv_string_view<T> &str, Args &&... args){
    adv_string_view<T> empty{stream.raw_format()};
    if(str.length() == 0)
        return adv_string<T>{empty, std::pmr::get_default_resource()};
    const auto fine = str.select_end();
    auto mid = str.select_begin();
    bool param = false;
    size_t i = 0;
    auto place = str.select_begin();
    typename T::ctype chr;
    while(place < fine){
        chr = str.get_char(place);
        if(param){
            if(chr == '}'_uni){
                param = false;
                dispatch_call(i, str.substring(mid, place), std::forward<Args>(args)...);
                str.select_next(place);
                mid = place;
            }
        }
        else{
            if(chr == '{'_uni){
                stream.string_write(str.substring(mid, place));
                str.select_next(place);
                i = get_id(place, str);
                str.select_next(place);
                chr = str.get_char(place);
                if(chr == '}'_uni){
                    dispatch_call(i, empty, std::forward<Args>(args)...);
                }
                else if(chr == '|'_uni){
                    param = true;
                }
                str.select_next(place);
                mid = place;
            }
        }
        str.select_next_eof(place);
    }
    return stream.move();
}
