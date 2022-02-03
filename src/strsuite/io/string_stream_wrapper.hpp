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
    namespace details{
        template<typename U>
        struct is_stringstream_t : public std::false_type{};

        template<typename T>
        struct is_stringstream_t<string_stream<T>> : public std::true_type{};
    }

    /*
     * Wrapper class that allows reading and/or writing ctypes from/to a char stream
     */

    template<typename Stream, bool r, bool w>
    class string_stream_wrapper{
    private:
        using enc = typename Stream::enc;
        Stream &stream;
        string_stream<enc> buf;
    public:
        static_assert(!details::is_stringstream_t<Stream>::value, "Template deduction failure");
        using ctype = typename Stream::ctype;

        string_stream_wrapper(Stream &s, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()) : stream{s}, buf{s.raw_format(), alloc} {}

        ctype ctype_read() requires r{
            if(buf.length() == 0)
                buf.get_char(stream);
            return buf.ctype_read();
        }
        ctype ctype_read_ghost() requires r{
            if(buf.length() == 0){
                buf.get_ghost(stream);
                return buf.ctype_read();
            }
            else{
                return buf.ctype_read_ghost();
            }
        }
        void dis_char() requires r{
            if(buf.length() == 0){
                buf.get_char(stream);
            }
            buf.dis_char();
        }
        uint ctype_write(const ctype &chr) requires w{
            uint red = buf.ctype_write(chr);
            buf.put_all(stream);
            return red;
        }
    };

    template<typename T, bool r, bool w>
    class string_stream_wrapper<string_stream<T>, r, w>{
    private:
        string_stream<T> &buf;
    public:
        using ctype = typename T::ctype;

        string_stream_wrapper(string_stream<T> &s, std::pmr::memory_resource * = std::pmr::get_default_resource()) : buf{s} {}

        ctype ctype_read() requires r{
            return buf.ctype_read();
        }
        ctype ctype_read_ghost() requires r{
            return buf.ctype_read_ghost();
        }
        void dis_char() requires r{
            buf.dis_char();
        }
        uint ctype_write(const ctype &chr) requires w{
            return buf.ctype_write(chr);
        }
    };

}
