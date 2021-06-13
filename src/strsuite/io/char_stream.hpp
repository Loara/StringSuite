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
        virtual void do_flush()=0;
        virtual EncMetric_info<T> do_encmetric() const noexcept=0;
    public:
        using ctype = typename T::ctype;
        virtual ~CharIStream() {}
        template<general_enctype S>
        uint char_read(tchar_pt<S> pt, size_t buf) {
            auto enc=do_encmetric();
            enc.assert_base_for(pt.raw_format());
            return do_char_read(tchar_pt<T>{pt.data(), enc}, buf);
        }
        /*
         * Read single character but don't update stream state: next read will read the same character
         */
        template<general_enctype S>
        uint ghost_read(tchar_pt<S> pt, size_t buf) {
            auto enc=do_encmetric();
            enc.assert_base_for(pt.raw_format());
            return do_ghost_read(tchar_pt<T>{pt.data(), enc}, buf);
        }
        void close() {return do_close();}
        void flush() {return do_flush();}
        EncMetric_info<T> raw_format() const noexcept{ return do_encmetric();}
        const EncMetric<typename T::ctype> *format() const noexcept{ return raw_format().format();}
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
        virtual size_t do_string_write(const adv_string_view<T> &str){
            return do_chars_write(str.begin(), str.size(), str.length());
        }
        virtual void do_close()=0;
        virtual void do_flush()=0;
        virtual EncMetric_info<T> do_encmetric() const noexcept=0;
    public:
        using ctype = typename T::ctype;
        virtual ~CharOStream() {}
        template<general_enctype S>
        uint char_write(const_tchar_pt<S> pt, size_t buf) {
            auto enc = do_encmetric();
            pt.raw_format().assert_base_for(enc);
            return do_char_write(const_tchar_pt<T>{pt.data(), enc}, buf);
        }
        template<general_enctype S>
        uint char_write(tchar_pt<S> pt, size_t buf) {
            return char_write(pt.cast(), buf);
        }
        template<general_enctype S>
        size_t chars_write(const_tchar_pt<S> pt, size_t buf, size_t nchr) {
            auto enc = do_encmetric();
            pt.raw_format().assert_base_for(enc);
            return do_chars_write(const_tchar_pt<T>{pt.data(), enc}, buf, nchr);
        }
        template<general_enctype S>
        size_t string_write(const adv_string_view<S> &str) {return do_string_write(str.rebase(do_encmetric()));}
        void close() {return do_close();}
        void flush() {return do_flush();}
        EncMetric_info<T> raw_format() const noexcept{ return do_encmetric();}
        const EncMetric<typename T::ctype> *format() const noexcept{ return raw_format().format();}
};

template<general_enctype T>
class NewlineIStream : public CharIStream<T>{
    protected:
        virtual adv_string_view<T> do_newline() const noexcept=0;
        virtual index_result do_is_endl(const byte *b, size_t siz)const noexcept{
            auto nl = do_newline();
            if(siz < nl.size())
                return index_result{false, 0};
            const byte *last = b + (siz - nl.size());
            return index_result{std::memcmp(last, nl.data(), nl.size()) == 0, nl.size()};
        }
        virtual adv_string<T> do_getline(std::pmr::memory_resource *all){
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
    public:
        adv_string_view<T> newline() const noexcept{return do_newline();}
        index_result is_endl(const byte *b, size_t siz)const noexcept{ return do_is_endl(b, siz);}
        adv_string<T> getline(std::pmr::memory_resource *all=std::pmr::get_default_resource()){ return do_getline(all);}
};

template<general_enctype T>
class NewlineOStream : public CharOStream<T>{
    protected:
        virtual adv_string_view<T> do_newline() const noexcept=0;
        virtual index_result do_is_endl(const byte *b, size_t siz)const noexcept{
            auto nl = do_newline();
            if(siz < nl.size())
                return index_result{false, 0};
            const byte *last = b + (siz - nl.size());
            return index_result{std::memcmp(last, nl.data(), nl.size()) == 0, nl.size()};
        }
        virtual size_t do_putnl(){
            auto nl = do_newline();
            return this->do_string_write(nl);
        }
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

/*
 * Threat bytestreams as char streams


template<general_enctype T>
class C_B_IStream : public CharIStream<T>{
    private:
        raw_buf c_buffer;
        ByteIStream *input;
        EncMetric_info<T> enc;
    protected:
        virtual uint do_char_read(tchar_pt<T> pt, size_t buf);
        virtual void do_close() {input->close();}
        virtual void do_flush() {
            input->flush();
            c_buffer.raw_clear();
        }
        virtual EncMetric_info<T> do_encmetric() const noexcept {return enc;}
    public:
        C_B_IStream(ByteIStream *in, EncMetric_info<T> e, std::pmr::memory_resource *all = std::pmr::get_default_resource()) : c_buffer{all}, input{in}, enc{e} {}

        C_B_IStream(ByteIStream *in, std::pmr::memory_resource *all = std::pmr::get_default_resource()) requires strong_enctype<T> : C_B_IStream{in, EncMetric_info<T>{}, all} {}
        C_B_IStream(ByteIStream *in, const EncMetric<typename T::ctype> *e, std::pmr::memory_resource *all = std::pmr::get_default_resource()) requires widenc<T> : C_B_IStream{in, EncMetric_info<T>{e}, all} {}
};

template<general_enctype T>
class C_B_OStream : public CharOStream<T>{
    protected:
        virtual uint do_char_write(const_tchar_pt<T>, size_t);
        virtual void do_close() {input->close();}
        virtual void do_flush() {input->flush();}
        virtual EncMetric_info<T> do_encmetric() const noexcept {return enc;}
};
*/
#include <strsuite/io/char_stream.tpp>
}



