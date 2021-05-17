
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
void deduce_lens(const_tchar_pt<T> ptr, size_t &len, size_t &siz, const terminate_func<T> &terminate){
	len=0;
	siz=0;
	int add;

	while(!terminate(ptr.data(), ptr.raw_format())){
		add = ptr.next();
		siz += add;
		len++;
	}
}

template<typename T>
void deduce_lens(const_tchar_pt<T> ptr, size_t dim, meas measure, size_t &len, size_t &siz){
	len = 0;
	siz = 0;
	bool issiz = measure == meas::size;
	if constexpr(fixed_size<T>){
		if(issiz){
			len = dim / T::unity();
		}
		else{
			len = dim;
		}
		siz = len * T::unity();
	}
	else{
		int add;
		size_t oldim;

		if(issiz){
			try{
				add = ptr.next();
				oldim = dim;
				dim -= add;
				while(dim < oldim){//Overflow test for unsigned integer
					siz += add;
					len++;
					if(dim == 0)
						break;
					add = ptr.next();
					oldim = dim;
					dim -= add;
				}
			}
			catch(const encoding_error &){}
		}
		else{
			try{
				for(size_t i=0; i<dim; i++){
					add = ptr.next();
					siz += add;
					len++;
				}
			}
			catch(const encoding_error &){}
		}
	}
}
//-----------------------
template<typename T>
adv_string_view<T>::adv_string_view(const_tchar_pt<T> cu, const terminate_func<T> &terminate) : ptr{cu}, len{0}, siz{0}{
	deduce_lens(cu, len, siz, terminate);
}

template<typename T>
adv_string_view<T>::adv_string_view(const_tchar_pt<T> cu, size_t dim, meas isdim) : ptr{cu}, len{0}, siz{0}{
	deduce_lens(cu, dim, isdim, len, siz);
}

template<typename T>
adv_string_view<T>::adv_string_view(const_tchar_pt<T> cu, size_t size, size_t lent) : ptr{cu}, len{0}, siz{0}{
	if constexpr(fixed_size<T>){
		if(size / T::unity() < lent)
			throw encoding_error("Too small string");//prevent integer overflow due to multiplication
		len = lent;
		siz = lent * T::unity(); //may be siz < size
	}
	else{
		int add;
		size_t olsiz;
		for(size_t i=0; i<lent; i++){
			add = cu.next();
			olsiz = size;
			size -= add;
			siz += add;
			if(size >= olsiz)
				throw encoding_error("Too small string");
		}
		len = lent;
	}
}

template<typename T>
void adv_string_view<T>::verify() const{
	size_t remlen = siz;
	const_tchar_pt<T> mem{ptr};
	for(size_t i=0; i<len; i++){
		if(!mem.valid_next(remlen))
			throw encoding_error("Invalid string encoding");
	}
	//La lunghezza deve essere esatta
	if(remlen != 0)
		throw encoding_error("Invalid string encoding");
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
	if constexpr(fixed_size<T>){
		return ptr + (chr * T::unity());
	}
	else{
		const_tchar_pt<T> ret = ptr;
		if(chr == len)
			return ret + siz;
		for(size_t i=0; i< chr; i++)
			ret.next();
		return ret;
	}
};

template<typename T>
size_t adv_string_view<T>::size(size_t a, size_t n) const{
	if(a+n < n || a+n > len)
		throw std::out_of_range{"Out of range"};
	if(n == 0)
		return 0;
	if constexpr (fixed_size<T>){
		return n * T::unity();
	}
	else{
		const_tchar_pt<T> mem = ptr;
		for(size_t i=0; i<a; i++)
			mem.next();
		size_t ret = 0;
		for(size_t i=0; i<n; i++){
			ret += mem.next();
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
	if constexpr(fixed_size<T>){
		const_tchar_pt<T> nei = ptr + (b * T::unity());
		return adv_string_view<T>{e-b, (e-b) * T::unity(), nei};
	}
	else{
		const_tchar_pt<T> nei = ptr;
		for(size_t i=0; i<b; i++)
			nei.next();
		size_t nlen = 0;
		const_tchar_pt<T> temp = nei;
		for(size_t i=0; i<(e-b); i++)
			nlen += temp.next();
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
bool adv_string_view<T>::operator==(const_tchar_pt<S> t) const{
	if(!sameEnc(ptr, t))
		return false;
	size_t tsiz, tch;
	deduce_lens(t, tch, tsiz);
	if(siz != tsiz)
		return false;
	return compare(data(), t.data(), siz);
}

template<typename T> template<general_enctype S>
size_t adv_string_view<T>::bytesOf(const adv_string_view<S> &sq, bool &found) const{
	if(!sameEnc(ptr, sq.begin())){
		found = false;
		return 0;
	}

	if(sq.size() == 0){
		found = true;
		return 0;
	}
	if(siz < sq.size()){
		found = false;
		return 0;
	}
	size_t rem = siz - sq.size();
	size_t byt = 0;
	const_tchar_pt<T> newi = ptr;
	while(byt <= rem){
		if(compare(newi.data(), sq.begin().data(), sq.size())){
			found = true;
			return byt;
		}
		byt += newi.next();
	}
	found = false;
	return 0;
}

template<typename T> template<general_enctype S>
size_t adv_string_view<T>::indexOf(const adv_string_view<S> &sq, bool &found) const{
	if(!sameEnc(ptr, sq.begin())){
		found = false;
		return 0;
	}

	if(sq.size() == 0){
		found = true;
		return 0;
	}
	if(siz < sq.size()){
		found = false;
		return 0;
	}
	size_t rem = siz - sq.size();
	size_t byt = 0;
	size_t chr = 0;
	const_tchar_pt<T> newi = ptr;
	while(byt <= rem){
		if(compare(newi.data(), sq.begin().data(), sq.size())){
			found = true;
			return chr;
		}
		byt += newi.next();
		chr++;
	}
	found = false;
	return 0;
}

template<typename T> template<general_enctype S>
bool adv_string_view<T>::containsChar(const_tchar_pt<S> cu) const{
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
}
//----------------------------------------------
template<typename T>
uint adv_string_buf<T>::append_chr(const_tchar_pt<T> ptr){
	uint chl = ptr.chLen();
	const byte *dat = ptr.data();
	append(buffer, siz, dat, chl);
	siz += chl;
	len++;
	return chl;
}

template<typename T>
size_t adv_string_buf<T>::append_chrs(const_tchar_pt<T> ptr, size_t nchr){
	size_t part = 0;
	for(size_t i=0; i<nchr; i++){
		part += append_chr(ptr);
		ptr.next();
	}
	return part;
}

template<typename T>
size_t adv_string_buf<T>::append_string(adv_string_view<T> str){
	size_t ret = str.size();
	const byte *ptr = str.data();
	append(buffer, siz, ptr, ret);
	siz += ret;
	len += str.length();
	return ret;
} 

template<typename T>
bool adv_string_buf<T>::append_chr_v(const_tchar_pt<T> ptr, size_t siz){
	uint chlen;
	if(!ptr.validChar(chlen))
		return false;
	else if(siz < chlen)
		return false;
	const byte *dat = ptr.data();
	append(buffer, siz, dat, chlen);
	siz += chlen;
	len++;
	return true;
}

template<typename T>
bool adv_string_buf<T>::append_chrs_v(const_tchar_pt<T> ptr, size_t siz, size_t nchr){
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
}

template<typename T>
template<typename S>
size_t adv_string_buf<T>::append_string_c(adv_string_view<S> str){
	static_assert(std::is_same_v<typename T::ctype, typename S::ctype>, "Impossible to convert this string");
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
		from.decode(&tempo, from_r);
		bool written=false;
		while(!written){
			try{
				to.encode(tempo, to_r);
				written=true;
			}
			catch(const buffer_small &err){
                size_t olddim = buffer.dimension; //total occupied memory
                buffer.exp_fit(olddim + err.get_required_size() +1);
                to_r += buffer.dimension - olddim;
                base = base.new_instance(buffer.memory);
			}
		}
		from_r -= from.next();
		uint wrt = to.next();
		to_r -= wrt;
		siz += wrt;
		return_r += wrt;
		len ++;
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

template<typename T>
adv_string<T> adv_string<T>::newinstance_ter(const_tchar_pt<T> pt, const terminate_func<T> &terminate, std::pmr::memory_resource *alloc){
	size_t len=0, siz=0;
	deduce_lens(pt, len, siz, terminate);
	return adv_string<T>{pt, len, siz, basic_ptr{pt.data(), (std::size_t)siz, alloc}, 0};
}



