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

//size_t raw_newline();

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
}



