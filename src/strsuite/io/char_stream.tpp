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
template<general_enctype T>
uint C_B_IStream<T>::do_char_read(tchar_pt<T> pt, size_t buf){
    if(!enc.equalTo(pt.raw_format()))
        throw encoding_error{"Not same encoding"};
    byte *raw = pt.data();
    if(c_buffer.siz == 0){
        c_buffer.raw_increase(pt.min_bytes());
        input->read_exactly(c_buffer.raw_first(), pt.min_bytes());
        c_buffer.siz += pt.min_bytes();
        uint chl = 0;
        bool readed = false;
        do{
            try{
                chl = enc.chLen(c_buffer.raw_first(), c_buffer.siz);
                if(chl > c_buffer.siz){
                    c_buffer.raw_fit(chl);
                    input->read_exactly(c_buffer.raw_last(), chl - c_buffer.siz);
                    c_buffer.siz = chl;
                }
                readed = true;
            }
            catch(buffer_small &err){
                c_buffer.raw_increase(err.get_required_size());
            }
        }
        while(!readed);
        c_buffer.len = 1;
    }
    if(buf >= c_buffer.siz){
        c_buffer.raw_copy_to(raw);
        uint ret = c_buffer.siz;
        c_buffer.raw_clear();
        return ret;
    }
    else
        throw IOBufsmall{};
}
