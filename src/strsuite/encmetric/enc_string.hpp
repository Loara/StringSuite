#pragma once

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
#include <compare>
#include <memory_resource>
#include <strsuite/encmetric/config.hpp>
#include <strsuite/encmetric/chite.hpp>
#include <strsuite/encmetric/basic_ptr.hpp>

namespace sts{

template<typename FuncType, typename T>
concept is_terminate_func = requires(const FuncType &ft, const tread_pt<T> ptr, const size_t siz){
    {ft(ptr, siz)}->std::same_as<bool>;
};

/*
 * Basic terminate function: string is terminated if and only if the encoded character is all 0 bytes
 */
template<general_enctype T>
bool zero_terminating(tread_pt<T> ptr, size_t){
    uint size = ptr.min_bytes();
    auto data = ptr.data();
	for(uint i=0; i<size; i++){
		if(data[i] != byte{0})
			return false;
	}
	return true;
}
/*
 * Another terminate function: terminate if and only if the encoded character is equivalent to 0
 *
 * WARNING: control up to st characters and may throw if the string is not correctly encoded
 */
template<general_enctype T>
bool encoding_terminating(tread_pt<T> ptr, size_t st){
    using ctype = typename T::ctype;
    ctype cha;
    try{
        std::tie(std::ignore, cha) = ptr.decode(st);
    }
    catch(const buffer_small &){
        return false;
    }
    return cha == ctype{0};
}

/*
 * length must be determined at definition time in order to avoid synchronization issues in multithreaded environments.
 *
 * If you don't want to evaluate length you should instead use the Token class in io submodule
 */
template<general_enctype T>
class adv_string_view{
	private:
		tread_pt<T> ptr;
		size_t siz;//bytes number
		size_t len;//character number
	protected:
		explicit adv_string_view(size_t length, size_t size, tread_pt<T> bin) noexcept : ptr{bin}, siz{size}, len{length} {}
	public:
        using ctype = typename T::ctype;
        using light_ctype = typename EncMetric_info<T>::light_ctype;

        /*
         * Class placeholder
         */
        class placeholder{
        private:
            const byte *start;
            size_t siz, len;
            placeholder(const byte *p, size_t s, size_t l) : start{p}, siz{s}, len{l} {}
            placeholder(const byte *p, size_t s) : start{p}, siz{s}, len{0} {}

        public:
            placeholder(const placeholder &) =default;
            ~placeholder() {}

            placeholder &operator=(const placeholder &) =default;

            const byte *data() const noexcept;
            size_t nbytes() const noexcept;
            size_t nchars() const noexcept;

            bool comparable(const placeholder &p) const noexcept{
                return start == p.start;
            }

            std::partial_ordering operator<=>(const placeholder &p) const noexcept{
                if(!comparable(p))
                    return std::partial_ordering::unordered;
                else
                    return siz <=> p.siz;
            }

            bool operator==(const placeholder &p) const noexcept { return (*this <=> p) == 0;}
            friend class adv_string_view<T>;
        };
        /*
         *
         */

        struct ded_type{
            size_t siz;
            size_t len;
        };

        explicit adv_string_view(tread_pt<T>, size_t siz);
        template<typename FuncType>
		explicit adv_string_view(tread_pt<T>, size_t maxsiz, const FuncType &);
		explicit adv_string_view(tread_pt<T>, size_t maxsiz, size_t maxlen);

        explicit adv_string_view(EncMetric_info<T> f) : adv_string_view{0, 0, tread_pt{nullptr, f}} {}
        explicit adv_string_view(const byte *b, EncMetric_info<T> f, size_t maxsiz) : adv_string_view{tread_pt<T>{b, f}, maxsiz} {}
        template<typename FuncType>
		explicit adv_string_view(const byte *b, EncMetric_info<T> f, size_t maxsiz, const FuncType &tf) : adv_string_view{tread_pt<T>{b, f}, maxsiz, tf} {}

        explicit adv_string_view() requires strong_enctype<T> : adv_string_view{EncMetric_info<T>{}} {}
		template<typename U>
        explicit adv_string_view(const U *b, size_t maxsiz) requires strong_enctype<T> : adv_string_view{tread_pt<T>{b}, maxsiz} {}
		template<typename U, typename FuncType>
		explicit adv_string_view(const U *b, size_t maxsiz, const FuncType &tf) requires strong_enctype<T> : adv_string_view{tread_pt<T>{b}, maxsiz, tf} {}

        explicit adv_string_view(const EncMetric<typename T::ctype> *f) requires widenc<T> : adv_string_view{EncMetric_info<T>{f}} {}
		template<typename U>
        explicit adv_string_view(const U *b, size_t maxsiz, const EncMetric<typename T::ctype> *f) requires widenc<T> : adv_string_view{tread_pt<T>{b, f}, maxsiz} {}
		template<typename U, typename FuncType>
		explicit adv_string_view(const U *b, size_t maxsiz, const EncMetric<typename T::ctype> *f, const FuncType &tf) requires widenc<T> : adv_string_view{tread_pt<T>{b, f}, maxsiz, tf} {}

		virtual ~adv_string_view() {}
		/*
		    Verify the string is correctly encoded
		*/
		void verify() const;
		bool verify_safe() const noexcept;

		EncMetric_info<T> raw_format() const noexcept{ return ptr.raw_format();}
		const EncMetric<ctype> *format() const noexcept{ return ptr.format();}

        /*
         * Returns the same string but with a compatible new encoding
         * For example if T=ASCII then S can be UTF8, Latin1, Windows codepages, ....
         */
        template<general_enctype S>
        constexpr bool can_rebase(EncMetric_info<S>) const noexcept;

        template<general_enctype S>
        adv_string_view<S> rebase(EncMetric_info<S>) const;

        template<strong_enctype S>
        adv_string_view<S> rebase() const {return rebase(EncMetric_info<S>{});}

        adv_string_view<WIDE<ctype>> rebase(const EncMetric<ctype> *denc) const {return rebase(EncMetric_info<WIDE<ctype>>{denc});}

        template<general_enctype S>
        adv_string_view<S> rebase_as(const adv_string_view<S> &as) const{ return rebase(as.raw_format());}

        void validate(const placeholder &) const;
        placeholder select(const placeholder &base, size_t nchr, bool exeption =false) const;
        placeholder select_begin() const noexcept;
        placeholder select_end() const noexcept;

        placeholder select(size_t nchr, bool exc =false) const{
            return select(select_begin(), nchr, exc);
        }

        void select_next(placeholder &p) const{
            p = select(p, 1, true);
        }
        void select_next(placeholder &p, size_t n) const{
            p = select(p, n, true);
        }
        bool select_next_eof(placeholder &p) const{
            if(p == select_end())
                return true;
            select_next(p);
            return false;
        }
		
		adv_string_view<T> substring(placeholder b, placeholder e) const;
		adv_string_view<T> substring(placeholder b, size_t e) const{ return substring(b, select(e));}
		adv_string_view<T> substring(size_t b, placeholder e) const{ return substring(select(b), e);}
		adv_string_view<T> substring(size_t b, size_t e) const{
            placeholder bb = select(b);
            placeholder ee = select(bb, e >= b ? e - b : 0);
            return substring(bb, ee);
        }
		adv_string_view<T> substring(placeholder b) const;
		adv_string_view<T> substring(size_t b) const;

        bool empty() const noexcept{ return siz == 0;}
		size_t length() const noexcept{ return len; }
		size_t size() const noexcept {return siz;}
		size_t size(placeholder, size_t n) const;
		size_t size(size_t a, size_t n) const {return size(select(a), n); }
		size_t size(size_t n) const {return size(select_begin(), n);}
		size_t rem_siz(placeholder) const;
        size_t rem_siz(size_t a) const {return rem_siz(select(a)); }

        /*
         * compare the encoded bytes of first n characters
         * and returns true if they've same size and same bytes
         */
		template<general_enctype S>
		bool equal_bytes(const adv_string_view<S> &, size_t n) const;

		/*
         * A byte-only lexicographics ordering, doesn't consider encoding nor effective characters
         *
         * Use it mainly for storing purposes
         */
		template<general_enctype S>
		std::weak_ordering operator<=>(const adv_string_view<S> &) const;

		template<general_enctype S>
		bool operator==(const adv_string_view<S> &t) const;

		template<general_enctype S>
		index_result bytesOf(const adv_string_view<S> &) const;

		template<general_enctype S>
		index_result indexOf(const adv_string_view<S> &) const;

        template<general_enctype S>
        placeholder placeOf(const adv_string_view<S> &) const;

		template<general_enctype S>
		index_result containsChar(const adv_string_view<S> &) const;

		template<general_enctype S>
		bool startsWith(const adv_string_view<S> &) const;

		template<general_enctype S>
		bool endsWith(const adv_string_view<S> &) const;

		template<general_enctype S>
		conditional_result<placeholder> endsWith_placeholder(const adv_string_view<S> &) const;

		const byte *data() const noexcept {return ptr.data();}
		/*
			Mustn't throw any exception if 0 <= a <= len
		*/
		const_tchar_pt<T> at(placeholder) const;
		const_tchar_pt<T> at(size_t chr) const {return at(select(chr));}
		const_tchar_pt<T> begin() const noexcept {return at(select_begin());}
		const_tchar_pt<T> end() const noexcept {return at(select_end());}

		ctype get_char(placeholder) const;
        ctype get_char(size_t chr) const {return get_char(select(chr));}
        ctype get_first_char() const {return get_char(select_begin());}

		light_ctype get_char_light(placeholder) const;
        light_ctype get_char_light(size_t chr) const {return get_char_light(select(chr));}
        light_ctype get_first_char_light() const {return get_char_light(select_begin());}

        template<typename Container>
        void get_all_char(Container &) const;

        friend adv_string_view<T> direct_build<T>(const_tchar_pt<T> , size_t , size_t ) noexcept;

        template<general_enctype S>
        friend class adv_string_view;
};

/*
 * Doesn't perform any control about dimensions
 *
 * Use with care
 */
template<general_enctype T>
adv_string_view<T> direct_build(const_tchar_pt<T> ptr, size_t len, size_t siz) noexcept{
    return adv_string_view<T>{len, siz, ptr};
}


using adv_wstr_view = adv_string_view<WIDE<unicode>>;

#include <strsuite/encmetric/enc_string.tpp>
}


