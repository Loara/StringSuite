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
void string_stream<T>::rewind(){
    if(fir.difff() == 0)
        return;
    buffer.shift(fir.difff(), siz);
    fir.set_diff(0);
    las.set_diff(siz);
}
template<general_enctype T>
void string_stream<T>::increase(uint inc){
    buffer.exp_fit(buffer.dimension + inc);
    base = base.new_instance(buffer.memory);
}

template<general_enctype T>
tchar_pt<T> string_stream<T>::reserve_space(size_t needed){
    buffer.exp_fit(las.difff() + needed);
    base = base.new_instance(buffer.memory);
    return las.convert();
}

template<general_enctype T>
template<general_enctype S>
uint string_stream<T>::get_char(CharIStream<S> &stm){
    bool read=false;
    uint ret = 0;
    while(!read){
        try{
            stm.char_read(las.convert(), remaining());
            ret = las.next(remaining());
            len++;
            siz += ret;
            read=true;
        }
        catch(IOBufsmall &e){
            if(fir.difff() != 0)
                rewind();
            else
                increase(e.get_required_size());
        }
    }
    return ret;
}
template<general_enctype T>
template<general_enctype S>
uint string_stream<T>::get_ghost(CharIStream<S> &stm){
    bool read=false;
    uint ret = 0;
    while(!read){
        try{
            stm.ghost_read(las.convert(), remaining());
            ret = las.next(remaining());
            len++;
            siz += ret;
            read=true;
        }
        catch(IOBufsmall &e){
            if(fir.difff() != 0)
                rewind();
            else
                increase(e.get_required_size());
        }
    }
    return ret;
}

template<general_enctype T>
template<general_enctype S>
uint string_stream<T>::put_char(CharOStream<S> &stm){
    if(len == 0)
        throw IOEOF{};
    stm.char_write(fir.convert().cast(), siz);
    uint ret = fir.next(siz);
    len--;
    siz -= ret;
    return ret;
}

template<general_enctype T>
template<general_enctype S>
size_t string_stream<T>::put_all(CharOStream<S> &stm){
    if(len == 0)
        throw IOEOF{};
    adv_string_view<T> data = view();
    size_t ret = stm.string_write(data);
    siz=0;
    len=0;
    fir.set_diff(0);
    las.set_diff(0);
    return ret;
}

template<general_enctype T>
template<general_enctype S>
size_t string_stream<T>::put_chars(CharOStream<S> &stm, size_t nchr){
    if(len < nchr )
        throw IOEOF{};
    size_t ret = stm.chars_write(fir.convert().cast(), siz, nchr);
    fir += ret;
    len -= nchr;
    siz -= ret;
    return ret;
}

template<general_enctype T>
uint string_stream<T>::do_char_read(tchar_pt<T> ptr, size_t tsiz){
    if(len == 0)
        throw IOEOF{};
    uint ret = fir.chLen(siz);
    if(tsiz < ret)
        throw IOBufsmall{ret - static_cast<uint>(tsiz)};
    std::memcpy(ptr.data(), fir.data(), ret);
    len--;
    siz -= ret;
    fir += ret;
    return ret;
}

template<general_enctype T>
uint string_stream<T>::do_ghost_read(tchar_pt<T> ptr, size_t tsiz){
    if(len == 0)
        throw IOEOF{};
    uint ret = fir.chLen(siz);
    if(tsiz < ret)
        throw IOBufsmall{ret - static_cast<uint>(tsiz)};
    std::memcpy(ptr.data(), fir.data(), ret);
    /*
    len--;
    siz -= ret;
    fir += ret;
    Nope
    */
    return ret;
}

template<general_enctype T>
uint string_stream<T>::do_char_write(const_tchar_pt<T> ptr, size_t tsiz){
    uint chsi;
    try{
        chsi = ptr.chLen(tsiz);
    }
    catch(buffer_small &e){
        throw IOBufsmall{e};
    }
    if(chsi > tsiz)
        throw IOBufsmall{chsi - static_cast<uint>(tsiz)};
    while(remaining() < chsi){
        if(fir.difff() != 0)
            rewind();
        else
            increase(chsi - static_cast<uint>(remaining()));
    }
    std::memcpy(las.data(), ptr.data(), chsi);
    las += chsi;
    len++;
    siz += chsi;
    return chsi;
}

template<general_enctype T>
size_t string_stream<T>::do_string_write(const adv_string_view<T> &str){
    size_t chsi = str.size();
    while(remaining() < chsi){
        if(fir.difff() != 0)
            rewind();
        else
            increase(chsi - static_cast<uint>(remaining()));
    }
    std::memcpy(las.data(), str.data(), chsi);
    las += chsi;
    len += str.length();
    siz += chsi;
    return chsi;
}

template<general_enctype T>
template<general_enctype R>
uint string_stream<T>::char_write_conv(const_tchar_pt<R> pt, size_t buf){
    if((pt.raw_format().base_for(base.raw_format())))
        return this->char_write(pt, buf);
    ctype temp;
    pt.decode(&temp, buf);
    uint ret;
    bool enc=false;
    do{
        try{
            ret = las.encode_next(temp, remaining());
            enc=true;
        }
        catch(buffer_small &e){
            increase(e.get_required_size());
        }
    }
    while(!enc);
    siz += ret;
    len++;
    return ret;
}

template<general_enctype T>
template<general_enctype R>
size_t string_stream<T>::string_write_conv(const adv_string_view<R> &str){
    if((str.raw_format().base_for(base.raw_format())))
        return this->string_write(str);
    ctype temp;
    const_tchar_pt<R> pt = str.begin();
    size_t xl = str.length();
    size_t xs = str.size();

    /*
     * Temporarly data
     * since in case of fatal error no byte should be permanently written
     */
    tchar_relative<T> dest{las};
    size_t wrby=0;

    uint inc = 0;
    bool enc;
    for(size_t i=0; i<xl; i++){
        pt.decode_next_update(&temp, xs);
        enc=false;
        do{
            try{
                inc = dest.encode_next(temp, remaining());
                enc=true;
            }
            catch(buffer_small &e){
                increase(e.get_required_size());
            }
        }
        while(!enc);
        wrby += inc;
    }
    las.set_diff(dest.difff());
    siz += wrby;
    len += xl;
    return wrby;
}

template<general_enctype T>
adv_string<T> string_stream<T>::move(){
    rewind();
    basic_ptr res = std::move(buffer);
    buffer.leave();
    size_t rsiz = siz, rlen = len;
    siz = 0;
    len = 0;
    fir.set_diff(0);
    las.set_diff(0);
    base = base.new_instance(nullptr);
    return direct_build_dyn(std::move(res), rlen, rsiz, base.raw_format());
}

template<general_enctype T>
adv_string<T> string_stream<T>::allocate_new(std::pmr::memory_resource *res) const{
    adv_string_view<T> vw = direct_build(fir.convert().cast(), len, siz);
    return adv_string<T>{vw, res};//Make a copy
}


