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
#include <strsuite/encmetric/basic_ptr.hpp>

using namespace sts;

byte *basic_ptr::raw_allocate(size_t dim){
    byte *ret = static_cast<byte *>(alloc->allocate(dim));
    return ret;
}

void basic_ptr::raw_deallocate(byte *mem, size_t dim){
    alloc->deallocate(mem, dim);
}

void basic_ptr::reset() noexcept{
	memory = nullptr;
	dimension = 0;
}

basic_ptr::basic_ptr(std::pmr::memory_resource *all) : alloc{all == nullptr ? std::pmr::null_memory_resource() : all}, memory{nullptr}, dimension{0} {}

basic_ptr::basic_ptr(std::size_t dim, std::pmr::memory_resource *all) : basic_ptr{all} {
	if(dim>0){
		dimension = dim;
		memory = raw_allocate(dim);
	}
}

basic_ptr::basic_ptr(const byte *pt, std::size_t dim, std::pmr::memory_resource *all) : basic_ptr{dim, all} {
    if(dim > 0 && pt != nullptr)
        std::memcpy(memory, pt, dim);
}

basic_ptr::basic_ptr(basic_ptr &&from) noexcept : alloc{std::move(from.alloc)}, memory{std::move(from.memory)}, dimension{from.dimension}{
    from.reset();
}

void basic_ptr::free(){
    if(memory != nullptr){
		raw_deallocate(memory, dimension);
		memory = nullptr;
	}
	dimension = 0;
}

basic_ptr::~basic_ptr(){
    try{
		free();
	}
	catch(...){
		memory = nullptr;
        dimension = 0;
	}
}

void basic_ptr::swap(basic_ptr &sw) noexcept{
    std::swap(memory, sw.memory);
	std::swap(dimension, sw.dimension);
	std::swap(alloc, sw.alloc);
}

basic_ptr &basic_ptr::operator=(basic_ptr &&ref){
    free();
	swap(ref);
	return *this;
}

void basic_ptr::reallocate(std::size_t dim){
    byte *newm = raw_allocate(dim);
    size_t mindim = dim > dimension ? dimension : dim;
    if(memory != nullptr)
        std::memcpy(newm, memory, mindim);
    free();
    memory = newm;
    dimension = dim;
}

void basic_ptr::reallocate_reverse(std::size_t dim){
    byte *newm = raw_allocate(dim);
    const byte *from = nullptr;
    byte *to = nullptr;
    if(memory != nullptr){
        if(dim > dimension){
            //bigger
            from = memory;
            to = newm + (dim - dimension);
            std::memcpy(to, from, dimension);
        }
        else{
            //smaller
            from = memory + (dimension - dim);
            to = newm;
            std::memcpy(to, from, dim);
        }
    }
    free();
    memory = newm;
    dimension = dim;
}

void basic_ptr::shift(std::size_t first, std::size_t n){
    if(memory == nullptr || first == 0 || n == 0)
        return;
    if(first + n > dimension || !no_overflow_sum(first, n))
        return;
    std::memmove(memory, memory+first, n);
}

void basic_ptr::exp_fit(std::size_t fit){
    if(fit == 0)
        return;
    std::size_t grown = dimension == 0 ? 8 : dimension;
    while(grown < fit)
        grown *= 2;
    if(grown > dimension)
        reallocate(grown);
}

void basic_ptr::exp_fit_reverse(std::size_t fit){
    if(fit == 0)
        return;
    std::size_t grown = dimension == 0 ? 8 : dimension;
    while(grown < fit)
        grown *= 2;
    if(grown > dimension)
        reallocate_reverse(grown);
}

byte* basic_ptr::leave() noexcept{
    byte *ret = nullptr;
	std::swap(ret, memory);
	dimension = 0;
	return ret;
}
