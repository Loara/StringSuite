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
	return direct_build<T>(const_tchar_pt<T>{buffer.memory, ei}, len, siz);
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
	adv_string_view<T> rey = direct_build<T>(const_tchar_pt<T>{buffer.memory, ei}, len, siz);
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



