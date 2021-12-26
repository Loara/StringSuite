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
     *
     * Both these functions are hint, but not satisfying them implies force_ members throw exceptions
     */
template<typename U, bool read, bool write>
class basic_buffer{
    protected:
        U *mycast() noexcept{ return static_cast<U *>(this);}
        U const *mycast() const noexcept{ return static_cast<U const *>(this);}
    public:
        byte *base;
        size_t fir, las, end;
        size_t siz, rem;

        basic_buffer(byte *b, size_t dim) : base{b}, fir{0}, las{0}, end{dim}, siz{0}, rem{dim} {}
        void rebase(byte *b, size_t dim){
            base = b;
            /*
             * Must be dim >= las
             */
            rem = dim - las;
            end = dim;
        }
        void reset(){
            base = nullptr;
            rem=0;
            siz=0;
            fir=0;
            las=0;
            end=0;
        }
        void discard_all(){
            fir = 0;
            las = 0;
            siz = 0;
            rem = end;
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
        void cut_ending(size_t dec){
            las -= dec;
            siz -= dec;
            rem += dec;
        }

        /*
         * moves data to begin in order to increase rem
         */
        void rewind(){
            if(fir == 0)
                return;
            size_t skip = fir;
            move_bytes(base, base + fir, siz);
            fir = 0;
            las = siz;
            rem += skip;
        }

        size_t ask_size(size_t nl) requires read{
            if(fir != 0)
                rewind();
            if(siz >= nl)
                return nl;
            else{
                mycast()->inc_siz(nl - siz);
                return siz >= nl ? nl : siz;
            }
        }

        void force_size(size_t nl) requires read{
            if(fir != 0)
                rewind();
            if(siz >= nl)
                return;
            else{
                mycast()->inc_siz(nl - siz);
                if(siz < nl)
                    throw IOFail{};
            }
        }

        template<typename T>
        uint get_chLen(EncMetric_info<T> rf) requires read{
            uint ret;
            bool get=false;
            force_size(rf.min_bytes());
            do{
                try{
                    ret = rf.chLen(base + fir, siz);
                    get = true;
                }
                catch(buffer_small &e){
                    if(fir != 0)
                        rewind();
                    mycast()->inc_siz(e.get_required_size());
                }
            }
            while(!get);
            force_size(ret);
            return ret;
        }

        /*
         * Tries to increase rem in order to be greater than nl
         *
         * returns minimum between rem and nl
         */
        size_t ask_rem(size_t nl) requires write{
            if(fir > 0)
                rewind();
            if(rem >= nl)
                return nl;
            else{
                mycast()->inc_rem(nl - rem);
                return rem >= nl ? nl : rem;
            }
        }
        void force_rem(size_t nl) requires write{
            if(fir > 0)
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
