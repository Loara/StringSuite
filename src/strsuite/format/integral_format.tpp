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

    template<typename Stream, std::integral I>
    void write_integer(Stream &out, I val, const Int_opts &opt){
        if(opt.base < 2 || opt.base > 36)
            return;
        bool minus = false;
        std::forward_list<unicode> stack;
        if(val < 0){
            minus = true;
            val = -val;
        }
        if(val == 0){
            out.ctype_write('0'_uni);
        }
        auto nval = static_cast<std::make_unsigned_t<I>>(val);
        do{
            stack.push_front(opt.convert_unit(nval));
            nval /= opt.base;
        }
        while(nval > 0);

        if(minus){
            out.ctype_write('-'_uni);
        }
        else if(opt.plus){
            out.ctype_write('+'_uni);
        }

        while(!stack.empty()){
            out.ctype_write(stack.front());
            stack.pop_front();
        }
    }

template<typename Stream, std::integral I>
void read_integer(Stream &in, I &val, uint base){
    val = 0;
    if(base < 2 || base > 36)
        return;
    bool sign = false;
    bool pos = true;
    unicode chr;
    while(true){
        try{
             chr = in.ctype_read_ghost();
        }
        catch(IOEOF &){ break;}
        if(chr == '+'_uni || chr == '-'_uni){
            if(sign)
                break;//already taken
            //accept it
            in.dis_char();
            sign = true;
            if(chr == '-'_uni)
                pos = false;
        }
        else{
            auto cnv = get_number<I>(chr, base);
            if(!cnv)
                break;
            //accept it
            in.dis_char();
            val = val * static_cast<I>(base) + cnv.get();
        }
    }
    if(!pos)
        val = -val;

}

