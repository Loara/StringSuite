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
#include <exception>

namespace sts{

class IOException : public std::exception{
    private:
        const char *m;
    public:
        IOException(const char *mu) : std::exception{}, m{mu} {}
        IOException() : IOException{"IO error"} {}
        const char *what() const noexcept {return m;}
};

class IOInfo : public IOException{
    public:
        IOInfo(const char *mu) : IOException{mu} {}
        IOInfo() : IOException{"IO special character"} {}
};

class IOFail : public IOException{
    public:
        IOFail(const char *mu) : IOException{mu} {}
        IOFail() : IOException{"IO failure"} {}
};

class EOF : public IOInfo{
    public:
        EOF() : IOInfo{"End of file"} {}
};

class EAGAIN : public IOInfo{
    public:
        EAGAIN() : IOInfo{"Nonblocking empty reading"} {}
};

class InvalidOP : public IOFail{
    public:
        InvalidOP() : IOFail{"Invalid operation for this stream"} {}
};

}



