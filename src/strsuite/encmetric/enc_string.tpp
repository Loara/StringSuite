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
adv_string_view<T>::adv_string_view(tread_pt<T> cu, size_t size) : ptr{cu}, siz{0}, len{0}{
    if(ptr.is_fixed()){
        uint csiz = ptr.min_bytes();
        if(size % csiz != 0)
            throw incorrect_encoding{"Last character is truncated"};
        siz = size;
        len = size / csiz;
    }
    else{
        size_t rem = size;
        size_t ded_siz=0, ded_len=0;
        auto look = cu;
        while(rem > 0){
            try{
                ded_siz += look.next_update(rem);
                ded_len ++;
            }
            catch(buffer_small &){
                throw incorrect_encoding{"Last character is truncated"};
            }
        }
        siz = ded_siz;
        len = ded_len;
    }
}

template<typename T>
template<typename FuncType>
adv_string_view<T>::adv_string_view(tread_pt<T> cu, size_t maxsiz, const FuncType &terminate) : ptr{cu}, siz{0}, len{0} {
    static_assert(is_terminate_func<FuncType, T>, "Not a terminate function");
    auto dec = cu;
    size_t ded_siz=0, ded_len=0;
    size_t rem = maxsiz;
    while(rem > 0){
        if(terminate(dec, rem))
            break;
        try{
            ded_siz += dec.next_update(rem);
            ded_len++;
        }
        catch(buffer_small &){
            break;
        }
    }
    siz = ded_siz;
    len = ded_len;
}

template<typename T>
void adv_string_view<T>::validate(const placeholder &plc) const{
    if(plc.start != ptr.data() || plc.siz > siz || plc.len > len)
        throw invalid_placeholder{};
}

/*
 * Placeholders
 */

template<typename T>
const byte * adv_string_view<T>::placeholder::data() const noexcept{
    return start + siz;
}
template<typename T>
size_t adv_string_view<T>::placeholder::nbytes() const noexcept{
    return siz;
}
template<typename T>
size_t adv_string_view<T>::placeholder::nchars() const noexcept{
    return len;
}

/*
 * Strings
 */

template<typename T>
bool adv_string_view<T>::verify_safe() const noexcept{
	size_t remlen = siz;
    size_t dlen = 0;
	tread_pt<T> mem = ptr;
	while(remlen > 0){
		if(!mem.valid_next_update(remlen))
			return false;
        dlen++;
	}
	return dlen == len;
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
    return adv_string_view<S>{len, siz, rebase_pointer(ptr, o)};
}

template<typename T>
adv_string_view<T>::placeholder adv_string_view<T>::select(const placeholder &base, size_t nchr, bool exc) const{
    validate(base);//implies base.siz <= siz and base.len <= len
    const byte *dat = ptr.data();
    size_t totalchr = base.len + nchr;
    if(totalchr > len){
        if(exc)
            throw out_of_range{"Past to end"};
        else
            return select_end();
    }

    if(ptr.is_fixed()){
        return placeholder{dat, totalchr * ptr.min_bytes(), totalchr};
    }
    else{
        tread_pt<T> mem = ptr + base.siz;
        size_t rem = siz - base.siz;
        size_t ret = base.siz;
        for(size_t i=0; i< nchr; i++)
            ret += mem.next_update(rem);
        return placeholder{dat, ret, totalchr};
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
    if(b > e)
        b = e;
    return adv_string_view<T>{e.len - b.len, e.siz - b.siz, at(b)};
}

template<typename T>
template<general_enctype S>
bool adv_string_view<T>::equal_bytes(const adv_string_view<S> &t, size_t ch) const{
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
    if(!ptr.raw_format().equalTo(t.raw_format()))
        return false;
    if(siz != t.size())
        return false;
    return compare(data(), t.data(), siz);
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
    tread_pt<T> newi = ptr;
    const char *u = sq.data();
    size_t s = sq.size();
    size_t byt = 0;

    if(raw_format().has_head()){
        uint hd = raw_format().head();
        while(byt <= rem){
            if(compare(newi.data(), u, s)){
                return index_result{true, byt};
            }
            newi += hd;
            byt += hd;
        }
    }
    else{
        size_t r = siz;
        while(byt <= rem){
            if(compare(newi.data(), u, s)){
                return index_result{true, byt};
            }
            byt += newi.next_update(r);
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
    const byte *u = sq.data();
    size_t s = sq.size();
    /*
     * In this case we can't use the preceding optimization since we want to know
     * also the number of characters
     */
	while(p.siz <= rem){
		if(compare(p.data(), u, s)){
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
    const byte *u = sq.data();
    size_t s = sq.size();
    /*
     * In this case we can't use the preceding optimization since we want to know
     * also the number of characters
     */
	while(p.siz <= rem){
		if(compare(p.data(), u, s)){
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
        tread_pt<T> poi = ptr + (siz - sq.size());
        return compare(poi.data(), sq.begin().data(), sq.size());
    }
    else{
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
	if(siz < sq.size() || len < sq.length()){
		return conditional_result{false, select_begin()};
	}

	placeholder th = select(len - sq.length());
    if(sq.size() != (siz - th.siz))
        return conditional_result{false, select_begin()};
    if(compare(th.data(), sq.data(), sq.size()))
        return conditional_result{true, th};
    else
        return conditional_result{false, select_begin()};
}

template<typename T>
adv_string_view<T>::ctype adv_string_view<T>::get_char(placeholder pch) const{
    validate(pch);
    if(pch == select_end())
        throw out_of_range{"Argument doesn't point to any valid character"};
    return get_chr_el(ptr.new_instance(pch.data()).decode(siz - pch.siz));
}

template<typename T>
adv_string_view<T>::light_ctype adv_string_view<T>::get_char_light(placeholder pch) const{
    validate(pch);
    if(pch == select_end())
        throw out_of_range{"Placeholder to end"};
    return ptr.raw_format().light_decode_direct(pch.data(), siz - pch.siz);
}

template<typename T>
template<typename Container>
void adv_string_view<T>::get_all_char(Container &cont) const{
    tread_pt<T> mem = ptr;
    size_t rem = siz;
    for(size_t i=0; i< len; i++){
        auto ret = mem.decode_next_update(rem);
        cont.push_back(get_chr_el(ret));
    }
}

