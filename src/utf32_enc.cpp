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
#include <strsuite/encmetric/utf32_enc_0.hpp>

namespace sts{
template<bool be>
validation_result UTF32<be>::validChar(const byte *data, size_t siz) noexcept{
    if(siz < 4)
        return validation_result{false, 0};
	if(access(data, be, 4, 0) != byte{0})
		return validation_result{false, 0};
	byte rew = access(data, be, 4, 1);
	if(!bit_zero(rew, 7, 6, 5))
		return validation_result{false, 0};
	if(bit_one(rew, 4) && !bit_zero(rew, 3, 2, 1, 0))
		return validation_result{false, 0};
	return validation_result{true, 4};
}

template<bool be>
uint UTF32<be>::decode(unicode *uni, const byte *by, size_t l){
	if(l < 4)
		throw buffer_small{4};
	*uni = unicode{0};
	for(int i=0; i<4; i++){
		*uni = unicode{(*uni << 8) + read_unicode(access(by, be, 4, i))};
	}
	return 4;
}

template<bool be>
uint UTF32<be>::encode(const unicode &unin, byte *by, size_t l){
	if(l < 4)
		throw buffer_small{4};
	byte temp[4];
	unicode uni=unin;
	for(int i=0; i<4; i++){
		access(temp, be, 4, 3-i) = byte{static_cast<uint8_t>(uni & 0xff)};
		uni=unicode{uni >> 8};
	}
	return 4;
}

	template class UTF32<true>;
	template class UTF32<false>;

}


