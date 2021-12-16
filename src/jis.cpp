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

sts::uint sts::EUC_JP::chLen(const sts::byte *b, sts::size_t s){
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

sts::tuple_ret<sts::jisx_213> sts::EUC_JP::decode(const sts::byte *b, sts::size_t s){
    if(s == 0)
        throw sts::buffer_small{1};
    sts::jisx_213 uni{};
    if(sts::bit_zero(b[0], 7)){
        uni.row = sts::byte{0};
        uni.col = b[0];
        return sts::tuple_ret<sts::jisx_213>{1, uni};
    }
    else if(b[0] == sts::byte{0x8e}){
        if(s < 2)
            throw sts::buffer_small{2 - static_cast<uint>(s)};
        uni.row = sts::byte{0};
        uni.col = b[1];
        return sts::tuple_ret<sts::jisx_213>{2, uni};
    }
    else if(b[0] == sts::byte{0x8f}){
        if(s < 3)
            throw sts::buffer_small{3 - static_cast<uint>(s)};
        // 0x20 + 0x80 = 0xa0 (minimum is 0xa1)
        if(sts::byte_less(b[1], sts::byte{0xa1}) || sts::byte_less(b[2], sts::byte{0xa1}))
            throw sts::incorrect_encoding{"Out of range JIS values"};
        uni.row = sts::byte_minus(b[1], sts::byte{0xa0});
        uni.col = sts::byte_minus(b[2], sts::byte{0xa0});
        uni.plane = sts::jisx_213::PLANE_2;
        return sts::tuple_ret<sts::jisx_213>{3, uni};
    }
    else{
        if(s < 2)
            throw sts::buffer_small{2 - static_cast<uint>(s)};
        if(sts::byte_less(b[0], sts::byte{0xa1}) || sts::byte_less(b[1], sts::byte{0xa1}))
            throw sts::incorrect_encoding{"Out of range JIS values"};
        uni.row = sts::byte_minus(b[0], sts::byte{0xa0});
        uni.col = sts::byte_minus(b[1], sts::byte{0xa0});
        uni.plane = sts::jisx_213::PLANE_1;
        return sts::tuple_ret<sts::jisx_213>{2, uni};
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
        by[0] = uni.row + 0xa0;
        by[1] = uni.col + 0xa0;
        return 2;
    }
    else{
        if(l < 3)
            throw sts::buffer_small{3 - static_cast<uint>(l)};
        by[0] = sts::byte{0x8f};
        by[1] = uni.row + 0xa0;
        by[2] = uni.col + 0xa0;
        return 3;
    }
}

sts::uint sts::SHIFT_JIS::chLen(const sts::byte *b, sts::size_t l){
    if(l == 0)
        throw sts::buffer_small{1};
    if(sts::is_in_range(b[0], 0x81, 0x9f) || is_in_range(b[0], 0xe0, 0xfc))
        return 2;
    return 1;
}

sts::validation_result sts::SHIFT_JIS::validChar(const sts::byte *b, sts::size_t l) noexcept{
    if(l == 0)
        return sts::validation_result{false, 0};
    if(sts::is_in_range(b[0], 0x81, 0x9f) || is_in_range(b[0], 0xe0, 0xfc)){
        if(l < 2)
            return sts::validation_result{false, 0};
        if(sts::is_in_range(b[1], 0x40, 0x7e) || sts::is_in_range(b[1], 0x80, 0xfc))
            return sts::validation_result{true, 2};
        else return sts::validation_result{false, 0};
    }
    else return sts::validation_result{true, 1};
}

sts::tuple_ret<sts::jisx_213> sts::SHIFT_JIS::decode(const sts::byte *by, sts::size_t l){
    if(l == 0)
        throw sts::buffer_small{1};
    sts::jisx_213 uni{};
    if(sts::is_in_range(by[0], 0, 0x80) || sts::is_in_range(by[0], 0xa0, 0xdf) || sts::is_in_range(by[0], 0xfd, 0xff)){
        uni.col = by[0];
        uni.row = sts::byte{0};
        return sts::tuple_ret<sts::jisx_213>{1, uni};
    }
    else{
        if(l < 2)
            throw sts::buffer_small{1};
        bool even;
        if(sts::is_in_range(by[1], 0x40, 0x7e)){
            even=false;
            uni.col = by[1] - 0x3f;
        }
        else if(sts::is_in_range(by[1], 0x80, 0x9e)){
            even=false;
            uni.col = by[1] - 0x40;
        }
        else if(sts::is_in_range(by[1], 0x9f, 0xfc)){
            even=true;
            uni.col = by[1] - 0x9e;
        }
        else throw sts::incorrect_encoding{"Invalid Shift JIS second byte"};

        if(sts::is_in_range(by[0], 0x81, 0x9f)){
            uni.plane = sts::jisx_213::PLANE_1;
            uni.row = even ? by[0] * 2 + 1 - 0x101 : by[0] * 2 - 0x101;
        }
        else if(sts::is_in_range(by[0], 0xe0, 0xef)){
            uni.plane = sts::jisx_213::PLANE_1;
            uni.row = even ? by[0] * 2 + 1 - 0x181 : by[0] * 2 - 0x181;
        }
        else if(sts::is_in_range(by[0], 0xf0, 0xfc)){
            uni.plane = sts::jisx_213::PLANE_2;
            sts::byte rtest = even ? by[0] * 2 + 1 - 0x1df : by[0] * 2 - 0x1df;
            if(rtest == 1_by || rtest == 3_by || rtest == 4_by || rtest == 5_by)
                uni.row = rtest;
            else{
                rtest = even ? (by[0] + 3) * 2 + 1 - 0x1df : (by[0] + 3) * 2 - 0x1df;
                if(rtest == 8_by || rtest == 12_by || rtest == 13_by || rtest == 14_by || rtest == 15_by)
                    uni.row = rtest;
                else{
                    uni.row = even ? by[0] * 2 + 1 - 0x19b : by[0] * 2 - 0x19b;
                }
            }
        }
        else throw sts::incorrect_encoding{"Invalid Shift JIS first byte"};

        return sts::tuple_ret<sts::jisx_213>{2, uni};
    }
}


sts::uint sts::SHIFT_JIS::encode(const sts::jisx_213 &uni, sts::byte *by, sts::size_t l){
    if(uni.row == sts::byte{0}){
        if(l == 0)
            throw sts::buffer_small{1};
        if(sts::is_in_range(uni.col, 0x81, 0x9f) || is_in_range(uni.col, 0xe0, 0xfc))
            throw sts::incorrect_encoding{"Cannot encode this character"};
        by[0] = uni.col;
        return 1;
    }
    else{
        if(l < 2)
            throw sts::buffer_small{2 - static_cast<uint>(l)};

        if(sts::bit_zero(uni.row, 0))
            by[1] = uni.col + 0x9e;
        else{
            if(sts::byte_less(uni.col, sts::byte{0x40}))
                by[1] = uni.col + 0x3f;
            else
                by[1] = uni.col + 0x40;
        }

        if(uni.plane == sts::jisx_213::PLANE_1){
            if(sts::is_in_range(uni.row, 0x1, 0x3e))
                by[0] = (uni.row + 0x101) / 2;
            else
                by[0] = (uni.row + 0x181) / 2;
        }
        else{
            if(sts::is_in_range(uni.row, 1, 7))
                by[0] = (uni.row + 0x1df) / 2;
            else if(sts::is_in_range(uni.row, 8, 15))
                by[0] = (uni.row + 0x1df) / 2 - 3;
            else
                by[0] = (uni.row + 0x19b) / 2;
        }
        return 2;
    }
}
