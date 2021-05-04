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

uint ASCII::chLen(const byte *data){
	//non è necessario fare tutti i controlli, poiché si suppone che la stringa sia corretta
	//usare validChar per effettuare tutti i controlli
	return 1;
}

bool ASCII::validChar(const byte *data, uint &add) noexcept{
	byte b = *data;
	add=1;
	if(!bit_zero(b, 7))
		return false;
	return true;
}

uint ASCII::decode(unicode *uni, const byte *by, size_t l){
	if(l == 0)
		throw buffer_small{};
	*uni = read_unicode(by[0]);
	return 1;
}

uint ASCII::encode(const unicode &uni, byte *by, size_t l){
	if(l == 0)
		throw buffer_small{};
	if(uni >= 128)
		throw encoding_error("Cannot convert to an ASCII character");
	by[0] = byte{static_cast<uint8_t>(uni & 0xff)};
	return 1;
}

//------------------------------

uint Latin1::chLen(const byte *){
	return 1;
}

bool Latin1::validChar(const byte *, uint &add) noexcept{
	add = 1;
	return true;
}

uint Latin1::decode(unicode *uni, const byte *by, size_t l){
	if(l == 0)
		throw buffer_small{};
	*uni = read_unicode(by[0]);
	return 1;
}

uint Latin1::encode(const unicode &uni, byte *by, size_t l){
	if(l == 0)
		throw buffer_small{};
	if(uni >= 256)
		throw encoding_error("Cannot convert to a Latin1 character");
	by[0] = byte{static_cast<std::uint8_t>(uni & 0xff)};
	return 1;
}

//------------------------------
/*
void copyN(const byte *src, byte *dest, int len) noexcept{
	for(int i=0; i<len; i++){
		dest[i] = src[i];
	}
}
*/


