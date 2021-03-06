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

#include <bit>

#undef Encmetric_end

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

# define Encmetric_end

#elif defined(__BYTE_ORDER__) && defined(__ORDER_PDP_ENDIAN__) && __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__

# error endianess not recognized

#elif defined(__LITTLE_ENDIAN__)

#elif defined(__BIG_ENDIAN__)

# define Encmetric_end

#elif defined(_MSC_VER) || defined(__i386__) || defined(__x86_64__)

#else

# error endianess not recognized

#endif

/*
 * detect if compiler support concepts in costructors
 */

#ifdef __GNUC__
#define costructors_concepts (__GNUC__ > 10 || (__GNUC__ == 10 && __GNUC_MINOR__ >= 3))
#else
#define costructors_concepts 0
#endif

/*
 * System-dependent string literals
 */
#ifdef using_windows
#define STS_IO_asv(str) u##str##_asv
#else
#define STS_IO_asv(str) u8##str##_asv
#endif

namespace sts{

constexpr bool is_windows() noexcept{
	#ifdef using_windows
		return true;
	#else
		return false;
	#endif
}

inline constexpr bool bend = std::endian::native == std::endian::big;

}
