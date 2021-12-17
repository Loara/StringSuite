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
#include <strsuite/encmetric/byte_tools.hpp>
#include <strsuite/encmetric/config.hpp>
#include <strsuite/encmetric/all_enc.hpp>
#include <strsuite/io/enc_io_exc.hpp>
#include <strsuite/io/enc_io_buffer.hpp>
#include <strsuite/io/nl_stream.hpp>

namespace sts{

//System encoding for IO operations
using IOenc = std::conditional_t<is_windows(), UTF16LE, UTF8>;

size_t def_strlen(const byte *);
adv_string_view<IOenc> console_endline();

class InputStream : public NewlineIStream<IOenc>{
protected:
    virtual void do_discard()=0;
public:
    void discard(){do_discard();}
};
class OutputStream : public NewlineOStream<IOenc>{};

InputStream &get_console_stdin();
OutputStream &get_console_stdout();
OutputStream &get_console_stderr();
/*
 * Decode program args
 */

class param_helper{
    private:
        int argn;
        const byte **args;
    public:
        param_helper(int, const char **);
        ~param_helper();
        int npar() const noexcept {return argn;}
        const byte *get(int i) const noexcept {return args[i];}
};

/*
 * Base implementations of Console streams
 */

template<typename Sys>
class Console_istream : public InputStream{
    private:
        istr_buffer<Sys, 60> buffer;
        string_stream<IOenc> conv;
    protected:
        adv_string_view<IOenc> do_newline() const noexcept{ return buffer.get_system_id().do_newline();}
        uint do_char_read(tchar_pt<IOenc> pt, size_t siz){
            if(conv.length() == 0)
                conv.get_char_bytes(buffer, false);
            return conv.char_read(pt, siz);
        }

        uint do_ghost_read(tchar_pt<IOenc> pt, size_t siz){
            if(conv.length() == 0)
                conv.get_char_bytes(buffer, false);
            return conv.ghost_read(pt, siz);
        }

        void do_close() {}

        void do_discard(){
            conv.discard();
            buffer.discard_buffer();
            buffer.get_system_id().discard_all_input();
        }

        EncMetric_info<IOenc> do_encmetric() const noexcept{ return EncMetric_info<IOenc>{};}
    public:
        Console_istream(const Sys &s) : buffer{s} {}
};

template<typename Sys>
class Console_ostream : public OutputStream{
    private:
        ostr_buffer<Sys, 60> buffer;
        string_stream<IOenc> conv;
    protected:
        adv_string_view<IOenc> do_newline() const noexcept{ return buffer.get_system_id().do_newline();}
        uint do_char_write(const_tchar_pt<IOenc> pt, size_t siz){
            uint ret = conv.char_write(pt, siz);
            while(conv.length() > 0)
                conv.put_char_bytes(buffer);
            return ret;
        }
        size_t do_string_write(const adv_string_view<IOenc> &str){
            size_t ret = conv.string_write(str);
            conv.put_all_char_bytes(buffer);
            return ret;
        }

        void do_close() {}

        void do_flush(){
            conv.put_all_char_bytes(buffer);
            buffer.flush();
        }

        EncMetric_info<IOenc> do_encmetric() const noexcept{ return EncMetric_info<IOenc>{};}
    public:
        Console_ostream (const Sys &s) : buffer{s} {}
};


}



