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

template<safe_hasmax T, std::integral I>
void write_integer(CharOStream<T> &out, I val, const Int_opts &opt, std::pmr::memory_resource *res){
    if(opt.base < 2 || opt.base > 26)
        return;
    std::pmr::forward_list<uint> cfr{std::pmr::polymorphic_allocator<uint>{res}};
    bool minus = false;
    c_buffer<T> buf{out.raw_format()};
    if(val < 0){
        minus = true;
        val = -val;
    }
    if(val == 0){
        buf.char_write(out, '0'_uni);
    }
    else{
        do{
            uint rem = static_cast<uint>(val % opt.base);
            cfr.push_front(rem);
            val /= opt.base;
        }
        while(val > 0);

        if(minus){
            buf.char_write(out, '-'_uni);
        }
        else if(opt.plus){
            buf.char_write(out, '+'_uni);
        }

        for(uint cf : cfr){
            //std::cerr << "c: " << cf << std::endl;
            if(cf < 10)
                buf.char_write(out, unicode{0x30 + cf});
            else
                buf.char_write(out, unicode{0x61 + (cf - 10)});
        }
    }
}
