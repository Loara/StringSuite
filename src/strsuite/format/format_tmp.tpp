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
adv_formatter<T, Formatter>::adv_formatter(U &&ff, EncMetric_info<T> i, std::pmr::memory_resource *alloc) : info{i}, f{std::forward<U>(ff)}, res{alloc} {}

template<general_enctype T, typename Formatter>
template<typename Arg, typename... Args>
void adv_formatter<T, Formatter>::dispatch_call(string_stream<T> &stream, size_t i, const adv_string_view<T> &s, Arg && a, Args &&... args){
    if(i == 0)
        return f.format(stream, std::forward<Arg>(a), s);
    else
        return dispatch_call(stream, i-1, s, std::forward<Args>(args)...);
}

template<general_enctype T, typename Formatter>
void adv_formatter<T, Formatter>::dispatch_call(string_stream<T> &, size_t , const adv_string_view<T> &){
    throw out_of_range{"Index is too big"};
}

template<general_enctype T, typename Formatter>
template<typename placeholder>
size_t adv_formatter<T, Formatter>::get_id(placeholder &place, const adv_string_view<T> &opt){
    return static_cast<size_t>(opt.get_char(place)) - static_cast<size_t>('0');
}

template<general_enctype T, typename Formatter>
template<general_enctype S, typename... Args>
adv_string<T> adv_formatter<T, Formatter>::format(const adv_string_view<S> &s, Args &&... args){
    adv_string_view<T> str = s.rebase(info);
    adv_string_view<T> empty{info};
    std::set<unicode> spaces{' '_uni, '\n'_uni, '\r'_uni, '\t'_uni};
    if(str.length() == 0)
        return adv_string<T>{empty, res};
    Token<T> token{str};
    string_stream<T> stream{info, res};
    while(!token.ended()){
        auto res = token.goUp_ctype('%'_uni);
        stream.string_write(token.share());
        token.flush();
        if(res.found()){
            token.step();
            token.goUntil_container(spaces);
            token.flush();
            res = token.goUp_ctype('%'_uni);
            if(res.found()){
                size_t red = 0;
                auto dec = token.subToken();
                read_integer(dec, red);
                dispatch_call(stream, res.idx, empty, args...);
                token.step();
            }
            token.flush();
        }
    }
    return stream.move();
}

