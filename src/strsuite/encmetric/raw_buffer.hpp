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
/*
 * Simple raw buffer with low-level operations.
 *
 * Don't use it if you can use adv_string_buf. If you want to extend it use private inheritance
 */
#include <strsuite/encmetric/basic_ptr.hpp>

namespace sts{
    struct raw_buf{
        basic_ptr buffer;
        size_t len;
        size_t siz;
        raw_buf(std::pmr::memory_resource *all) : buffer{all}, len{0}, siz{0} {}
        raw_buf(size_t dim, std::pmr::memory_resource *all) : buffer{dim, all}, len{0}, siz{0} {}
        size_t raw_capacity() const noexcept {return buffer.dimension;}
        size_t raw_rem() const noexcept {return buffer.dimension-siz;}
        byte *raw_first() const noexcept {return buffer.memory;}
        byte *raw_last() const noexcept {return buffer.memory + siz;}
        void raw_clear() noexcept{
            len=0;
            siz=0;
        }
        void raw_leave(){
            buffer.leave();
            raw_clear();
        }

        void raw_fit(size_t fit){
            buffer.exp_fit(fit);
        }
        void raw_increase(size_t inc){
            buffer.exp_fit(buffer.dimension + inc);
        }
        void raw_newchar(uint chl) noexcept{
            len++;
            siz += chl;
        }
        void raw_append_chrs(const byte *from, size_t fs, size_t fl){
            append(buffer, siz, from, fs);
            siz += fs;
            len += fl;
        }
        void raw_append_chr(const byte *from, size_t fs){
            raw_append_chrs(from, fs, 1);
        }
        void raw_copy_to(byte *to) const{
            std::memcpy(to, buffer.memory, siz);
        }
    };


    template<size_t N>
    concept buffer_len = N > 1 && N < (static_cast<size_t>(1) << (sizeof(size_t) * 8 -1));

    /*
     * N must be <2^64/2 in order to avoid integer overflow
     */
    template<size_t N> requires buffer_len<N>
    struct stat_buf{
        byte buf[N];
        size_t fir, siz;
        stat_buf() : fir{0}, siz{0} {}
        size_t raw_freespace(){
            return N-siz;
        }
        byte *raw_first(){
            return buf+fir;
        }
        byte *raw_last(){
            return buf + ((fir+siz) % N);
        }
        /*
         * This returns 0 if and only if buffer is empty
         */
        size_t raw_contiguous_first(){
            if((fir+siz) <= N)
                return siz;
            else
                return N - fir;
        }
        /*
         * This returns 0 if and only if buffer is full
         */
        size_t raw_contiguous_last(){
            if((fir+siz) < N)
                return N - (fir+siz);
            else
                return N-siz;
        }
        void raw_get(size_t l){
            fir = (fir + l) % N;
            siz -= l;
        }
        void raw_set(size_t l){
            siz += l;
        }
    };

}
