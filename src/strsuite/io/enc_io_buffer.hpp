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
#include <cstring>

namespace sts{

template<typename Sys>
class istr_buffer{
    private:
        Sys sy;
        byte buf[Sys::buffer_size];
        size_t siz, fir;
        void refresh(){
            fir=0;
            siz=0;
        }
        void fill(){
            size_t wt;
            bool repeat;
            do{
                repeat=false;
                wt = sy.read_wrap(buf + siz, Sys::buffer_size - siz, repeat);
            }while(repeat);
            siz += wt;
        }
        void shift(size_t &tmp){
            if(fir == 0)
                return;
            siz = siz - fir;
            tmp = tmp - fir;
            std::memmove(buf, buf + fir, siz);
            fir = 0;
        }
    public:
        /*
         * Read exactly len bytes, throw exception (IOEOF for example) if there are less
         *
         * Since it reads only single character we have len <= 4
         * so tmp - fir <= 4 and if siz = tmp then
         * buffer_size > 4 >= siz - fir and  buffer_size - (siz - fir) > 0, this implies that shift + fill will read at least 1 byte
         */
        void copy_to(byte *to, size_t len){
            size_t tmp = fir;
            byte *inc = to;
            while(len > 0){
                size_t buf_wrt = siz - tmp;
                if(buf_wrt > 0){
                    size_t min = buf_wrt > len ? len : buf_wrt;
                    std::memcpy(inc, buf + tmp, min);
                    tmp += min;
                    inc += min;
                    len -= min;
                }
                else{
                    shift(tmp);
                    fill();
                }
            }
            fir = tmp;
        }
        void ghost_copy(byte *to, size_t len){
            size_t tmp = fir;
            byte *inc = to;
            while(len > 0){
                size_t buf_wrt = siz - tmp;
                if(buf_wrt > 0){
                    size_t min = buf_wrt > len ? len : buf_wrt;
                    std::memcpy(inc, buf + tmp, min);
                    tmp += min;
                    inc += min;
                    len -= min;
                }
                else{
                    shift(tmp);
                    fill();
                }
            }
            //Nope
            //fir = tmp;
        }

        Sys get_system_id() const noexcept {return sy;}

        istr_buffer(const Sys &f) : sy{f} {}
};

template<typename Sys>
class ostr_buffer{
    private:
        Sys sy;
        byte buf[Sys::buffer_size];
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
    public:
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
            while(true){
                shift();
                if(pt2 == 0)
                    break;
                push();
            }
        }

        Sys get_system_id() const noexcept {return sy;}

        ostr_buffer(const Sys &f) : sy{f} {}
};

}
