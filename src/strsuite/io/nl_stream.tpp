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
index_result NewlineIStream<T>::do_is_endl(const byte *b, size_t siz)const noexcept{
    auto nl = do_newline();
    if(siz < nl.size())
        return index_result{false, 0};
    const byte *last = b + (siz - nl.size());
    return index_result{std::memcmp(last, nl.data(), nl.size()) == 0, nl.size()};
}

template<general_enctype T>
adv_string<T> NewlineIStream<T>::do_getline(std::pmr::memory_resource *all){
    string_stream stream{this->do_encmetric(), all};
    adv_string_view<T> nl = do_newline();
    bool endl=false;
    while(!endl){
        stream.get_char(*this);
        if(stream.view().endsWith(nl))
            endl=true;
    }
    return stream.move();
}
    /*
            basic_ptr ptda{all};
            size_t retsiz=0, retlen=0;
            tchar_pt<T> base{ptda.memory, this->raw_format()};
            tchar_relative<T> to{base};
            adv_string_view<T> nl = do_newline();
            bool endl = false;
            while(!endl){
                try{
                    uint chl = this->char_read(to.convert(), ptda.dimension - retsiz);
                    to.next(ptda.dimension - retsiz);
                    retsiz += chl;
                    retlen++;
                    endl = do_is_endl(ptda.memory, retsiz).success;
                }
                catch(IOBufsmall &bs){
                    ptda.exp_fit(ptda.dimension + bs.get_required_size());
                    base = base.new_instance(ptda.memory);
                }
            }
            return direct_build_dyn<T>(std::move(ptda), retlen, retsiz, this->raw_format());
        }
        */

template<general_enctype T>
index_result NewlineOStream<T>::do_is_endl(const byte *b, size_t siz)const noexcept{
    auto nl = do_newline();
    if(siz < nl.size())
        return index_result{false, 0};
    const byte *last = b + (siz - nl.size());
    return index_result{std::memcmp(last, nl.data(), nl.size()) == 0, nl.size()};
}

template<general_enctype T>
size_t NewlineOStream<T>::do_putnl(){
    auto nl = do_newline();
    return this->do_string_write(nl);
}


