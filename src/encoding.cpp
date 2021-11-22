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
#include <strsuite/encmetric/encoding.hpp>

using namespace sts;

validation_result ASCII::validChar(const byte *data, size_t siz) noexcept{
	byte b = *data;
	if(siz == 0)
        return validation_result{false, 0};
	if(!bit_zero(b, 7))
		return validation_result{false, 0};
	return validation_result{true, 1};
}

tuple_ret<unicode> ASCII::decode(const byte *by, size_t l){
	if(l == 0)
		throw buffer_small{1};
	return tuple_ret<unicode>{1u, read_unicode(by[0])};
}

uint ASCII::encode(const unicode &uni, byte *by, size_t l){
	if(l == 0)
		throw buffer_small{1};
	if(uni >= 128)
		throw encoding_error("Cannot convert to an ASCII character");
	by[0] = byte{static_cast<std::uint8_t>(uni & 0xff)};
	return 1;
}

//------------------------------

validation_result Latin1::validChar(const byte *, size_t siz) noexcept{
	return validation_result{siz >= 1, 1};
}

tuple_ret<unicode> Latin1::decode(const byte *by, size_t l){
	if(l == 0)
		throw buffer_small{1};
	return tuple_ret<unicode>{1u, read_unicode(by[0])};
}

uint Latin1::encode(const unicode &uni, byte *by, size_t l){
	if(l == 0)
		throw buffer_small{1};
	if(uni >= 256)
		throw encoding_error("Cannot convert to a Latin1 character");
	by[0] = byte{static_cast<std::uint8_t>(uni & 0xff)};
	return 1;
}


