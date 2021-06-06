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
#include <strsuite/encmetric/enc_string.hpp>
#include <strsuite/io/byte_stream.hpp>

namespace sts{

template<general_enctype T>
class CharIStream{
    protected:
        virtual uint do_char_read(tchar_pt<T>, size_t)=0;
        virtual size_t do_chars_read(tchar_pt<T> pt, size_t buf, size_t nchr){
            if(nchr==0)
                return 0;
            size_t ret=0;
            uint read=0;
            for(size_t i=0; i<nchr; i++){
                read = do_char_read(pt, buf);
                ret += read;
                buf-= read;
                pt += read;
            }
            return ret;
        }
        virtual void do_close()=0;
        virtual void do_flush()=0;
        virtual EncMetric_info<T> do_encmetric() const noexcept=0;
    public:
        using ctype = typename T::ctype;
        virtual ~CharIStream() {}
        uint char_read(tchar_pt<T> pt, size_t buf) {return do_char_read(pt, buf);}
        size_t chars_read(tchar_pt<T> pt, size_t buf, size_t nchr) {return do_chars_read(pt, buf, nchr);}
        void close() {return do_close();}
        void flush() {return do_flush();}
        EncMetric_info<T> raw_format() const noexcept{ return do_encmetric();}
        const Encmetric<typename T::ctype> *format() const noexcept{ return raw_format().format();}
};

template<general_enctype T>
class CharOStream{
    protected:
        virtual uint do_char_write(const_tchar_pt<T>, size_t)=0;
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
        virtual void do_close()=0;
        virtual void do_flush()=0;
        virtual EncMetric_info<T> do_encmetric() const noexcept=0;
    public:
        using ctype = typename T::ctype;
        virtual ~CharIStream() {}
        uint char_write(const_tchar_pt<T> pt, size_t buf) {return do_char_write(pt, buf);}
        size_t chars_write(const_tchar_pt<T> pt, size_t buf, size_t nchr) {return do_chars_write(pt, buf, nchr);}
        void close() {return do_close();}
        void flush() {return do_flush();}
        EncMetric_info<T> raw_format() const noexcept{ return do_encmetric();}
        const Encmetric<typename T::ctype> *format() const noexcept{ return raw_format().format();}
};

/*
 * Threat bytestreams as char streams
 */

template<general_enctype T>
class C_B_IStream : public CharIStream<T>{
    private:
        ByteIStream *input;
        EncMetric_info<T> enc;
    protected:
        virtual uint do_char_read(tchar_pt<T> pt, size_t buf)=0{
            if(buf < pt.min_bytes()){
                return 0;
            }
            byte *raw = pt.data();
            uint chl = 0;
            uint tmp = static_cast<uint>(input->read(raw, pt.min_bytes()));
            uint red = tmp;
            raw += tmp;
            bool ret = false;
            do{
                try{
                    chl = pt.chLen(red);
                    ret = true;
                }
                catch(buffer_small &e){
                    uint inc = e.get_required_size();
                    tmp = static_cast<uint>(input->read(raw, inc));
                    red += tmp;
                    raw += tmp;
                }
                if(ret && (chl > red)){
                    tmp = static_cast<uint>(input->read(raw, chl-red));
                    red += tmp;
                }
            }
            while(!ret);
            return red;
        }
        virtual void do_close() {input->close();}
        virtual void do_flush() {input->flush();}
        virtual EncMetric_info<T> do_encmetric() const noexcept {return enc;}
};

template<general_enctype T>
class CharOStream{
    protected:
        virtual uint do_char_write(const_tchar_pt<T>, size_t)=0;
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
        virtual void do_close()=0;
        virtual void do_flush()=0;
        virtual EncMetric_info<T> do_encmetric() const noexcept=0;
    public:
        virtual ~CharIStream() {}
        uint char_write(const_tchar_pt<T> pt, size_t buf) {return do_char_write(pt, buf);}
        size_t chars_write(const_tchar_pt<T> pt, size_t buf, size_t nchr) {return do_chars_write(pt, buf, nchr);}
        void close() {return do_close();}
        void flush() {return do_flush();}
        EncMetric_info<T> raw_format() const noexcept{ return do_encmetric();}
        const Encmetric<typename T::ctype> *format() const noexcept{ return raw_format().format();}
};


}



