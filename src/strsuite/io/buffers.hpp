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
    protected:
        U *mycast() noexcept{ return static_cast<U *>(this);}
        U const *mycast() const noexcept{ return static_cast<U const *>(this);}
    public:
        tchar_pt<T> base;
        tchar_relative<T> fir, las, end;
        size_t siz, rem;
        basic_buffer(byte *b, size_t dim, EncMetric_info<T> ec) : base{b, ec}, fir{base}, las{base}, end{base, dim}, siz{0}, rem{dim} {}
        void rebase(byte *b, size_t dim){
            base = base.new_instance(b);
            /*
             * Must be dim >= las.difff()
             */
            rem = dim - las.difff();
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
        void base_flush(){
            fir.set_diff(0);
            las.set_diff(0);
            siz = 0;
            rem = end.difff();
        }

        void raw_fir_step(size_t inc){
            fir += inc;
            siz -= inc;
        }
        void raw_las_step(size_t inc){
            las += inc;
            siz += inc;
            rem -= inc;
        }

        void rewind(){
            size_t skip = fir.difff();
            std::memmove(base.data(), fir.data(), siz);
            fir.set_diff(0);
            las.set_diff(siz);
            rem += skip;
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

        /*
         * Tries to increase rem in otrder to be greater than nl
         *
         * returns minimum between rem and nl
         */
        size_t req_frspc(size_t nl) requires write{
            if(fir.difff() > 0)
                rewind();
            if(rem >= nl)
                return nl;
            else{
                mycast()->inc_rem(nl - rem);
                return rem >= nl ? nl : rem;
            }
        }
        void force_frspc(size_t nl) requires write{
            if(fir.difff() > 0)
                rewind();
            if(rem >= nl)
                return;
            else{
                mycast()->inc_rem(nl - rem);
                if(rem < nl)
                    throw IOFail{};
            }
        }
};


}
