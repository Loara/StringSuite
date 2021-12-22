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

#include <strsuite/encmetric/byte_tools.hpp>
#include <cstring>

bool sts::compare(const sts::byte *a, const sts::byte *b, std::size_t nsiz) noexcept{
	return std::memcmp(a, b, nsiz) == 0;
}

void sts::copy_bytes(sts::byte *a, const sts::byte *b, std::size_t nsiz) noexcept{
	std::memcpy(a, b, nsiz);
}
