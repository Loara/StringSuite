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
dimensions deduce_lens(const_tchar_pt<T> ptr, size_t maxsiz){
    dimensions ret{};
	uint add=0;

	while(maxsiz > 0){
        try{
            add = ptr.next_update(maxsiz);
            ret.siz += add;
            ret.len++;
        }
        catch(buffer_small &){break;}
	}
	return ret;
}

template<typename T, typename FuncType>
dimensions deduce_lens(const_tchar_pt<T> ptr, size_t maxsiz, const FuncType &terminate){
    dimensions ret{};
	uint add=0;

	while(maxsiz > 0 && !terminate(ptr.data(), ptr.raw_format(), maxsiz)){
        try{
            add = ptr.next_update(maxsiz);
            ret.siz += add;
            ret.len++;
        }
        catch(buffer_small &){break;}
	}
	return ret;
}

template<typename T>
dimensions deduce_lens(const_tchar_pt<T> ptr, size_t maxsiz, size_t chMax){
	dimensions ret{};
	if constexpr(feat::fixed_size<T>::value){
        /*
         * Let i the max integer such that T::min_bytes() * i <= maxsiz
         */
        size_t i = maxsiz / T::min_bytes();
        ret.len = i;
        ret.siz = T::min_bytes() * i;
	}
	else{
        uint add=0;
        while(maxsiz > 0 && chMax > 0){
            try{
                add = ptr.next_update(maxsiz);
                ret.siz += add;
                ret.len++;
                chMax--;
            }
            catch(buffer_small &){break;}
        }
	}
	return ret;
}

//-----------------------

template<typename T>
adv_string_view<T>::adv_string_view(const_tchar_pt<T> cu, size_t maxsiz) : ptr{cu}, len{0}, siz{0}{
	dimensions d = deduce_lens(cu, maxsiz);
    len = d.len;
    siz = d.siz;
}

template<typename T>
template<typename FuncType>
adv_string_view<T>::adv_string_view(const_tchar_pt<T> cu, size_t maxsiz, const FuncType &terminate) : ptr{cu}, len{0}, siz{0}{
    static_assert(is_terminate_func<FuncType, T>, "Not a terminate function");
	dimensions d = deduce_lens(cu, maxsiz, terminate);
    len = d.len;
    siz = d.siz;
}

template<typename T>
adv_string_view<T>::adv_string_view(const_tchar_pt<T> cu, size_t maxsiz, size_t maxlen) : ptr{cu}, len{0}, siz{0}{
	dimensions d = deduce_lens(cu, maxsiz, maxlen);
    len = d.len;
    siz = d.siz;
}

template<typename T>
void adv_string_view<T>::validate(const placeholder &plc) const{
    if(plc.start != ptr.data())
        throw invalid_placeholder{};
}

template<typename T>
const byte * adv_string_view<T>::placeholder::data() const noexcept{
    return start + siz;
}
template<typename T>
size_t adv_string_view<T>::placeholder::nbytes() const noexcept{
    return siz;
}
template<typename T>
size_t adv_string_view<T>::placeholder::nchr() const noexcept{
    return len;
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

template<general_enctype T>
template<general_enctype S>
constexpr bool adv_string_view<T>::can_rebase(EncMetric_info<S> o) const noexcept{
    return can_rebase_pointer(ptr, o);
}

template<general_enctype T>
template<general_enctype S>
adv_string_view<S> adv_string_view<T>::rebase(EncMetric_info<S> o) const{
    return direct_build(rebase_pointer(ptr, o), len, siz);
}

template<typename T>
adv_string_view<T>::placeholder adv_string_view<T>::select(size_t chr, bool exc) const{
    const byte *dat = ptr.data();
	if(chr >= len){
        if(exc && chr > len)
            throw out_of_range{"Past to end"};
        else
            return placeholder{dat, siz, len};
    }
	if(chr == 0){
		return placeholder{dat, 0, 0};
    }
	if(ptr.is_fixed()){
		return placeholder{dat, chr * ptr.raw_format().min_bytes(), chr};
	}
	else{
		const_tchar_pt<T> mem = ptr;
		size_t ret = 0;
		for(size_t i=0; i< chr; i++)
			ret += mem.next(siz);
		return placeholder{dat, ret, chr};
	}
}

template<typename T>
adv_string_view<T>::placeholder adv_string_view<T>::select(const placeholder &base, size_t nchr, bool exc) const{
    validate(base);
    size_t totalchr = base.len + nchr;
    const byte *dat = ptr.data();
    if(totalchr >= len){
        if(exc && totalchr > len)
            throw out_of_range{"Past to end"};
        else
            return placeholder{dat, siz, len};
    }
	if(nchr == 0){
		return base;
    }
	if(ptr.is_fixed()){
		return placeholder{dat, base.siz + nchr * ptr.raw_format().min_bytes(), base.len + nchr};
	}
	else{
		const_tchar_pt<T> mem = ptr.new_instance(base.data());
		size_t ret = 0;
		for(size_t i=0; i< nchr; i++)
			ret += mem.next(siz);
		return placeholder{dat, base.siz + ret, base.len + nchr};
	}
}

template<typename T>
adv_string_view<T>::placeholder adv_string_view<T>::select_begin() const noexcept{
    return placeholder{ptr.data(), 0, 0};
}
template<typename T>
adv_string_view<T>::placeholder adv_string_view<T>::select_end() const noexcept{
    return placeholder{ptr.data(), siz, len};
}

template<typename T>
const_tchar_pt<T> adv_string_view<T>::at(placeholder plc) const{
    validate(plc);
    return ptr + plc.siz;
};

template<typename T>
size_t adv_string_view<T>::size(size_t a, size_t n) const{
    placeholder b = select(a);
    placeholder e = select(b, n);
    return e.siz - b.siz;
    /*
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
	*/
}

template<typename T>
adv_string_view<T> adv_string_view<T>::substring(placeholder b, placeholder e) const{
    validate(b);
    validate(e);
    //if(e > select_end())
    //    e = select_end();
    //useless
    if(b > e)
        b = e;
    return adv_string_view<T>{e.len - b.len, e.siz - b.siz, at(b)};
}

template<typename T>
template<general_enctype S>
bool adv_string_view<T>::equal_to(const adv_string_view<S> &t, size_t ch) const{
	if(!raw_format().equalTo(t.raw_format()))
		return false;
	size_t l1 = size(ch);
	size_t l2 = t.size(ch);
	if(l1 != l2)
		return false;
	return compare(data(), t.data(), l1);
}

template<typename T>
template<general_enctype S>
std::weak_ordering adv_string_view<T>::operator<=>(const adv_string_view<S> &t) const{
    size_t mins = siz > t.size() ? t.size() : siz;
    int cmp = 0;
    if(mins > 0)
        cmp = std::memcmp(data(), t.data(), mins);
    if(cmp > 0)
        return std::weak_ordering::greater;
    else if(cmp < 0)
        return std::weak_ordering::less;
    else{
        if(siz > t.size())
            return std::weak_ordering::greater;
        else if(siz < t.size())
            return std::weak_ordering::less;
        else
            return std::weak_ordering::equivalent;
    }
}

template<typename T>
template<general_enctype S>
index_result adv_string_view<T>::bytesOf(const adv_string_view<S> &sq) const{
	if(!sq.can_rebase(raw_format()))
		throw incorrect_encoding{"Impossible to perform encode rebase"};
	if(sq.size() == 0){
		return index_result{true, 0};
	}
	if(siz < sq.size()){
		return index_result{false, 0};
	}
	size_t rem = siz - sq.size();
	size_t byt = 0;
	const_tchar_pt<T> newi = ptr;
    if(raw_format().has_head()){
        uint hd = raw_format().head();
        while(byt <= rem){
            if(compare(newi.data(), sq.begin().data(), sq.size())){
                return index_result{true, byt};
            }
            newi += hd;
            byt += hd;
        }
    }
    else{
        while(byt <= rem){
            if(compare(newi.data(), sq.begin().data(), sq.size())){
                return index_result{true, byt};
            }
            byt += newi.next(rem - byt);
        }
    }
    return index_result{false, 0};
}

template<typename T>
template<general_enctype S>
index_result adv_string_view<T>::indexOf(const adv_string_view<S> &sq) const{
	if(!sq.can_rebase(raw_format()))
		throw incorrect_encoding{"Impossible to perform encode rebase"};
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
    /*
     * In this case we can't use the preceding optimization since we want to know
     * also the number of characters
     */
	while(byt <= rem){
		if(compare(newi.data(), sq.begin().data(), sq.size())){
            return index_result{true, chr};
		}
		byt += newi.next(siz);
		chr++;
	}
    return index_result{false, 0};
}

template<typename T>
template<general_enctype S>
adv_string_view<T>::placeholder adv_string_view<T>::placeOf(const adv_string_view<S> &sq) const{
	if(!sq.can_rebase(raw_format()))
		throw incorrect_encoding{"Impossible to perform encode rebase"};
	if(sq.size() == 0){
		return select_begin();
	}
	if(siz < sq.size()){
		return select_end();
	}
	size_t rem = siz - sq.size();
	size_t byt = 0;
	size_t chr = 0;
	const_tchar_pt<T> newi = ptr;
    const byte *data = ptr.data();
    /*
     * In this case we can't use the preceding optimization since we want to know
     * also the number of characters
     */
	while(byt <= rem){
		if(compare(newi.data(), sq.begin().data(), sq.size())){
            return placeholder{data, byt, chr};
		}
		byt += newi.next(siz);
		chr++;
	}
    return select_end();
}

template<typename T>
template<general_enctype S>
index_result adv_string_view<T>::containsChar(const adv_string_view<S> &cu) const{
	if(!cu.can_rebase(raw_format()))
		throw incorrect_encoding{"Impossible to perform encode rebase"};
    if(cu.length() == 0)
        return index_result{true, 0};
    adv_string_view<T> strip = cu.substring(0, 1);
    return indexOf(strip);
}

template<typename T>
template<general_enctype S>
bool adv_string_view<T>::startsWith(const adv_string_view<S> &sq) const{
	if(!sq.can_rebase(raw_format()))
		throw incorrect_encoding{"Impossible to perform encode rebase"};
	if(sq.size() == 0){
		return true;
	}
	if(siz < sq.size() || len < sq.length()){
		return false;
	}
	return compare(ptr.data(), sq.begin().data(), sq.size());
}

template<typename T>
template<general_enctype S>
bool adv_string_view<T>::endsWith(const adv_string_view<S> &sq) const{
	if(!sq.can_rebase(raw_format()))
		throw incorrect_encoding{"Impossible to perform encode rebase"};
	if(sq.size() == 0){
		return true;
	}
	if(siz < sq.size() || len < sq.length()){
		return false;
	}
	if(raw_format().has_head()){
        const_tchar_pt<T> poi = ptr + (siz - sq.size());
        return compare(poi.data(), sq.begin().data(), sq.size());
    }
    else{
        size_t discard = size(len - sq.length());
        const_tchar_pt<T> poi = ptr + discard;
        if(sq.size() != (siz - discard))
            return false;
        return compare(poi.data(), sq.begin().data(), sq.size());
    }
}

template<typename T>
template<general_enctype S>
conditional_result<typename adv_string_view<T>::placeholder> adv_string_view<T>::endsWith_placeholder(const adv_string_view<S> &sq) const{
	if(!sq.can_rebase(raw_format()))
		throw incorrect_encoding{"Impossible to perform encode rebase"};
	if(sq.size() == 0){
		return conditional_result{true, select_end()};
	}
	if(siz < sq.size() || len < sq.length()){
		return conditional_result{false, select_end()};
	}
	size_t psiz = siz - sq.size();
    size_t plen = len - sq.length();
	if(raw_format().has_head()){
        const byte *poi = ptr.data() + psiz;
        if(compare(poi, sq.begin().data(), sq.size())){
            return conditional_result{true, placeholder{ptr.data(), psiz, plen}};
        }
        else
            return conditional_result{false, select_end()};
    }
    else{
        placeholder pu = select(plen);
        if(pu.siz != psiz)
            return conditional_result{false, select_end()};
        if(compare(pu.data(), sq.begin().data(), sq.size()))
            return conditional_result{true, pu};
        else
            return conditional_result{false, select_end()};
    }
}

template<typename T>
adv_string_view<T>::ctype adv_string_view<T>::get_char(placeholder pch) const{
    if(pch == select_end())
        throw out_of_range{"Placeholder to end"};
    auto chr = ptr.new_instance(pch.data()).decode(siz - pch.siz);
    return get_chr_el(chr);
}

template<typename T>
template<typename Container>
void adv_string_view<T>::get_all_char(Container &cont) const{
    const_tchar_pt<T> mem = ptr;
    size_t rem = siz;
    for(size_t i=0; i< len; i++){
        auto ret = mem.decode_next_update(rem);
        cont.push_back(get_chr_el(ret));
    }
}

