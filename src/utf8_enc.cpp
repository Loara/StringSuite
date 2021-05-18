
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
#include <strsuite/encmetric/utf8_enc.hpp>

using namespace sts;

uint UTF8::chLen(const byte *data, size_t siz){
	if(siz == 0)
        throw buffer_small{1};
	byte b = *data;
	if(bit_zero(b, 7))
		return 1;
	else if(bit_zero(b, 6))
		throw incorrect_encoding("Invalid utf8 character");
	else if(bit_zero(b, 5))
		return 2;
	else if(bit_zero(b, 4))
		return 3;
	else if(bit_zero(b, 3))
		return 4;
	else{
		throw encoding_error("Invalid utf8 character");
	}
}

validation_result UTF8::validChar(const byte *data, size_t siz) noexcept{
    if(siz == 0)
        return validation_result{false, 0};

    uint add;
	byte b = *data;
	if(bit_zero(b, 7))
		add = 1;
	else if(bit_zero(b, 6))
		return validation_result{false, 0};
	else if(bit_zero(b, 5))
		add = 2;
	else if(bit_zero(b, 4))
		add = 3;
	else if(bit_zero(b, 3))
		add = 4;
	else
		return validation_result{false, 0};
	for(int i=1; i<add; i++){
		b=data[i];
		if(bit_zero(b, 7) || bit_one(b, 6))
			return validation_result{false, 0};
	}
	return validation_result{true, add};
}

uint UTF8::decode(unicode *uni, const byte *by, size_t l){
	if(l == 0)
		throw buffer_small{1};
	size_t y_byte = 0;
	byte b = *by;
	*uni = unicode{0};
	
	if(bit_zero(b, 7)){
		y_byte = 1;
	}
	else if(bit_zero(b, 6))
		throw encoding_error("Invalid utf8 character");
	else if(bit_zero(b, 5)){
		y_byte = 2;
		reset_bits(b, 7, 6);
	}
	else if(bit_zero(b, 4)){
		y_byte = 3;
		reset_bits(b, 7, 6, 5);
	}
	else if(bit_zero(b, 3)){
		y_byte = 4;
		reset_bits(b, 7, 6, 5, 4);
	}
	else
		throw encoding_error("Invalid utf8 character");

	if(l < y_byte )
		throw buffer_small{(uint)y_byte};
	*uni = read_unicode(b);
	for(size_t i = 1; i < y_byte; i++){
		byte temp = by[i];
		reset_bits(temp, 6, 7);
		*uni = unicode{(*uni << 6) + read_unicode(temp)};
	}
	return y_byte;
}

uint UTF8::encode(const unicode &unin, byte *by, size_t l){
	if(l == 0)
		throw buffer_small{1};
	size_t y_byte;
	byte set_mask{0};
	if(unin < 0x80){
		y_byte = 1;
	}
	else if(unin >= 0x80 && unin < 0x800){
		y_byte = 2;
		set_mask = compose_bit_mask<byte>(7, 6);
	}
	else if(unin >= 0x800 && unin < 0x10000){
		y_byte = 3;
		set_mask = compose_bit_mask<byte>(7, 6, 5);
	}
	else if(unin >= 0x10000 && unin < 0x110000){
		y_byte = 4;
		set_mask = compose_bit_mask<byte>(7, 6, 5, 4);
	}
	else throw encoding_error("Not Unicode character");

	unicode uni=unin;
	if(l < y_byte )
		throw buffer_small{(uint)y_byte};
	for(size_t i = y_byte-1; i>=1; i--){
		by[i] = byte{static_cast<uint8_t>(uni & 0x3f)};
		uni=unicode{uni >> 6};
		set_bits(by[i], 7);
	}
	by[0] = byte{static_cast<uint8_t>(uni)};
	by[0] |= set_mask;
	return y_byte;
}


