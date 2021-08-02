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

template<general_enctype T>
class NewlineIStream : public CharIStream<T>{
    protected:
        virtual adv_string_view<T> do_newline() const noexcept=0;
        virtual index_result do_is_endl(const byte *b, size_t siz)const noexcept;
        virtual adv_string<T> do_getline(std::pmr::memory_resource *all);
        virtual adv_string<T> do_get_line(std::pmr::memory_resource *all);
    public:
        adv_string_view<T> newline() const noexcept{return do_newline();}
        index_result is_endl(const byte *b, size_t siz)const noexcept{ return do_is_endl(b, siz);}
        adv_string<T> getline(std::pmr::memory_resource *all=std::pmr::get_default_resource()){ return do_getline(all);}
        /*
         * Same as getline, but removes newlines
         */
        adv_string<T> get_line(std::pmr::memory_resource *all=std::pmr::get_default_resource()){ return do_get_line(all);}
};

template<general_enctype T>
class NewlineOStream : public CharOStream<T>{
    protected:
        virtual adv_string_view<T> do_newline() const noexcept=0;
        virtual index_result do_is_endl(const byte *b, size_t siz)const noexcept;
        virtual size_t do_putnl();
    public:
        adv_string_view<T> newline() const noexcept{return do_newline();}
        index_result is_endl(const byte *b, size_t siz)const noexcept{ return do_is_endl(b, siz);}
        size_t putNL(){ return do_putnl();}
        size_t endl(){
            auto ret = this->do_putnl();
            this->do_flush();
            return ret;
        }
        template<general_enctype S>
        size_t print(const adv_string_view<S> &str){
            return this->string_write(str);
        }
        template<general_enctype S>
        size_t println(const adv_string_view<S> &str){
            size_t par = this->string_write(str);
            par += endl();
            return par;
        }
};

#include <strsuite/io/nl_stream.tpp>
}



