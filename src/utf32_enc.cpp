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
template class Endian_enc_size<char32_t, 4, BE_end<4>>;
template class Endian_enc_size<char32_t, 4, LE_end<4>>;

template<typename Seq>
using myend=Endian_enc_size<char32_t, 4, Seq>;

template<typename Seq>
validation_result UTF32<Seq>::validChar(const byte *data, size_t siz) noexcept{
    if(siz < 4){
        return validation_result{false, 0};
    }
    char32_t value;
    std::tie(std::ignore, value) = myend<Seq>::decode(data, siz);
    return validation_result{value < 0x110000, 4};
}

template<typename Seq>
tuple_ret<unicode> UTF32<Seq>::decode(const byte *by, size_t l){
	if(l < 4)
		throw buffer_small{4-static_cast<uint>(l)};
    char32_t tmp;
    std::tie(std::ignore, tmp) = myend<Seq>::decode(by, l);
    return tuple_ret<unicode>{4, unicode{tmp}};
}

template<typename Seq>
uint UTF32<Seq>::encode(const unicode &unin, byte *by, size_t l){
	if(l < 4)
		throw buffer_small{4-static_cast<uint>(l)};
	char32_t cast = static_cast<char32_t>(unin);
    myend<Seq>::encode(cast, by, l);
	return 4;
}

	template class UTF32<BE_end<4>>;
	template class UTF32<LE_end<4>>;

}


