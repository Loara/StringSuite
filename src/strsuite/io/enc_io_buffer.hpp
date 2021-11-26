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
#include <type_traits>
#include <strsuite/encmetric/base.hpp>
#include <strsuite/encmetric/byte_tools.hpp>
#include <strsuite/io/buffers.hpp>
#include <cstring>

namespace sts{

template<typename Sys>
concept isyscall = requires(Sys s, byte *a, size_t t){
    {s.read_wrap(a, t)} -> std::same_as<size_t>;
};

template<typename Sys>
concept osyscall = requires(Sys s,  const byte *b, size_t t){
    {s.write_wrap(b, t)} -> std::same_as<size_t>;
};

template<isyscall Sys, size_t bufsiz>
class istr_buffer : private basic_buffer<istr_buffer<Sys, bufsiz>, true, false>{
    private:
        Sys sy;
        byte buf[bufsiz];

        void inc_siz(uint){

            size_t wt = sy.read_wrap(this->base + this->las, this->rem);
            if(wt > 0){
                this->raw_las_step(wt);
            }
        }

    public:
        istr_buffer(const Sys &f) : basic_buffer<istr_buffer<Sys, bufsiz>, true, false>{buf, bufsiz}, sy{f} {}
        istr_buffer(Sys &&f) : basic_buffer<istr_buffer<Sys, bufsiz>, true, false>{buf, bufsiz}, sy{std::move(f)} {}

        Sys get_system_id() const noexcept {return sy;}

        Sys &system_ref() noexcept {return sy;}

        size_t read(byte *by, size_t stm){
            if(stm == 0)
                return 0;
            size_t inc = this->req_sizle(stm);
            copyN(this->base + this->fir, by, inc);
            this->raw_fir_step(inc);
            return inc + read(by + inc, stm - inc);
        }

        friend class basic_buffer<istr_buffer<Sys, bufsiz>, true, false>;
};

template<osyscall Sys, size_t bufsiz>
class ostr_buffer : private basic_buffer<ostr_buffer<Sys, bufsiz>, false, true>{
    private:
        Sys sy;
        byte buf[bufsiz];

        void inc_rem(size_t){
            size_t wt;
            bool repeat;
            do{
                repeat=false;
                wt = sy.write_wrap(this->base + this->fir, this->siz);
                if(wt > 0){
                    this->raw_fir_step(wt);
                }
                else
                    repeat=true;
            }
            while(repeat);
            this->rewind();//increases rem by n
        }

    public:
        ostr_buffer(const Sys &f) : basic_buffer<ostr_buffer<Sys, bufsiz>, false, true>{buf, Sys::buffer_size}, sy{f} {}
        ostr_buffer(Sys &&f) : basic_buffer<ostr_buffer<Sys, bufsiz>, false, true>{buf, Sys::buffer_size}, sy{std::move(f)} {}

        size_t write(const byte *by, size_t stm){
            if(stm == 0)
                return 0;
            size_t mi = this->req_frspc(stm);
            copyN(by, this->base + this->las, mi);
            this->raw_las_step(mi);
            return mi + write(by + mi, stm - mi);
        }

        void flush(){
            while(this->siz > 0){
                size_t wt = sy.write_wrap(this->base + this->fir, this->siz);
                if(wt > 0){
                    this->raw_fir_step(wt);
                }
            }
            this->base_flush();
        }

        Sys get_system_id() const noexcept {return sy;}
        Sys &system_ref() noexcept {return sy;}

        friend class basic_buffer<ostr_buffer<Sys, bufsiz>, false, true>;
};

}
