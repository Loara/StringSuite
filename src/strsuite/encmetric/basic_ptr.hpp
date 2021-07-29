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
#include <new>
#include <memory_resource>
#include <cstring>
#include <strsuite/encmetric/base.hpp>

namespace sts{

/*
    A basic unique_ptr that uses memory resources
*/
class basic_ptr{
	private:
		void reset() noexcept;
		std::pmr::memory_resource *alloc;
        byte *raw_allocate(size_t);
        void raw_deallocate(byte *, size_t);
	public:
		byte *memory;
		std::size_t dimension;

		explicit basic_ptr(std::pmr::memory_resource *all = std::pmr::get_default_resource());
		explicit basic_ptr(std::size_t dim, std::pmr::memory_resource *all = std::pmr::get_default_resource());
		explicit basic_ptr(const byte *pt, std::size_t dim, std::pmr::memory_resource *all = std::pmr::get_default_resource());
		basic_ptr(const basic_ptr &) = delete;
		basic_ptr(basic_ptr &&from) noexcept;
		void free();
		~basic_ptr();
		void swap(basic_ptr &sw) noexcept;
		basic_ptr &operator=(basic_ptr &&ref);
		basic_ptr &operator=(const basic_ptr &)=delete;

		void reallocate(std::size_t dim);
        /*
         * shift n bytes from first to 0
         *
         * WARNING: you should be sure that first + n is lesser or equal than dimension
         */
        void shift(std::size_t first, std::size_t n);
		void exp_fit(std::size_t fit);
		byte* leave() noexcept;

		std::pmr::memory_resource *get_allocator() const noexcept{
			return alloc;
		}
		basic_ptr copy() const{
			return basic_ptr{memory, dimension, alloc};
		}
};

/*
 * Append bytes at a specified position
 *
 * Usually final dimension is strictly greater than pos+siz
 */
inline void append(basic_ptr &mem, std::size_t pos, const byte *from, std::size_t siz){
	if(mem.dimension < pos + siz){
		mem.exp_fit(pos+siz);
	}
	if(siz != 0)
        std::memcpy(mem.memory + pos, from, siz);
}

}
