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

namespace sts{

using std::byte;

/*
    A basic unique_ptr that uses memory resources
*/
class basic_ptr{
	private:
		void reset() noexcept{
			memory = nullptr;
			dimension = 0;
		}
		std::pmr::memory_resource *alloc;
	public:
		byte *memory;
		std::size_t dimension;
		basic_ptr(std::pmr::memory_resource *all = std::pmr::get_default_resource()) : alloc{all == nullptr ? std::pmr::null_memory_resource() : all}, memory{nullptr}, dimension{0} {}
		explicit basic_ptr(std::size_t dim, std::pmr::memory_resource *all = std::pmr::get_default_resource()) : alloc{all == nullptr ? std::pmr::null_memory_resource() : all}, memory{nullptr}, dimension{0} {
			if(dim>0){
				dimension = dim;
				memory = static_cast<byte *>(alloc->allocate(dim));
			}
		}
		explicit basic_ptr(const byte *pt, std::size_t dim, std::pmr::memory_resource *all = std::pmr::get_default_resource()) : basic_ptr{dim, all} {
			if(dim > 0 && pt != nullptr)
				std::memcpy(memory, pt, dim);
		}
		basic_ptr(const basic_ptr &) = delete;
		basic_ptr(basic_ptr &&from) noexcept : memory{std::move(from.memory)}, dimension{from.dimension}, alloc{std::move(from.alloc)}
		{
			from.reset();
		}
		void free(){
			if(memory != nullptr){
				alloc->deallocate(memory, dimension);
				memory = nullptr;
			}
			dimension = 0;
		}
		~basic_ptr(){
			try{
				free();
			}
			catch(...){
				memory = nullptr;
                dimension = 0;
			}
		}
		void swap(basic_ptr &sw) noexcept{
			std::swap(memory, sw.memory);
			std::swap(dimension, sw.dimension);
			std::swap(alloc, sw.alloc);
		}
		basic_ptr &operator=(basic_ptr &&ref) noexcept{
			free();
			swap(ref);
			return *this;
		}
		basic_ptr &operator=(const basic_ptr &)=delete;

		void reallocate(std::size_t dim){
			byte *newm = static_cast<byte *>(alloc->allocate(dim));
			int mindim = dim > dimension ? dimension : dim;
			if(memory != nullptr)
				std::memcpy(newm, memory, mindim);
			free();
			memory = newm;
			dimension = dim;
		}
		void exp_fit(std::size_t fit){
			if(fit == 0)
				return;
			std::size_t grown = dimension == 0 ? 1 : dimension;
			while(grown < fit)
				grown *= 2;
			if(grown > dimension)
				reallocate(grown);
		}
		byte* leave() noexcept{
			byte *ret = nullptr;
			std::swap(ret, memory);
			dimension = 0;
			return ret;
		}

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
