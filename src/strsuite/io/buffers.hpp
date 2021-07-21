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
#include <strsuite/encmetric/chite.hpp>
#include <strsuite/encmetric/byte_tools.hpp>
#include <cstring>

namespace sts{

    /*
     * Buffer structure
     *   |----------|abcdefghijklmno|---------------|
     *   ↑          ↑               ↑               ↑
     * base        fir     DATA    las             end
     *               <     siz     > <     rem     >
     * all 4 pointers should be modified by superclass
     *
     * If read=true then U must have inc_siz(uint) member function
     * If write=true then U must have inc_rem(size_t) member function
     */
template<general_enctype T, typename U, bool read, bool write>
class basic_buffer{
    private:
        U *mycast() noexcept{ return static_cast<U *>(this);}
        U const *mycast() const noexcept{ return static_cast<U const *>(this);}
    protected:
        tchar_pt<T> base;
        tchar_relative<T> fir, las, end;
        size_t siz, rem;
        basic_buffer(byte *b, size_t dim, EncMetric_info<T> ec) : base{b, ec}, fir{base}, las{base}, end{base, dim}, siz{0}, rem{dim} {}
        void rebase(byte *b, size_t dim){
            base = base.new_instance(b);
            /*
             * Must be dim >= end.difff()
             */
            rem += dim - end.difff();
            end.set_diff(dim);
        }
        void reset(){
            base = base.new_instance((byte*)nullptr);
            rem=0;
            siz=0;
            fir.set_diff(0);
            las.set_diff(0);
            end.set_diff(0);
        }
    public:
        struct buf_result{
            byte *pt;
            size_t s;
        };
        size_t size() const noexcept {return siz;}
        tchar_pt<T> get_ptr() const noexcept{ return fir.convert();}
        tchar_pt<T> set_ptr() const noexcept{ return las.convert();}
        tchar_pt<T> end_ptr() const noexcept{ return end.convert();}
        void rewind(){
            size_t skip = fir.difff();
            std::memmove(base.data(), fir.data(), siz);
            fir.set_diff(0);
            las.set_diff(siz);
            rem += skip;
        }

        void base_flush(){
            fir.set_diff(0);
            las.set_diff(0);
            siz = 0;
            rem = end.difff();
        }

        uint get_chLen() requires read{
            uint ret;
            bool get=false;
            do{
                try{
                    ret = fir.chLen(siz);
                    get=true;
                }
                catch(buffer_small &e){
                    if(fir.difff() != 0)
                        rewind();
                    mycast()->inc_siz(e.get_required_size());
                }
            }
            while(!get);

            while(ret > siz){
                if(fir.difff() != 0)
                    rewind();
                mycast()->inc_siz(ret - static_cast<uint>(siz));
            }
            return ret;
        }
        uint get_step() requires read{
            uint ret = fir.next(siz);
            siz -= ret;
            return ret;
        }

        void req_chLen(size_t nl) requires write{
            while(rem < nl){
                if(fir.difff() > 0)
                    rewind();
                else
                    mycast()->inc_rem(nl - rem);
            }
        }

        size_t req_step(size_t nl) requires write{
            las.set_diff(las.difff() + nl);
            rem -= nl;
            siz += nl;
            return nl;
        }
};


}
