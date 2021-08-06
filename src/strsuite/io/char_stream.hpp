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
#include <strsuite/encmetric/raw_buffer.hpp>
#include <strsuite/io/enc_io_exc.hpp>

namespace sts{

template<general_enctype T>
class CharIStream{
    protected:
        virtual uint do_char_read(tchar_pt<T>, size_t)=0;
        virtual uint do_ghost_read(tchar_pt<T>, size_t)=0;
        virtual void do_close()=0;
        //virtual void do_flush()=0;
        virtual EncMetric_info<T> do_encmetric() const noexcept=0;
    public:
        using ctype = typename T::ctype;
        virtual ~CharIStream() {}
        template<general_enctype S>
        uint char_read(tchar_pt<S> pt, size_t buf) {
            auto enc=do_encmetric();
            //enc.assert_base_for(pt.raw_format());
            //return do_char_read(tchar_pt<T>{pt.data(), enc}, buf);
            return do_char_read(inv_rebase_pointer(pt, enc), buf);
        }
        template<general_enctype S>
        uint char_read_v(tchar_pt<S> pt, size_t buf) {
            /*
            auto enc=do_encmetric();
            enc.assert_base_for(pt.raw_format());
            uint ret = do_char_read(tchar_pt<T>{pt.data(), enc}, buf);
            */
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
            /*
            auto enc=do_encmetric();
            enc.assert_base_for(pt.raw_format());
            uint ret = do_ghost_read(tchar_pt<T>{pt.data(), enc}, buf);
            */
            uint ret = ghost_read(pt, buf);
            if(!pt.validChar(ret))
                throw IOIncomplete{"Not a valid character"};
            return ret;
        }
        void close() {return do_close();}
        //void flush() {return do_flush();}
        EncMetric_info<T> raw_format() const noexcept{ return do_encmetric();}
        const EncMetric<typename T::ctype> *format() const noexcept{ return raw_format().format();}
};

template<general_enctype T>
class CharOStream{
    protected:
        virtual uint do_char_write(const_tchar_pt<T>, size_t)=0;
        /*
        virtual size_t do_chars_write(const_tchar_pt<T> pt, size_t buf, size_t nchr){
            if(nchr==0)
                return 0;
            size_t ret=0;
            uint wrt=0;
            for(size_t i=0; i<nchr; i++){
                wrt = do_char_write(pt, buf);
                ret += wrt;
                buf-= wrt;
                pt += wrt;
            }
            return ret;
        }
        */
        virtual size_t do_string_write(const adv_string_view<T> &str)=0;
        virtual void do_close()=0;
        virtual void do_flush()=0;
        virtual EncMetric_info<T> do_encmetric() const noexcept=0;
    public:
        using ctype = typename T::ctype;
        virtual ~CharOStream() {}
        template<general_enctype S>
        uint char_write(const_tchar_pt<S> pt, size_t buf) {
            auto enc = do_encmetric();
            /*
            pt.raw_format().assert_base_for(enc);
            return do_char_write(const_tchar_pt<T>{pt.data(), enc}, buf);
            */
            return do_char_write(rebase_pointer(pt, enc), buf);
        }
        template<general_enctype S>
        uint char_write(tchar_pt<S> pt, size_t buf) {
            return char_write(pt.cast(), buf);
        }
        /*
        template<general_enctype S>
        size_t chars_write(const_tchar_pt<S> pt, size_t buf, size_t nchr) {
            auto enc = do_encmetric();
            pt.raw_format().assert_base_for(enc);
            return do_chars_write(const_tchar_pt<T>{pt.data(), enc}, buf, nchr);
        }
        */
        template<general_enctype S>
        size_t string_write(const adv_string_view<S> &str) {return do_string_write(str.rebase(do_encmetric()));}
        void close() {return do_close();}
        void flush() {return do_flush();}
        EncMetric_info<T> raw_format() const noexcept{ return do_encmetric();}
        const EncMetric<typename T::ctype> *format() const noexcept{ return raw_format().format();}
};

}



