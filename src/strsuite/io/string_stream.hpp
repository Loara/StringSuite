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
#include <strsuite/encmetric/basic_ptr.hpp>
#include <strsuite/encmetric/enc_string.hpp>
#include <strsuite/io/char_stream.hpp>

namespace sts{
    template<general_enctype T>
    class string_stream : public virtual CharIStream<T>, public virtual CharOStream<T>{
    private:
        basic_ptr buffer;
        tchar_pt<T> base;
        tchar_relative<T> fir, las;
        size_t siz, len;
        void rewind();
        void increase(uint);
    public:
        string_stream(EncMetric_info<T> info, std::pmr::memory_resource *alloc) : buffer{alloc}, base{buffer.memory}, fir{base}, las{base}, siz{0}, len{0} {}

        string_stream(std::pmr::memory_resource *alloc = std::pmr::get_default_resource()) requires strong_enctype<T> : string_stream{EncMetric_info<T>{}, alloc} {}

        string_stream(const EncMetric<typename T::ctype> *f, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()) requires widenc<T> : string_stream{EncMetric_info<T>{f}, alloc} {}

        size_t size() const noexcept {return siz;}
        size_t length() const noexcept {return len;}
        size_t remaining() const noexcept {return buffer.dimension - las.difff();}

        adv_string_view<T> view() const noexcept {return direct_build(fir.convert().cast(), len, siz);}
        adv_string<T> move();
        adv_string<T> allocate_new(std::pmr::memory_resource *res) const;
        adv_string<T> allocate_new() const {return allocate_new(buffer.get_allocator());}

        template<general_enctype S>
        uint get_char(CharIStream<S> &);
        template<general_enctype S>
        uint get_ghost(CharIStream<S> &);
        template<general_enctype S>
        uint put_char(CharOStream<S> &);
        template<general_enctype S>
        size_t put_chars(CharOStream<S> *, size_t);

    protected:
        EncMetric_info<T> do_encmetric() const noexcept{ return base.raw_format();}
        uint do_char_read(tchar_pt<T>, size_t);
        uint do_ghost_read(tchar_pt<T>, size_t);
        uint do_char_write(const_tchar_pt<T>, size_t);
        size_t do_string_write(const adv_string_view<T> &);
        void do_close() {}
        void do_flush() {}
    };
#include <strsuite/io/string_stream.tpp>
}
