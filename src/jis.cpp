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

#include <strsuite/encmetric/jis.hpp>

uint sts::EUC_JP::chLen(const sts::byte *b, sts::size_t s){
    if(s == 0)
        throw sts::buffer_small{1};
    if(sts::bit_zero(b[0], 7))
        return 1;
    else if(b[0] == sts::byte{0x8f})
        return 3;
    else return 2;
}

sts::validation_result sts::EUC_JP::validChar(const sts::byte *b, sts::size_t s) noexcept{
    if(s == 0)
        return sts::validation_result{false, 0};
    if(sts::bit_zero(b[0], 7))
        return sts::validation_result{true, 1};
    if(b[0] == sts::byte{0x8e}){
        if(s < 2)
            return sts::validation_result{false, 0};
        if(sts::bit_zero(b[1], 7))
            return sts::validation_result{false, 0};
        else
            return sts::validation_result{true, 2};
    }
    if(b[0] == sts::byte{0x8f}){
        if(s < 3)
            return sts::validation_result{false, 0};
        if(sts::byte_less(b[1], sts::byte{0xa0}) || sts::byte_less(b[2], sts::byte{0xa0}))
            return sts::validation_result{false, 0};
        return sts::validation_result{true, 3};
    }
    else{
        if(s < 2)
            return sts::validation_result{false, 0};
        if(sts::byte_less(b[0], sts::byte{0xa1}) || sts::byte_less(b[1], sts::byte{0xa1}))
            return sts::validation_result{false, 0};
        return sts::validation_result{true, 3};
    }
}

sts::uint sts::EUC_JP::decode(sts::jisx_213 *uni, const sts::byte *b, sts::size_t s){
    if(s == 0)
        throw sts::buffer_small{1};
    if(sts::bit_zero(b[0], 7)){
        uni->row = sts::byte{0};
        uni->col = b[0];
        return 1;
    }
    else if(b[0] == sts::byte{0x8e}){
        if(s < 2)
            throw sts::buffer_small{2 - static_cast<uint>(s)};
        uni->row = sts::byte{0};
        uni->col = b[1];
        return 2;
    }
    else if(b[0] == sts::byte{0x8f}){
        if(s < 3)
            throw sts::buffer_small{3 - static_cast<uint>(s)};
        // 0x20 + 0x80 = 0xa0 (minimum is 0xa1)
        if(sts::byte_less(b[1], sts::byte{0xa1}) || sts::byte_less(b[2], sts::byte{0xa1}))
            throw sts::incorrect_encoding{"Out of range JIS values"};
        uni->row = sts::byte_minus(b[1], sts::byte{0xa0});
        uni->col = sts::byte_minus(b[2], sts::byte{0xa0});
        uni->plane = sts::jisx_213::PLANE_2;
        return 3;
    }
    else{
        if(s < 2)
            throw sts::buffer_small{2 - static_cast<uint>(s)};
        if(sts::byte_less(b[0], sts::byte{0xa1}) || sts::byte_less(b[1], sts::byte{0xa1}))
            throw sts::incorrect_encoding{"Out of range JIS values"};
        uni->row = sts::byte_minus(b[0], sts::byte{0xa0});
        uni->col = sts::byte_minus(b[1], sts::byte{0xa0});
        uni->plane = sts::jisx_213::PLANE_1;
        return 2;
    }
}

sts::uint sts::EUC_JP::encode(const sts::jisx_213 &uni, sts::byte *by, sts::size_t l){
    if(l == 0)
        throw sts::buffer_small{1};
    if(uni.row == sts::byte{0}){
        if(sts::bit_zero(uni.col, 7)){
            by[0] = uni.col;
            return 1;
        }
        else{
            if(l < 2)
                throw sts::buffer_small{1};
            by[0] = sts::byte{0x8e};
            by[1] = uni.col;
            return 2;
        }
    }
    else if(uni.plane == sts::jisx_213::PLANE_1){
        if(l < 2)
            throw sts::buffer_small{1};
        by[0] = sts::byte_plus(uni.row, sts::byte{0xa0});
        by[1] = sts::byte_plus(uni.col, sts::byte{0xa0});
        return 2;
    }
    else{
        if(l < 3)
            throw sts::buffer_small{3 - static_cast<uint>(l)};
        by[0] = sts::byte{0x8f};
        by[1] = sts::byte_plus(uni.row, sts::byte{0xa0});
        by[2] = sts::byte_plus(uni.col, sts::byte{0xa0});
        return 3;
    }
}
/*
 * Shift jis ranges (wiki)
 * . 129 - 159
 * . 224 - 239
 * . . 240 - 242
 * . . 240 - 244
 * . 244 - 252
 * , 64 - 126
 * , 128 - 158
 * , 159 - 252
 */
