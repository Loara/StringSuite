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
#include <strsuite/io/buffers.hpp>

namespace sts{
    template<general_enctype T>
    class string_stream : protected basic_buffer<string_stream<T>, true, true>, public virtual CharIStream<T>, public virtual CharOStream<T>{
    private:
        basic_ptr buffer;
        size_t len;
        EncMetric_info<T> format;
    public:
        using ctype=typename T::ctype;
        friend class basic_buffer<string_stream<T>, true, true>;

        string_stream(EncMetric_info<T> info, std::pmr::memory_resource *alloc) : basic_buffer<string_stream<T>, true, true>{nullptr, 0}, buffer{alloc}, len{0}, format{info} {
            this->rebase(buffer.memory, buffer.dimension);
        }

        string_stream(std::pmr::memory_resource *alloc = std::pmr::get_default_resource()) requires strong_enctype<T> : string_stream{EncMetric_info<T>{}, alloc} {}

        string_stream(const EncMetric<typename T::ctype> *f, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()) requires widenc<T> : string_stream{EncMetric_info<T>{f}, alloc} {}

        size_t size() const noexcept {return this->siz;}
        size_t length() const noexcept {return len;}
        size_t remaining() const noexcept {return this->rem;}

        adv_string_view<T> view() const noexcept {return direct_build(this->get_fir_as(format), len, this->siz);}
        adv_string<T> move();
        adv_string<T> allocate_new(std::pmr::memory_resource *res) const;
        adv_string<T> allocate_new() const {return allocate_new(buffer.get_allocator());}

        template<general_enctype S, read_char_stream<S> IStream>
        uint get_char(IStream &);
        template<general_enctype S, read_char_stream<S> IStream>
        uint get_ghost(IStream &);
        template<general_enctype S, write_char_stream<S> OStream>
        uint put_char(OStream &);
        template<general_enctype S, write_char_stream<S> OStream>
        size_t put_all(OStream &);


        //Make encoding conversion
        template<general_enctype R>
        uint char_write_conv(const_tchar_pt<R>, size_t);
        template<general_enctype R>
        size_t string_write_conv(const adv_string_view<R> &);

        uint char_write(const ctype &);
    protected:
        void inc_siz(uint);
        void inc_rem(size_t);
        EncMetric_info<T> do_encmetric() const noexcept{ return format;}
        uint do_char_read(tchar_pt<T>, size_t);
        uint do_ghost_read(tchar_pt<T>, size_t);
        uint do_char_write(const_tchar_pt<T>, size_t);
        size_t do_string_write(const adv_string_view<T> &);
        void do_close() {}
        void do_flush() {}
    };
#include <strsuite/io/string_stream.tpp>
}
