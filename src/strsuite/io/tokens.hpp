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
#include <strsuite/encmetric/enc_string.hpp>
#include <strsuite/io/enc_io_exc.hpp>

namespace sts{

/*
 * A simple class useful to divide a string into tokens
 */
template<general_enctype T>
class Token{
	private:
		adv_string_view<T> data;
        using placeholder = typename adv_string_view<T>::placeholder;
        placeholder b, e, end;

        void rec_find(const adv_string_view<T> &, bool &b, size_t &) const noexcept{
            b = false;
        }

        template<typename S, typename...Ty>
        void rec_find(const adv_string_view<T> &chr, bool &b, size_t &ret, const adv_string_view<S> & cu, const adv_string_view<Ty> &... next) const{
            if(cu.containsChar(chr)){
                b = true;
            }
            else{
                ret++;
                rec_find(chr, b, ret, next...);
            }
        }

	public:

        /*
         * Type returned by goUp and goUntil
         */

        struct path_finder{
            /*
             * True if e has not increased
             */
            bool eof_init;
            /*
             * True if originally e != end but now these are equal
             */
            bool eof_ter;
            /*
             * Index of the first string in which delimited character was found
             * set to 0 by goUntil
             */
            size_t idx;

            bool eof() const noexcept{ return eof_init || eof_ter; }
            bool found() const noexcept{ return !eof_init && !eof_ter; }

            operator bool() const noexcept{ return !eof_init;}
        };

        template<general_enctype S>
		Token(const adv_string_view<S> &main, EncMetric_info<T> info) : data{main.rebase(info)}, b{data.select_begin()}, e{data.select_begin()}, end{data.select_end()} {
            data.verify();
        }

		template<general_enctype S>
		Token(const adv_string_view<S> &main) requires strong_enctype<T> : Token{main, EncMetric_info<T>{}} {}

		template<general_enctype S>
		Token(const adv_string_view<S> &main, const EncMetric<typename T::ctype> *format) requires widenc<T> : Token{main, EncMetric_info<T>{format}} {}

		bool end_reached() const noexcept {return e == end;}
		void flush() noexcept {b = e;}
		/*
         * Step token parser by one character
         */
		bool step() {
			return data.select_next_eof(e);
		}
		/*
         * Share a view of current token
         */
		adv_string_view<T> share() const {return data.substring(b, e);}
		adv_string<T> allocate(std::pmr::memory_resource *alloc = std::pmr::get_default_resource()) const{ return adv_string<T>{data.substring(b, e), alloc}; }


		/*
         * Steps the token pointer until it encounter a character contained in the argumet
         */
        template<typename... Ty>
		path_finder goUp(const adv_string_view<Ty> &... delim){
			if(e == end)
				return path_finder{true, false, 0};
            while(true){
                bool find = false;
                size_t idx = 0;
                auto chr = data.substring(e, end);
                rec_find(chr, find, idx, delim...);
                if(find)
                    return path_finder{false, false, idx};
                if(data.select_next_eof(e))
                    break;
            }
            return path_finder{false, true, 0};
		}
		/*
         * Steps the token pointer until it encounter a character NOT contained in the argumet
         */
        template<typename... Ty>
		path_finder goUntil(const adv_string_view<Ty> &... delim){
			if(e == end)
				return path_finder{true, false, 0};
            while(true){
                bool find = false;
                size_t idx = 0;
                auto chr = data.substring(e, end);
                rec_find(chr, find, idx, delim...);
                if(!find)
                    return path_finder{false, false, 0};
                if(data.select_next_eof(e))
                    break;
            }
            return path_finder{false, true, 0};
		}
		/*
         * Get a view of token delimited by any delimiter character passed in the argument
         */
        template<general_enctype S>
		adv_string_view<T> proceed(const adv_string_view<S> &delim){
            goUntil(delim);
			flush();
			goUp(delim);;
			return share();
		}

		/*
         * Threat Token as a read_only character stream with respect the e placeholder
         */

        template<general_enctype S>
        uint char_read(tchar_pt<S> ptr, size_t l){
            assert_inv_rebase_pointer(ptr, data.raw_format());
            if(e == end)
                throw IOEOF{};
            uint gs = static_cast<uint>(data.size(e, 1));
            if(l < gs)
                throw IOBufsmall{gs - static_cast<uint>(l)};
            copy_bytes(ptr.data(), e.data(), gs);
            return step();
        }

        template<general_enctype S>
        uint char_read_v(tchar_pt<S> ptr, size_t l){
            return char_read(ptr, l);
        }

        template<general_enctype S>
        uint ghost_read(tchar_pt<S> ptr, size_t l) const{
            assert_inv_rebase_pointer(ptr, data.raw_format());
            if(e == end)
                throw IOEOF{};
            uint gs = static_cast<uint>(data.size(e, 1));
            if(l < gs)
                throw IOBufsmall{gs - static_cast<uint>(l)};
            copy_bytes(ptr.data(), e.data(), gs);
            return gs;
        }

        template<general_enctype S>
        uint ghost_read_v(tchar_pt<S> ptr, size_t l) const{
            return ghost_read(ptr, l);
        }
};

}
