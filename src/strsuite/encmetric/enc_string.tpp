
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
template<typename T>
void deduce_lens(const_tchar_pt<T> ptr, size_t maxsiz, size_t &len, size_t &siz, const terminate_func<T> &terminate){
	len=0;
	siz=0;
	uint add=0;

	while(maxsiz > 0 && !terminate(ptr.data(), ptr.raw_format(), maxsiz)){
        try{
            add = ptr.next_update(maxsiz);
            siz += add;
            len++;
        }
        catch(buffer_small &){break;}
	}
}

template<typename T>
void deduce_lens(const_tchar_pt<T> ptr, size_t maxsiz, size_t chMax, size_t &len, size_t &siz){
	len = 0;
	siz = 0;
	if constexpr(fixed_size<T>){
        /*
         * Let i the max integer such that T::min_bytes() * i <= maxsiz
         */
        size_t i = maxsiz / T::min_bytes();
        len = i;
        siz = T::min_bytes() * i;
	}
	else{
        uint add=0;
        while(maxsiz > 0 && chMax > 0){
            try{
                add = ptr.next_update(maxsiz);
                siz += add;
                len++;
                chMax--;
            }
            catch(buffer_small &){break;}
        }
	}
}
//-----------------------
template<typename T>
adv_string_view<T>::adv_string_view(const_tchar_pt<T> cu, size_t maxsiz, const terminate_func<T> &terminate) : ptr{cu}, len{0}, siz{0}{
	deduce_lens(cu, maxsiz, len, siz, terminate);
}

template<typename T>
adv_string_view<T>::adv_string_view(const_tchar_pt<T> cu, size_t maxsiz, size_t maxlen) : ptr{cu}, len{0}, siz{0}{
	deduce_lens(cu, maxsiz, maxlen, len, siz);
}

template<typename T>
void adv_string_view<T>::verify() const{
	size_t remlen = siz;
	const_tchar_pt<T> mem{ptr};
	for(size_t i=0; i<len; i++){
		if(!mem.valid_next_update(remlen))
			throw incorrect_encoding("Invalid string encoding");
	}
	if(remlen != 0)
		throw incorrect_encoding("Invalid string encoding");
}

template<typename T>
bool adv_string_view<T>::verify_safe() const noexcept{
	try{
		verify();
		return true;
	}
	catch(...){
		return false;
	}
}

template<typename T>
const_tchar_pt<T> adv_string_view<T>::at(size_t chr) const{
	if(chr > len)
		throw std::out_of_range{"Out of range"};
	if(chr == 0)
		return ptr;
	if(ptr.is_fixed()){
		return ptr + (chr * ptr.raw_format().min_bytes());
	}
	else{
		const_tchar_pt<T> ret = ptr;
		if(chr == len)
			return ret + siz;
		for(size_t i=0; i< chr; i++)
			ret.next(siz);
		return ret;
	}
};

template<typename T>
size_t adv_string_view<T>::size(size_t a, size_t n) const{
	if(a+n < n || a+n > len)
		throw std::out_of_range{"Out of range"};
	if(n == 0)
		return 0;
	if (ptr.is_fixed()){
		return n * ptr.raw_format().min_bytes();
	}
	else{
		const_tchar_pt<T> mem = ptr;
		for(size_t i=0; i<a; i++)
			mem.next(siz);
		size_t ret = 0;
		for(size_t i=0; i<n; i++){
			ret += mem.next(siz);
		}
		return ret;
	}
}

template<typename T>
adv_string_view<T> adv_string_view<T>::substring(size_t b, size_t e, bool ign) const{
	if(ign)
		e = len;
	else if(e > len)
		e = len;
	if(b > e)
		b = e;
	if(ptr.is_fixed()){
        uint cl = ptr.raw_format().min_bytes();
		const_tchar_pt<T> nei = ptr + (b * cl);
		return adv_string_view<T>{e-b, (e-b) * cl, nei};
	}
	else{
		const_tchar_pt<T> nei = ptr;
		for(size_t i=0; i<b; i++)
			nei.next(siz);
		size_t nlen = 0;
		const_tchar_pt<T> temp = nei;
		for(size_t i=0; i<(e-b); i++)
			nlen += temp.next(siz);
		return adv_string_view<T>{e - b, nlen, nei};
	}
}

template<typename T> template<general_enctype S>
bool adv_string_view<T>::equal_to(const adv_string_view<S> &t, size_t ch) const{
	if(!sameEnc(ptr, t.begin()))
		return false;
	size_t l1 = size(ch);
	size_t l2 = t.size(ch);
	if(l1 != l2)
		return false;
	return compare(data(), t.data(), l1);
}

template<typename T> template<general_enctype S>
bool adv_string_view<T>::operator==(const adv_string_view<S> &t) const{
	if(!sameEnc(ptr, t.begin()))
		return false;
	if(siz != t.size())
		return false;
    if(len != t.length())
        return false;
	return compare(data(), t.data(), siz);
}

template<typename T> template<general_enctype S>
index_result adv_string_view<T>::bytesOf(const adv_string_view<S> &sq) const{
	if(!sameEnc(ptr, sq.begin())){
		return index_result{false, 0};
	}

	if(sq.size() == 0){
		return index_result{true, 0};
	}
	if(siz < sq.size()){
		return index_result{false, 0};
	}
	size_t rem = siz - sq.size();
	size_t byt = 0;
	const_tchar_pt<T> newi = ptr;
	while(byt <= rem){
		if(compare(newi.data(), sq.begin().data(), sq.size())){
            return index_result{true, byt};
		}
		byt += newi.next(siz);
	}
    return index_result{false, 0};
}

template<typename T> template<general_enctype S>
index_result adv_string_view<T>::indexOf(const adv_string_view<S> &sq) const{
	if(!sameEnc(ptr, sq.begin())){
		return index_result{false, 0};
	}

	if(sq.size() == 0){
		return index_result{true, 0};
	}
	if(siz < sq.size()){
		return index_result{false, 0};
	}
	size_t rem = siz - sq.size();
	size_t byt = 0;
	size_t chr = 0;
	const_tchar_pt<T> newi = ptr;
	while(byt <= rem){
		if(compare(newi.data(), sq.begin().data(), sq.size())){
            return index_result{true, chr};
		}
		byt += newi.next(siz);
		chr++;
	}
    return index_result{false, 0};
}

template<typename T> template<general_enctype S>
bool adv_string_view<T>::containsChar(const adv_string_view<S> &cu) const{
    if(cu.length() == 0)
        return true;
    adv_string_view<S> strip = cu.substring(0, 1);
    return indexOf(strip);
    /*
	if(!sameEnc(ptr, cu))
		return false;

	size_t chl = cu.chLen();
	if(siz < chl)
		return false;
	size_t rem = siz - chl;
	size_t byt = 0;
	const_tchar_pt<T> newi = ptr;
	while(byt <= rem){
		if(compare(newi.data(), cu.data(), chl)){
			return true;
		}
		byt += newi.next();
	}
	return false;
    */
}

template<typename T> template<general_enctype S>
bool adv_string_view<T>::startsWith(const adv_string_view<S> &sq) const{
	if(!sameEnc(ptr, sq.begin())){
		return false;
	}
	
	if(sq.size() == 0){
		return true;
	}
	if(siz < sq.size()){
		return false;
	}
	return compare(ptr.data(), sq.begin().data(), sq.size());
}

template<typename T> template<general_enctype S>
bool adv_string_view<T>::endsWith(const adv_string_view<S> &sq) const{
	if(!sameEnc(ptr, sq.begin())){
		return false;
	}
	
	if(sq.size() == 0){
		return true;
	}
	if(siz < sq.size()){
		return false;
	}
	const_tchar_pt<T> poi = ptr + siz - sq.size();
	return compare(poi.data(), sq.begin().data(), sq.size());
}

template<typename T>
template<general_enctype S>
adv_string<T> adv_string_view<T>::concatenate(const adv_string_view<S> &err, std::pmr::memory_resource *alloc) const{
    adv_string_buf<T> buffer{*this, alloc};
    buffer.append_string_c(err);
    return buffer.move();
    /*
	if(!sameEnc(*this, err))
		throw encoding_error("Not same encoding");
	size_t esiz = err.size();
	size_t elen = err.length();
	const byte *buf1 = data();
	const byte *buf2 = err.data();
	basic_ptr allocater{(std::size_t)(siz+esiz), alloc};
	byte *buf = allocater.memory;
	for(size_t i=0; i<siz; i++)
		buf[i] = buf1[i];
	for(size_t j=0; j<esiz; j++)
		buf[j + siz] = buf2[j];
	return adv_string<T>{std::move(allocater), ptr, len+elen, siz+esiz};
    */
}


//----------------------------------------------


template<typename T>template<general_enctype S>
size_t adv_string_buf<T>::append_string(const adv_string_view<S> &str){
    if(!ei.equalTo(str.raw_format()))
        throw encoding_error{"Use append_string_c"};
	size_t ret = str.size();
	const byte *ptr = str.data();
	append(buffer, siz, ptr, ret);
	siz += ret;
	len += str.length();
	return ret;
} 

template<typename T>
bool adv_string_buf<T>::append_chr_v(const_tchar_pt<T> ptr, size_t psiz){
	validation_result valid = ptr.validChar(psiz);
	if(!valid)
		return false;
	const byte *dat = ptr.data();
	append(buffer, siz, dat, valid.get());
	siz += valid.get();
	len++;
	return true;
}

template<typename T>
bool adv_string_buf<T>::append_chrs_v(const_tchar_pt<T> ptr, size_t psiz, size_t nchr){
    size_t totalsize=0;
    const_tchar_pt<T> validation{ptr};
    for(size_t i=0; i < nchr; i++){
        validation_result valid = validation.valid_next_update(psiz);
        if(!valid)
            return false;
        totalsize += valid.get();
    }
    const byte *data = ptr.data();
    append(buffer, siz, data, totalsize);
    siz += totalsize;
    len += nchr;
    return true;
    /*
	uint lbuf;
	size_t siztotal=0;
	const_tchar_pt<T> verify = ptr;
	for(size_t i=0; i<nchr; i++){
		if(!verify.validChar(lbuf))
			return false;
		siztotal += lbuf;
		if(siztotal > siz)
			return false;
		verify.next();
	}
	const byte *lay=ptr.data();
	append(buffer, siz, lay, siztotal);
	siz += siztotal;
	len += nchr;
	return true;
    */
}

template<typename T>
template<general_enctype S>
size_t adv_string_buf<T>::append_string_c(const adv_string_view<S> &str){
	static_assert(std::is_same_v<typename T::ctype, typename S::ctype>, "Impossible to convert this string");
    if(ei.equalTo(str.raw_format()))
        return append_string(str);

	if(str.length() == 0)
		return 0;
	const_tchar_pt<S> from = str.begin();
	size_t from_r = str.size();

	size_t nchr=str.length();

	tchar_pt<T> base{buffer.memory, ei};
	size_t to_r = buffer.dimension -siz;
    tchar_relative<T> to{base, siz};

	size_t return_r = 0;

	typename T::ctype tempo{};

	for(size_t i=0; i<nchr; i++){
		from.decode_next_update(&tempo, from_r);
		bool written=false;
        uint wrt=0;
		while(!written){
			try{
				wrt=to.encode_next_update(tempo, to_r);
				written=true;
			}
			catch(const buffer_small &err){
                size_t olddim = buffer.dimension; //total occupied memory
                buffer.exp_fit(olddim + err.get_required_size() +1);
                to_r += buffer.dimension - olddim;
                base = base.new_instance(buffer.memory);
			}
		}
		siz += wrt;
		len ++;
		return_r += wrt;
	}
	return return_r;
}

template<typename T>
void adv_string_buf<T>::clear() noexcept{
	siz=0;
	len=0;
}

template<typename T>
adv_string_view<T> adv_string_buf<T>::view() const noexcept{
	return adv_string_view<T>{len, siz, const_tchar_pt<T>{buffer.memory, ei}};
}

template<typename T>
adv_string<T> adv_string_buf<T>::move(){
	size_t l=len;
	size_t s=siz;
	basic_ptr to = std::move(buffer);
	buffer.leave();
	len=0;
	siz=0;
	return adv_string<T>{const_tchar_pt<T>{nullptr, ei}, l, s, std::move(to), 0};
}

template<typename T>
adv_string<T> adv_string_buf<T>::allocate(std::pmr::memory_resource *all) const{
	adv_string_view<T> rey{len, siz, const_tchar_pt<T>{buffer.memory, ei}};
	return adv_string<T>{rey, all};
}

//----------------------------------------------

template<typename T>
adv_string<T>::adv_string(const_tchar_pt<T> ptr, size_t len, size_t siz, basic_ptr by) : adv_string_view<T>{len, siz, ptr}, bind{std::move(by)} {}

//ignore the memory pointed bu ptr, use the memory pointed by by
template<typename T>
adv_string<T>::adv_string(const_tchar_pt<T> ptr, size_t len, size_t siz, basic_ptr by, [[maybe_unused]] int ignore) : adv_string_view<T>{len, siz, ptr.new_instance(by.memory)}, bind{std::move(by)} {}


template<typename T>
adv_string<T>::adv_string(const adv_string_view<T> &st, std::pmr::memory_resource *alloc)
	 : adv_string{st.begin(), st.length(), st.size(), basic_ptr{st.data(), (std::size_t)st.size(), alloc}, 0} {}



