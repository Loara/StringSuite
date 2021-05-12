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
#include <strsuite/encmetric/base64.hpp>

using namespace sts;

bool Base64_padding::validChar(const byte *b, uint &siz) noexcept{
	bool padding=false;
	for(int i=0; i<4; i++){
		int code = find(b[i]);
		if(code == -1)
			return false;
		else if(code == 64){
			if(i < 2)
				return false;
			padding=true;			
		}
		else if(padding)
			return false;
	}
	siz=4;
	return true;
}

uint Base64_padding::decode(three_byte *uni, const byte *by, size_t l){
	if(l < 4)
		throw buffer_small{};
	int data[4];
	uni->nbyte=3;
	for(int i=0; i<4; i++){
		data[i]=find(by[i]);
		if(data[i] == -1)
			throw encoding_error{};
		else if(data[i] == 64){
			if(i < 2)
				throw encoding_error{};
			if(uni->nbyte == 3)
				uni->nbyte = i-1;
			data[i]=0;
		}
		else if(uni->nbyte != 3)
			throw encoding_error{};
	}
	uni->bytes[0] = static_cast<byte>( (data[0] << 2) + (data[1] >> 4) );
	uni->bytes[1] = static_cast<byte>( (leave_b(data[1], 0, 1, 2, 3) << 4) + (data[2] >> 2) );
	uni->bytes[2] = static_cast<byte>( (leave_b(data[3], 0, 1) << 6) + data[3] );
	return 4;
}

uint Base64_padding::encode(const three_byte &uni, byte *by, size_t l){
	if(l < 4)
		throw buffer_small{};
	if(uni.nbyte == 0)
		return 0;
	else if(uni.nbyte > 3)
		throw encoding_error{};
	byte encoded[4];
	encoded[0] = uni.bytes[0] >> 2;
	encoded[1] = (leave_b(uni.bytes[0], 0, 1) << 4) | ((uni.nbyte >= 2 ? uni.bytes[1] : byte{0}) >> 4);
	encoded[2] = (leave_b( (uni.nbyte >= 2 ? uni.bytes[1] : byte{0}) , 0, 1, 2, 3) << 2) | ( (uni.nbyte == 3 ? uni.bytes[2] : byte{0}) >> 6);
	encoded[3] = leave_b( (uni.nbyte == 3 ? uni.bytes[2] : byte{0}) , 0, 1, 2, 3, 4, 5);
	by[0] = byte{recode[std::to_integer<uint>(encoded[0])]};
	by[1] = byte{recode[std::to_integer<uint>(encoded[1])]};
	if(uni.nbyte >= 2)
		by[2] = byte{recode[std::to_integer<uint>(encoded[2])]};
	else
		by[2] = byte{'='};
	if(uni.nbyte == 3)
		by[3] = byte{recode[std::to_integer<uint>(encoded[3])]};
	else
		by[3] = byte{'='};
	return 4;
}

void sts::base64_encode(const byte *from, byte *to, size_t siz){
	const byte *fpt=from;
	byte *tpt=to;
	while(true){
		uint mylen;
		if(siz == 0)
			break;
		else if(siz >= 3){
			siz -= 3;
			mylen=3;
		}
		else{
			mylen=(uint)siz;
			siz=0;
		}
		three_byte av{const_cast<byte *>((const byte *)fpt), mylen};
		Base64_padding::encode(av, tpt, 4);
		fpt += 3;
		tpt += 4;
	}
}
//void adv::base64_decode(const byte *from, byte *to, size_t siz);



