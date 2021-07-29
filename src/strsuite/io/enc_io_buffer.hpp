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
#include <strsuite/encmetric/base.hpp>
#include <strsuite/encmetric/byte_tools.hpp>
#include <strsuite/io/buffers.hpp>
#include <cstring>

namespace sts{
template<size_t t>
inline constexpr size_t tester = t;

template<typename Sys>
concept syscall = requires(Sys s, byte *a, const byte *b, size_t t){
    tester<Sys::buffer_size>;
    {s.read_wrap(a, t)} -> std::same_as<size_t>;
    {s.write_wrap(b, t)} -> std::same_as<size_t>;
};

template<strong_enctype T, syscall Sys>
class istr_buffer : public basic_buffer<T, istr_buffer<T, Sys>, true, false>{
    private:
        Sys sy;
        byte buf[Sys::buffer_size];
    public:
        istr_buffer(const Sys &f) : basic_buffer<T, istr_buffer<T, Sys>, true, false>{buf, Sys::buffer_size, EncMetric_info<T>{}}, sy{f} {}

        void inc_siz(uint inc){
            if(this->rem < static_cast<size_t>(inc))
                throw IOEOF{};
            size_t wt, tu = static_cast<size_t>(inc);
            bool repeat;
            do{
                repeat=false;
                wt = sy.read_wrap(this->las.data(), this->rem);
                if(wt > 0){
                    this->raw_las_step(wt);
                }
                if(wt < tu){
                    tu -= wt;
                    repeat = true;
                }
                else
                    tu = 0;
            }
            while(repeat);
        }
        Sys get_system_id() const noexcept {return sy;}
};

template<strong_enctype T, syscall Sys>
class ostr_buffer : public basic_buffer<T, ostr_buffer<T, Sys>, false, true>{
    private:
        Sys sy;
        byte buf[Sys::buffer_size];
        /*
        size_t pt1, pt2; // 0 < pt1 < pt2 < tmp < buffer_size
        void refresh(){
            pt1=0;
            pt2=0;
        }
        void push(){
            size_t wt;
            bool repeat;
            do{
                repeat=false;
                wt = sy.write_wrap(buf + pt1, pt2 - pt1, repeat);
            }while(repeat);
            pt1 += wt;
        }
        void shift(){
            std::memmove(buf, buf + pt1, pt2 - pt1);
            pt2 = pt2 - pt1;
            pt1 = 0;
        }
        */
    public:
        ostr_buffer(const Sys &f) : basic_buffer<T, ostr_buffer<T, Sys>, false, true>{buf, Sys::buffer_size, EncMetric_info<T>{}}, sy{f} {}
        /*
        void copy_from(const byte *from, size_t len){
            const byte *inc = from;
            while(len > 0){
                size_t buf_wrt = Sys::buffer_size - pt2;
                if(buf_wrt > 0){
                    size_t min = buf_wrt > len ? len : buf_wrt;
                    std::memcpy(buf+pt2, inc, min);
                    pt2 += min;
                    inc += min;
                    len -= min;
                }
                else{
                    push();
                    shift();
                    //pt1 will always be 0
                }
            }
        }

        void flush(){
            base_flush();
        }
        */
        void inc_rem(size_t){
            size_t wt;
            bool repeat;
            do{
                repeat=false;
                wt = sy.write_wrap(this->fir.data(), this->siz);
                if(wt > 0){
                    this->raw_fir_step(wt);
                }
                else
                    repeat=true;
            }
            while(repeat);
            this->rewind();//increases rem by n
        }

        void flush(){
            inc_rem(0);
            this->base_flush();
        }

        Sys get_system_id() const noexcept {return sy;}
};

}
