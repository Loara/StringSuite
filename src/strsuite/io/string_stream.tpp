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
/*
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
*/

template<general_enctype T>
void string_stream<T>::inc_siz(uint){
    throw IOEOF{};
}

template<general_enctype T>
void string_stream<T>::inc_rem(size_t inc){
    buffer.exp_fit(buffer.dimension + inc);
    this->rebase(buffer.memory, buffer.dimension);
}

template<general_enctype T>
template<read_char_stream<T> IStream>
uint string_stream<T>::get_char(IStream &stm){
    bool read=false;
    uint ret = 0;
    while(!read){
        try{
            uint red = stm.char_read(this->set_las_as(format), this->rem);
            this->raw_las_step(red);
            len++;
            read=true;
        }
        catch(IOBufsmall &e){
            if(this->fir != 0)
                this->rewind();
            else
                inc_rem(e.get_required_size());
        }
    }
    return ret;
}
template<general_enctype T>
template<read_char_stream<T> IStream>
uint string_stream<T>::get_ghost(IStream &stm){
    bool read=false;
    uint ret = 0;
    while(!read){
        try{
            uint red = stm.ghost_read(this->set_las_as(format), this->rem);
            this->raw_las_step(red);
            len++;
            read=true;
        }
        catch(IOBufsmall &e){
            if(this->fir != 0)
                this->rewind();
            else
                inc_rem(e.get_required_size());
        }
    }
    return ret;
}

template<general_enctype T>
template<write_char_stream<T> OStream>
uint string_stream<T>::put_char(OStream &stm){
    if(len == 0)
        throw IOEOF{};
    uint ret = stm.char_write(this->get_fir_as(format), this->siz);
    this->raw_fir_step(ret);
    len--;
    return ret;
}

template<general_enctype T>
template<write_char_stream<T> OStream>
size_t string_stream<T>::put_all(OStream &stm){
    if(len == 0)
        throw IOEOF{};
    adv_string_view<T> data = view();
    size_t ret = stm.string_write(data);
    len=0;
    this->base_flush();
    return ret;
}

template<general_enctype T>
uint string_stream<T>::do_char_read(tchar_pt<T> ptr, size_t tsiz){
    if(len == 0)
        throw IOEOF{};
    uint ret = this->get_chLen(format);
    if(tsiz < ret)
        throw IOBufsmall{ret - static_cast<uint>(tsiz)};
    std::memcpy(ptr.data(), this->base + this->fir, ret);
    len--;
    this->raw_fir_step(ret);
    return ret;
}

template<general_enctype T>
uint string_stream<T>::do_ghost_read(tchar_pt<T> ptr, size_t tsiz){
    if(len == 0)
        throw IOEOF{};
    uint ret = this->get_chLen(format);
    if(tsiz < ret)
        throw IOBufsmall{ret - static_cast<uint>(tsiz)};
    std::memcpy(ptr.data(), this->base + this->fir, ret);
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
    this->force_frspc(chsi);
    std::memcpy(this->base + this->las, ptr.data(), chsi);
    len++;
    this->raw_las_step(chsi);
    return chsi;
}

template<general_enctype T>
size_t string_stream<T>::do_string_write(const adv_string_view<T> &str){
    size_t chsi = str.size();
    this->force_frspc(chsi);
    std::memcpy(this->base + this->las, str.data(), chsi);
    len += str.length();
    this->raw_las_step(chsi);
    return chsi;
}

template<general_enctype T>
template<general_enctype R>
uint string_stream<T>::char_write_conv(const_tchar_pt<R> pt, size_t buf){
    if((pt.raw_format().base_for(format)))
        return this->char_write(pt, buf);
    ctype temp;
    pt.decode(&temp, buf);
    uint ret;
    bool enc=false;
    do{
        try{
            ret = format.encode(temp, this->base + this->las, this->rem);
            enc=true;
        }
        catch(buffer_small &e){
            inc_rem(e.get_required_size());
        }
    }
    while(!enc);
    len++;
    this->raw_las_step(ret);
    return ret;
}

template<general_enctype T>
template<general_enctype R>
size_t string_stream<T>::string_write_conv(const adv_string_view<R> &str){
    if((str.raw_format().base_for(format)))
        return this->string_write(str);
    ctype temp;
    const_tchar_pt<R> pt = str.begin();
    size_t xl = str.length();
    size_t xs = str.size();

    uint inc = 0;
    size_t wrby=0;
    bool enc;
    for(size_t i=0; i<xl; i++){
        pt.decode_next_update(&temp, xs);
        enc=false;
        do{
            try{
                inc = format.encode(temp, this->base + this->las, this->rem);
                enc=true;
            }
            catch(buffer_small &e){
                inc_rem(e.get_required_size());
            }
        }
        while(!enc);
        this->raw_las_step(inc);
        len++;
        wrby += inc;
    }
    return wrby;
}

template<general_enctype T>
uint string_stream<T>::char_write(const ctype &c){
    uint ret;
    bool ext=false;
    do{
        try{
            ret = format.encode(c, this->base + this->las, this->rem);
            ext = true;
        }
        catch(buffer_small &e){
            this->force_frspc(this->rem + e.get_required_size());
        }
    }
    while(!ext);
    len++;
    this->raw_las_step(ret);
    return ret;
}

template<general_enctype T>
adv_string<T> string_stream<T>::move(){
    this->rewind();
    basic_ptr res = std::move(buffer);
    buffer.leave();
    size_t rsiz = this->siz, rlen = len;
    len = 0;
    this->reset();
    return direct_build_dyn(std::move(res), rlen, rsiz, format);
}

template<general_enctype T>
adv_string<T> string_stream<T>::allocate_new(std::pmr::memory_resource *res) const{
    adv_string_view<T> vw = direct_build(this->get_fir_as(format), len, this->siz);
    return adv_string<T>{vw, res};//Make a copy
}


