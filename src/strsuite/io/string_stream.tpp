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
void string_stream<T>::inc_siz(uint){
    //throw IOEOF{};
}

template<general_enctype T>
void string_stream<T>::inc_rem(size_t inc){
    buffer.exp_fit(buffer.dimension + inc);
    this->rebase(buffer.memory, buffer.dimension);
}

template<general_enctype T>
void string_stream<T>::discard() noexcept{
    len=0;
    this->discard_all();
}

template<general_enctype T>
template<typename IStream> requires read_char_stream<IStream, T>
uint string_stream<T>::get_char(IStream &stm){
    bool read=false;
    uint ret = 0;
    while(!read){
        try{
            uint red = stm.char_read(tchar_pt{this->base + this->las, format}, this->rem);
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
            uint red = stm.ghost_read(tchar_pt{this->base + this->las, format}, this->rem);
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
    uint ret = stm.char_write(const_tchar_pt{this->base + this->fir, format}, this->siz);
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
    this->discard_all();
    return ret;
}

template<general_enctype T>
template<read_byte_stream IBStream>
uint string_stream<T>::get_char_bytes(IBStream &stm, bool verify){
    uint get_siz = format.min_bytes();
    uint chr_siz = 0;
    uint by_read = 0;
    uint red = 0;

    this->force_rem(get_siz);
    red = force_byte_read(stm, this->base + this->las, get_siz);
    by_read += red;

    bool ended = false;
    do{
        try{
            chr_siz = format.chLen(this->base + this->las, by_read);
            ended = true;
        }
        catch(buffer_small &e){
            get_siz = e.get_required_size();
            this->force_rem(by_read + get_siz);
            red = force_byte_read(stm, this->base + this->las + by_read, get_siz);
            by_read += red;
        }
    }
    while(!ended);

    if(chr_siz > by_read){
        this->force_rem(chr_siz);
        red = force_byte_read(stm, this->base + this->las + by_read, chr_siz - by_read);
        by_read += red;
    }

    if(!verify || format.validChar(this->base + this->las, by_read)){
        this->raw_las_step(chr_siz);
        len++;
        return chr_siz;
    }
    else
        throw IOFail{"Invalid character encoding"};
}

template<general_enctype T>
template<write_byte_stream OBStream>
uint string_stream<T>::put_char_bytes(OBStream &stm){
    if(len == 0)
        throw IOEOF{};
    uint ret = this->get_chLen(format);

    uint eff = force_byte_write(stm, this->base + this->fir, ret);
    len--;
    this->raw_fir_step(eff);
    return eff;
}

template<general_enctype T>
template<write_byte_stream OBStream>
void string_stream<T>::put_all_char_bytes(OBStream &stm){
    if(len > 0){
        force_byte_write(stm, this->base + this->fir, this->siz);
        len=0;
    }
    this->discard_all();
}

template<general_enctype T>
template<general_enctype S>
uint string_stream<T>::char_read(tchar_pt<S> ptrS, size_t tsiz){
    tchar_pt<T> ptr = inv_rebase_pointer(ptrS, format);
    if(len == 0)
        throw IOEOF{};
    uint ret = this->get_chLen(format);
    if(tsiz < ret)
        throw IOBufsmall{ret - static_cast<uint>(tsiz)};
    copy_bytes(ptr.data(), this->base + this->fir, ret);
    len--;
    this->raw_fir_step(ret);
    return ret;
}

template<general_enctype T>
template<general_enctype S>
uint string_stream<T>::ghost_read(tchar_pt<S> ptrS, size_t tsiz){
    tchar_pt<T> ptr = inv_rebase_pointer(ptrS, format);
    if(len == 0)
        throw IOEOF{};
    uint ret = this->get_chLen(format);
    if(tsiz < ret)
        throw IOBufsmall{ret - static_cast<uint>(tsiz)};
    copy_bytes(ptr.data(), this->base + this->fir, ret);
    return ret;
}

template<general_enctype T>
uint string_stream<T>::char_write_0(const byte *ptr, size_t tsiz){
    uint chsi;
    try{
        chsi = format.chLen(ptr, tsiz);
    }
    catch(buffer_small &e){
        throw IOBufsmall{e};
    }
    if(chsi > tsiz)
        throw IOBufsmall{chsi - static_cast<uint>(tsiz)};
    this->force_rem(chsi);
    copy_bytes(this->base + this->las, ptr, chsi);
    len++;
    this->raw_las_step(chsi);
    return chsi;
}

template<general_enctype T>
template<general_enctype S>
size_t string_stream<T>::string_write(const adv_string_view<S> &strS){
    adv_string_view<T> str = strS.rebase(format);
    size_t chsi = str.size();
    this->force_rem(chsi);
    copy_bytes(this->base + this->las, str.data(), chsi);
    len += str.length();
    this->raw_las_step(chsi);
    return chsi;
}

template<general_enctype T>
template<general_enctype R>
uint string_stream<T>::char_write(const_tchar_pt<R> pt, size_t buf){
    if constexpr(strong_enctype<R> && strong_enctype<T>){
        if constexpr(is_base_for<R, T>)
            return char_write_0(pt.data(), buf);
        else
            return char_write_conv_0(pt, buf);
    }
    else{
        if(pt.raw_format().base_for(format))
            return char_write_0(pt.data(), buf);
        else
            return char_write_conv_0(pt, buf);
    }
}

template<general_enctype T>
template<general_enctype R>
uint string_stream<T>::char_write_conv_0(const_tchar_pt<R> pt, size_t buf){
    auto assume = pt.decode(buf);
    ctype temp = get_chr_el(assume);
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
void string_stream<T>::dis_char(){
    if(len == 0)
        return;
    uint in = this->get_chLen(format);
    len--;
    this->raw_fir_step(in);
}
/*
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
*/

template<general_enctype T>
uint string_stream<T>::ctype_write(const ctype &c){
    uint ret;
    bool ext=false;
    do{
        try{
            ret = format.encode(c, this->base + this->las, this->rem);
            ext = true;
        }
        catch(buffer_small &e){
            this->force_rem(this->rem + e.get_required_size());
        }
    }
    while(!ext);
    len++;
    this->raw_las_step(ret);
    return ret;
}

template<general_enctype T>
string_stream<T>::ctype string_stream<T>::ctype_read(){
    if(len == 0)
        throw IOEOF{};
    tuple_ret<ctype> r = format.decode(this->base + this->fir, this->siz);
    len--;
    this->raw_fir_step(get_len_el(r));
    return get_chr_el(r);
}

template<general_enctype T>
string_stream<T>::ctype string_stream<T>::ctype_read_ghost(){
    if(len == 0)
        throw IOEOF{};
    tuple_ret<ctype> r = format.decode(this->base + this->fir, this->siz);
    //len--;
    //this->raw_fir_step(get_len_el(r));
    return get_chr_el(r);
}

template<general_enctype T>
feat::Proxy_wrapper_ctype<T> string_stream<T>::light_ctype_read() requires strong_enctype<T>{
    if(len == 0)
        throw IOEOF{};
    tuple_ret<feat::Proxy_wrapper_ctype<T>> r = feat::Proxy_wrapper<T>::light_decode(this->base + this->fir, this->siz);
    len--;
    this->raw_fir_step(get_len_el(r));
    return get_chr_el(r);
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
    adv_string_view<T> vw = direct_build(const_tchar_pt{this->base + this->fir, format}, len, this->siz);
    return adv_string<T>{vw, res};//Make a copy
}

template<general_enctype T>
template<general_enctype S>
bool string_stream<T>::opt_cut_endl(const adv_string_view<S> &en){
    if(view().endsWith(en)){
        this->cut_ending(en.size());
        len -= en.length();
        return true;
    }
    else return false;
}


