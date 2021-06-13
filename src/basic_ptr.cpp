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

void basic_ptr::reset() noexcept{
	memory = nullptr;
	dimension = 0;
}

basic_ptr::basic_ptr(std::pmr::memory_resource *all) : alloc{all == nullptr ? std::pmr::null_memory_resource() : all}, memory{nullptr}, dimension{0} {}

basic_ptr::basic_ptr(std::size_t dim, std::pmr::memory_resource *all) : basic_ptr{all} {
	if(dim>0){
		dimension = dim;
		memory = static_cast<byte *>(alloc->allocate(dim));
	}
}

basic_ptr::basic_ptr(const byte *pt, std::size_t dim, std::pmr::memory_resource *all) : basic_ptr{dim, all} {
    if(dim > 0 && pt != nullptr)
        std::memcpy(memory, pt, dim);
}

basic_ptr::basic_ptr(basic_ptr &&from) noexcept : memory{std::move(from.memory)}, dimension{from.dimension}, alloc{std::move(from.alloc)}{
    from.reset();
}

void basic_ptr::free(){
    if(memory != nullptr){
		alloc->deallocate(memory, dimension);
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
    byte *newm = static_cast<byte *>(alloc->allocate(dim));
    size_t mindim = dim > dimension ? dimension : dim;
    if(memory != nullptr)
        std::memcpy(newm, memory, mindim);
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

byte* basic_ptr::leave() noexcept{
    byte *ret = nullptr;
	std::swap(ret, memory);
	dimension = 0;
	return ret;
}