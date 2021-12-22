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
#include <strsuite/encmetric/dynstring.hpp>
#include <strsuite/io/enc_io_exc.hpp>

namespace sts{

template<typename T>
concept read_byte_stream = requires(T stream, byte * const b, const size_t bsiz){
    {stream.read(b, bsiz)}->std::convertible_to<size_t>;
};
template<typename T>
concept write_byte_stream = requires(T stream, const byte * const b, const size_t bsiz){
    {stream.write(b, bsiz)}->std::convertible_to<size_t>;
};
template<typename T>
concept byte_stream = read_byte_stream<T> && write_byte_stream<T>;

/*
 * A read/write byte stream can return lesser bytes than required, in order to force it you need to call these functions
 */

template<read_byte_stream T>
size_t force_byte_read(T &stream, byte *b, size_t siz){
    size_t ret = 0;
    size_t step = 0;
    while(siz > 0){
        step = stream.read(b, siz);
        b += step;
        siz -= step;
        ret += step;
    }
    return ret;
}

template<write_byte_stream T>
size_t force_byte_write(T &stream, const byte *b, size_t siz){
    size_t ret = 0;
    size_t step = 0;
    while(siz > 0){
        step = stream.write(b, siz);
        b += step;
        siz -= step;
        ret += step;
    }
    return ret;
}

template<typename T, typename S>
concept read_char_stream = general_enctype<S> && requires(T stream){
        {stream.raw_format()} noexcept->std::same_as<EncMetric_info<typename T::enc>>;
    }
    && requires(T stream, const tchar_pt<S> dat, const size_t siz){
        {stream.char_read(dat, siz)}->std::convertible_to<uint>;
        {stream.char_read_v(dat, siz)}->std::convertible_to<uint>;
        {stream.ghost_read(dat, siz)}->std::convertible_to<uint>;
        {stream.ghost_read_v(dat, siz)}->std::convertible_to<uint>;
    };

template<typename T, typename S>
concept write_char_stream = general_enctype<S> && requires(T stream){
        {stream.raw_format()} noexcept->std::same_as<EncMetric_info<typename T::enc>>;
        stream.flush();
    }
    && requires(T stream, const const_tchar_pt<S> dat, const tchar_pt<S> vdat, const adv_string_view<S> str, const size_t siz){
        {stream.char_write(dat, siz)}->std::convertible_to<uint>;
        {stream.char_write(vdat, siz)}->std::convertible_to<uint>;
        {stream.string_write(str)}->std::convertible_to<size_t>;
    };


template<general_enctype T>
class CharIStream{
    protected:
        virtual uint do_char_read(tchar_pt<T>, size_t)=0;
        virtual uint do_ghost_read(tchar_pt<T>, size_t)=0;
        virtual void do_close()=0;
        virtual EncMetric_info<T> do_encmetric() const noexcept=0;
    public:
        using ctype = typename T::ctype;
        using enc = T;
        virtual ~CharIStream() {}
        template<general_enctype S>
        uint char_read(tchar_pt<S> pt, size_t buf) {
            auto enc=do_encmetric();
            return do_char_read(inv_rebase_pointer(pt, enc), buf);
        }
        template<general_enctype S>
        uint char_read_v(tchar_pt<S> pt, size_t buf) {
            uint ret = char_read(pt, buf);
            if(!pt.validChar(ret))
                throw IOIncomplete{"Not a valid character"};
            return ret;
        }
        /*
         * Read single character but don't update stream state: next read will read the same character
         */
        template<general_enctype S>
        uint ghost_read(tchar_pt<S> pt, size_t buf) {
            auto enc=do_encmetric();
            //enc.assert_base_for(pt.raw_format());
            return do_ghost_read(inv_rebase_pointer(pt, enc), buf);
        }
        template<general_enctype S>
        uint ghost_read_v(tchar_pt<S> pt, size_t buf) {
            uint ret = ghost_read(pt, buf);
            if(!pt.validChar(ret))
                throw IOIncomplete{"Not a valid character"};
            return ret;
        }
        void close() {return do_close();}
        EncMetric_info<T> raw_format() const noexcept{ return do_encmetric();}
        const EncMetric<typename T::ctype> *format() const noexcept{ return raw_format().format();}
};

template<general_enctype T>
class CharOStream{
    protected:
        virtual uint do_char_write(const_tchar_pt<T>, size_t)=0;

        virtual size_t do_string_write(const adv_string_view<T> &str)=0;
        virtual void do_close()=0;
        virtual void do_flush()=0;
        virtual EncMetric_info<T> do_encmetric() const noexcept=0;
    public:
        using ctype = typename T::ctype;
        using enc = T;
        virtual ~CharOStream() {}
        template<general_enctype S>
        uint char_write(const_tchar_pt<S> pt, size_t buf) {
            auto enc = do_encmetric();
            return do_char_write(rebase_pointer(pt, enc), buf);
        }
        template<general_enctype S>
        uint char_write(tchar_pt<S> pt, size_t buf) {
            return char_write(pt.cast(), buf);
        }

        template<general_enctype S>
        size_t string_write(const adv_string_view<S> &str) {return do_string_write(str.rebase(do_encmetric()));}
        void close() {return do_close();}
        void flush() {return do_flush();}
        EncMetric_info<T> raw_format() const noexcept{ return do_encmetric();}
        const EncMetric<typename T::ctype> *format() const noexcept{ return raw_format().format();}
};

}



