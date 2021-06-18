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
#include <strsuite/encmetric/utf16_enc_0.hpp>

namespace sts{

template class Endian_enc_size<true, char16_t, 2>;
template class Endian_enc_size<false, char16_t, 2>;

template<bool be>
using myend = Endian_enc_size<be, char16_t, 2>;

template<bool be>
struct uhelp{
static bool H_range(const byte *datas) noexcept{
    char16_t val;
    myend<be>::decode(&val, datas, 2);
    return val >= 0xd800 && val < 0xdc00;
    /*
	byte data = access(datas, be, 2, 0);
	return bit_one(data, 7, 6, 4, 3) && bit_zero(data, 5, 2);
	*/
}

static bool L_range(const byte *datas) noexcept{
    char16_t val;
    myend<be>::decode(&val, datas, 2);
    return val >= 0xdc00 && val < 0xe000;
    /*
	byte data = access(datas, be, 2, 0);
	return bit_one(data, 7, 6, 4, 3, 2) && bit_zero(data, 5);
	*/
}

static bool range(const byte *datas) noexcept{
    char16_t val;
    myend<be>::decode(&val, datas, 2);
    return val >= 0xd800 && val < 0xe000;
    /*
	byte data = access(datas, be, 2, 0);
	return bit_one(data, 7, 6, 4, 3) && bit_zero(data, 5);
	*/
}
};

template<bool be>
uint UTF16<be>::chLen(const byte *data, size_t siz){
    if(siz < 2){
        throw buffer_small{2-static_cast<uint>(siz)};
    }
	if(uhelp<be>::range(data))
		return 4;
	else
		return 2;
}

template<bool be>
validation_result UTF16<be>::validChar(const byte *data, size_t siz) noexcept{
    if(siz < 2){
        return validation_result{false, 0};
    }
	if(uhelp<be>::H_range(data)){
		if(!uhelp<be>::L_range(data+2))
			return validation_result{false, 0};
        else
            return validation_result{true, 4};
	}
	else if(uhelp<be>::L_range(data))
		return validation_result{false, 0};
	else
        return validation_result{true, 2};
}

template<bool be>
uint UTF16<be>::decode(unicode *uni, const byte *by, size_t l){
	if(l < 2)
		throw buffer_small{2-static_cast<uint>(l)};
	uint y_byte = 0;
	*uni = unicode{0};
	
	if(uhelp<be>::range(by))
		y_byte = 4;
	else
		y_byte = 2;

	if(l < y_byte)
		throw buffer_small{y_byte-static_cast<uint>(l)};
	if(y_byte == 4){
        char16_t temph, templ;
        myend<be>::decode(&temph, by, 2);
        myend<be>::decode(&templ, by+2, 2);
        *uni = unicode{(( static_cast<uint>(temph) - 0xd800) << 10) + ( static_cast<uint>(templ) - 0xdc00)};
		/*
		unicode p_word{(read_unicode( leave_b(access(by, be, 2, 0), 0, 1) ) << 8) + read_unicode(access(by, be, 2, 1))};
		unicode s_word{(read_unicode( leave_b(access(by+2, be, 2, 0), 0, 1) ) << 8) + read_unicode(access(by+2, be, 2, 1))};
		*uni = unicode{(p_word << 10) + s_word + 0x10000};
        */
	}
	else{
        char16_t temp;
        myend<be>::decode(&temp, by, 2);
		*uni = unicode{temp};
	}
	return y_byte;
}

template<bool be>
uint UTF16<be>::encode(const unicode &unin, byte *by, size_t l){
	if(l < 2)
		throw buffer_small{2-static_cast<uint>(l)};
	uint y_byte;
	if(unin >= 0 && unin < 0xffff){
		y_byte = 2;
	}
	else if(unin >= 0x10000 && unin < 0x110000){
		y_byte = 4;
	}
	else throw encoding_error("Not Unicode character");

	if(l < y_byte)
		throw buffer_small{y_byte-static_cast<uint>(l)};
	
	if(y_byte == 4){
		unicode uni{unin - 0x10000};
		access(by+2, be, 2, 1) = byte{static_cast<uint8_t>(uni & 0xff)};
		uni=unicode{uni >> 8};
		access(by+2, be, 2, 0) = byte{static_cast<uint8_t>(uni & 0x03)};
		uni=unicode{uni >> 2};
		access(by, be, 2, 1) = byte{static_cast<uint8_t>(uni & 0xff)};
		uni=unicode{uni >> 8};
		access(by, be, 2, 0) = byte{static_cast<uint8_t>(uni & 0x03)};

		set_bits(access(by+2, be, 2, 0), 7, 6, 4, 3, 2);
		set_bits(access(by, be, 2, 0), 7, 6, 4, 3);
	}
	else{
		unicode uni = unin;
		access(by, be, 2, 1) = byte{static_cast<uint8_t>(uni & 0xff)};
		uni=unicode{uni >> 8};
		access(by, be, 2, 0) = byte{static_cast<uint8_t>(uni & 0xff)};
	}
	return y_byte;
}
	template class UTF16<true>;
	template class UTF16<false>;

}


