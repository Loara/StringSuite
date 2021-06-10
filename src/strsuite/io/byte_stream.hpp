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

class ByteIStream{
    protected:
        virtual size_t do_read(byte *, size_t)=0;
        virtual void do_close()=0;
        virtual void do_flush()=0;
    public:
        virtual ~ByteIStream() {}
        size_t read(byte *b, size_t s){return do_read(b, s);}
        void read_exactly(byte *b, size_t s){
            size_t r = read(b, s);
            if(r != s)
                throw IOIncomplete{};
        }
        void close(){ return do_close();}
        void flush(){return do_flush();}
};

class ByteOStream{
    protected:
        virtual size_t do_write(const byte *, size_t)=0;
        virtual void do_close()=0;
        virtual void do_flush()=0;
    public:
        virtual ~ByteOStream() {}
        size_t write(const byte *b, size_t s){return do_write(b, s);}
        void write_exactly(const byte *b, size_t s){
            size_t r = write(b, s);
            if(r != s)
                throw IOIncomplete{};
        }
        void close(){ return do_close();}
        void flush(){return do_flush();}
};

}



