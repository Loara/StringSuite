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

namespace sts{

size_t def_strlen(const byte *);

size_t raw_stdin_readbytes(byte *, size_t);
size_t raw_stdout_writebytes(const byte *, size_t);
size_t raw_stderr_writebytes(const byte *, size_t);

size_t raw_newline();

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



