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
//-----------------------

template<typename T>
void adv_string_view<T>::step_internal() const{
    size_t rem = siz - ded_siz;
    auto p = ptr + ded_siz;
    uint csiz = p.chLen(rem);
    ded_siz += csiz;
    len++;
}
/*
template<typename T>
void adv_string_view<T>::deduce_len() const{
    while(!is_len_deduced())
        step_internal();
}
*/
template<typename T>
void adv_string_view<T>::set_length(size_t nlen) const noexcept{
    ded_siz = siz;
    len = nlen;
}
/*
template<typename T>
bool adv_string_view<T>::step_internal_v() const{
    if(is_len_deduced())
        return true;
    size_t rem = siz - ev.siz;
    auto p = ptr.new_instance(ev.data());
    validation_result cval = p.validChar(rem);
    if(!cval)
        return false;
    uint csiz = cval.get();
    ev.siz += csiz;
    ev.len++;
    if(rem == csiz)
        len = ev.len;
    return true;
}

template<typename T>
bool adv_string_view<T>::deduce_len_v() const{
    while(!is_len_deduced()){
        if(!step_internal_v())
            return false;
    }
    return true;
}
*/
template<typename T>
adv_string_view<T>::adv_string_view(const_tchar_pt<T> cu, size_t size) : ptr{cu}, siz{size}, len{0}, ded_siz{0}{
    if(ptr.is_fixed()){
        uint csiz = ptr.min_bytes();
        if(size % csiz != 0)
            throw incorrect_encoding{"Last character is truncated"};
        set_length(size / csiz);
    }
}

template<typename T>
template<typename FuncType>
adv_string_view<T>::adv_string_view(const_tchar_pt<T> cu, size_t maxsiz, const FuncType &terminate) : ptr{cu}, siz{0}, len{0}, ded_siz{0}{
    static_assert(is_terminate_func<FuncType, T>, "Not a terminate function");
    const_tchar_pt<T> dec = cu;
    while(maxsiz > 0){
        if(terminate(dec, maxsiz))
            break;
        uint csiz = dec.next_update(maxsiz);
        siz += csiz;
        len++;
    }
    ded_siz = siz;
}

/*
template<typename T>
adv_string_view<T>::adv_string_view(const_tchar_pt<T> cu, size_t maxsiz, size_t maxlen) : ptr{cu}, len{0}, siz{0}{
	dimensions d = deduce_lens(cu, maxsiz, maxlen);
    len = d.len;
    siz = d.siz;
}
*/

template<typename T>
void adv_string_view<T>::validate(const placeholder &plc) const{
    if(plc.start != ptr.data() || plc.siz > siz)
        throw invalid_placeholder{};
    if(plc.len > len){
        //can deduce length, that is automatically defined in plc
        ded_siz = plc.siz;
        len = plc.len;
    }
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
bool adv_string_view<T>::placeholder::is_len_defined() const noexcept{
    return len != 0 || siz == 0;
    //undefined when len == 0 and siz > 0
}

template<typename T>
bool adv_string_view<T>::verify_safe() const noexcept{
	size_t remlen = siz;
    size_t dlen = 0;
	const_tchar_pt<T> mem{ptr};
	while(remlen > 0){
		if(!mem.valid_next_update(remlen))
			return false;
        dlen++;
	}
	if(siz == ded_siz)
        return dlen == len;
    else{
        set_length(dlen);
        return true;
    }
}

template<typename T>
void adv_string_view<T>::verify() const{
	if(!verify_safe())
		throw incorrect_encoding("Invalid string encoding");
}

template<general_enctype T>
template<general_enctype S>
constexpr bool adv_string_view<T>::can_rebase(EncMetric_info<S> o) const noexcept{
    return can_rebase_pointer(ptr, o);
}

template<general_enctype T>
template<general_enctype S>
adv_string_view<S> adv_string_view<T>::rebase(EncMetric_info<S> o) const{
    return adv_string_view<S>{len, siz, rebase_pointer(ptr, o), ded_siz};
}

template<typename T>
adv_string_view<T>::placeholder adv_string_view<T>::select(size_t chr, bool exc) const{
    if(ptr.is_fixed()){
        if(chr > len){
            if(exc)
                throw out_of_range{"Past to end"};
            else
                return select_end();
        }
        return placeholder{ptr.data(), chr * ptr.min_bytes(), chr};
    }
    else if(chr < len){
        const_tchar_pt<T> mem = ptr;
        size_t rem = ded_siz;
        size_t ret = 0;
		for(size_t i=0; i< chr; i++)
			ret += mem.next_update(rem);
		return placeholder{ptr.data(), ret, chr};
    }
    else{
        size_t rchr = chr - len;
        while(rchr > 0){
            if(siz == ded_siz){
                if(exc)
                    throw out_of_range{"Past to end"};
                else
                    return select_end();
            }
            step_internal();
            rchr--;
        }
        return placeholder{ptr.data(), ded_siz, len};
    }
}

template<typename T>
adv_string_view<T>::placeholder adv_string_view<T>::select(const placeholder &base, size_t nchr, bool exc) const{
    validate(base);//implies base.siz <= ded_siz <= siz and base.len <= len
    const byte *dat = ptr.data();

    if(base.is_len_defined()){
        size_t totalchr = base.len + nchr;

        if(ptr.is_fixed()){
            if(totalchr > len){
                if(exc)
                    throw out_of_range{"Past to end"};
                else
                    return select_end();
            }
            return placeholder{ptr.data(), totalchr * ptr.min_bytes(), totalchr};
        }
        else if(totalchr < len){
            const_tchar_pt<T> mem = ptr + base.siz;
            size_t rem = ded_siz - base.siz;
            size_t ret = base.siz;
            for(size_t i=0; i< nchr; i++)
                ret += mem.next_update(rem);
            return placeholder{dat, ret, totalchr};
        }
        else{
            size_t rchr = totalchr - len;
            while(rchr > 0){
                if(siz == ded_siz){
                    if(exc)
                        throw out_of_range{"Past to end"};
                    else
                        return select_end();
                }
                step_internal();
                rchr--;
            }
            return placeholder{dat, ded_siz, len};
        }
    }
    else{
        size_t rem = siz - base.siz;
        size_t ret = base.siz;
        const_tchar_pt<T> mem = ptr + base.siz;
        for(size_t i=0; i < nchr; i++){
            if(ret == siz){
                if(exc)
                    throw out_of_range{"Past to end"};
                else
                    return select_end();
            }
            ret += mem.next_update(rem);
        }
        return placeholder{dat, ret, 0};
    }
    /*
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
	*/
}

template<typename T>
adv_string_view<T>::placeholder adv_string_view<T>::select_begin() const noexcept{
    return placeholder{ptr.data(), 0, 0};
}
template<typename T>
adv_string_view<T>::placeholder adv_string_view<T>::select_end() const noexcept{
    return placeholder{ptr.data(), siz, siz == ded_siz ? len : 0};
}

template<typename T>
void adv_string_view<T>::det_length() const{
    size_t rem = siz - ded_siz;
    auto p = ptr + ded_siz;
    while(rem > 0){
        ded_siz += p.next_update(rem);
        len++;
    }
}

template<typename T>
void adv_string_view<T>::det_length(placeholder &p) const{
    validate(p);
    if(p.is_len_defined())
        return;
    size_t rem = p.siz;
    auto mem = ptr;
    size_t tlen = 0;
    while(rem > 0){
        mem.next_update(rem);
        tlen ++;
    }
    p.len = tlen;
}

template<typename T>
size_t adv_string_view<T>::length() const{
    if(siz == ded_siz)
        return len;
    else{
        det_length();
        return len;
    }
}

template<typename T>
const_tchar_pt<T> adv_string_view<T>::at(placeholder plc) const{
    validate(plc);
    return ptr + plc.siz;
};

template<typename T>
size_t adv_string_view<T>::size(placeholder b, size_t n) const{
    placeholder e = select(b, n);
    return e.siz - b.siz;
}

template<typename T>
size_t adv_string_view<T>::rem_siz(placeholder b) const {
    validate(b);
    return siz - b.siz;
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
    if(b.is_len_defined() && e.is_len_defined())
        return adv_string_view<T>{e.len - b.len, e.siz - b.siz, at(b), e.siz - b.siz};
    else
        return adv_string_view<T>{0, e.siz - b.siz, at(b), 0};
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
bool adv_string_view<T>::operator==(const adv_string_view<S> &t) const{
    if(siz != t.size())
        return false;
    return std::memcmp(data(), t.data(), siz) == 0;
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
    if(raw_format().has_head()){
        size_t byt = 0;
        uint hd = raw_format().head();
        const_tchar_pt<T> newi = ptr;
        while(byt <= rem){
            if(compare(newi.data(), sq.begin().data(), sq.size())){
                return index_result{true, byt};
            }
            newi += hd;
            byt += hd;
        }
    }
    else{
        placeholder p = select_begin();
        while(p.siz <= rem){
            if(compare(p.data(), sq.begin().data(), sq.size())){
                return index_result{true, p.siz};
            }
            select_next(p);
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
	placeholder p = select_begin();
    /*
     * In this case we can't use the preceding optimization since we want to know
     * also the number of characters
     */
	while(p.siz <= rem){
		if(compare(p.data(), sq.begin().data(), sq.size())){
            return index_result{true, p.len};
		}
        select_next(p);
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
	placeholder p = select_begin();
    /*
     * In this case we can't use the preceding optimization since we want to know
     * also the number of characters
     */
	while(p.siz <= rem){
		if(compare(p.data(), sq.begin().data(), sq.size())){
            return p;
		}
		select_next(p);
	}
    return select_end();
}

template<typename T>
template<general_enctype S>
index_result adv_string_view<T>::containsChar(const adv_string_view<S> &cu) const{
    if(cu.empty())
        return index_result{true, 0};
    adv_string_view<S> strip = cu.substring(0, 1);
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
	if(siz < sq.size()){
		return false;
	}
	return compare(ptr.data(), sq.data(), sq.size());
}

template<typename T>
template<general_enctype S>
bool adv_string_view<T>::endsWith(const adv_string_view<S> &sq) const{
	if(!sq.can_rebase(raw_format()))
		throw incorrect_encoding{"Impossible to perform encode rebase"};
	if(sq.size() == 0){
		return true;
	}
	if(siz < sq.size()){
		return false;
	}
	if(raw_format().has_head()){
        const_tchar_pt<T> poi = ptr + (siz - sq.size());
        return compare(poi.data(), sq.begin().data(), sq.size());
    }
    else{
        det_length();// calculate effective len
        if(len < sq.length())
            return false;
        placeholder th = select(len - sq.length());
        if(sq.size() != (siz - th.siz))
            return false;
        return compare(th.data(), sq.data(), sq.size());
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
	if(siz < sq.size()){
		return conditional_result{false, select_begin()};
	}
	if(raw_format().has_head()){
        const_tchar_pt<T> poi = ptr + (siz - sq.size());
        if( compare(poi.data(), sq.begin().data(), sq.size()) )
            return conditional_result{true, placeholder{ptr.data(), siz - sq.size()}};//length not determined
        else
            return conditional_result{false, select_begin()};
    }
    else{
        det_length();// calculate effective len
        if(len < sq.length())
            return false;
        placeholder th = select(len - sq.length());
        if(sq.size() != (siz - th.siz))
            return false;
        if( compare(th.data(), sq.data(), sq.size()) )
            return conditional_result{true, th};
        else
            return conditional_result{false, select_begin()};
    }
}

template<typename T>
adv_string_view<T>::ctype adv_string_view<T>::get_char(placeholder pch) const{
    validate(pch);
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

