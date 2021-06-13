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

        size_t raw_freespace() const noexcept{
            return N-siz;
        }
        byte *raw_first() noexcept{
            return buf+fir;
        }
        byte *raw_last() noexcept{
            return buf + ((fir+siz) % N);
        }
        const byte *raw_first() const noexcept{
            return buf+fir;
        }
        const byte *raw_last() const noexcept{
            return buf + ((fir+siz) % N);
        }
        /*
         * This returns 0 if and only if buffer is empty
         */
        size_t raw_contiguous_first() const noexcept{
            if((fir+siz) <= N)
                return siz;
            else
                return N - fir;
        }
        /*
         * This returns 0 if and only if buffer is full
         */
        size_t raw_contiguous_last() const noexcept{
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

    /*
     * Useful for ghost reads
     */

    template<size_t N> requires buffer_len<N>
    struct tmp_buf{
        stat_buf<N> &ref;
        size_t tfir, tsiz;

        tmp_buf(stat_buf<N> &a) : ref{a}, tfir{a.fir}, tsiz{a.siz} {}
        /*
         * Should always be fir + siz = tfir + tsiz
         */
        byte *raw_first() const noexcept{
            return ref.buf+tfir;
        }
        byte *raw_last() const noexcept{
            return ref.raw_last();
        }
        size_t raw_contiguous_first() const noexcept{
            if((tfir+tsiz) <= N)
                return tsiz;
            else
                return N - tfir;
        }
        /*
         * Mustn't override written memory shadowed by tfir
         */
        size_t raw_contiguous_last() const noexcept{
            return ref.raw_contiguous_last();
        }
        void raw_get(size_t l){
            tfir = (tfir + l) % N;
            tsiz -= l;
        }
        void raw_set(size_t l){
            tsiz += l;
        }
    };
}
