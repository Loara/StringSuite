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
    public:
        using ctype = typename T::ctype;

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

        void rec_find_ctype(const ctype &, bool &b, size_t &) const noexcept{
            b = false;
        }

        template<typename...Ty>
        void rec_find_ctype(const ctype &chr, bool &b, size_t &ret, const ctype & cu, const Ty &... next) const{
            if(chr == cu){
                b = true;
            }
            else{
                ret++;
                rec_find_ctype(chr, b, ret, next...);
            }
        }

        template<typename Container, typename...Ty> requires requires(const Container a, const ctype c) {{a.contains(c)}->std::same_as<bool>;}
        void rec_find_ctype(const ctype &chr, bool &b, size_t &ret, const Container & cont, const Ty &... next) const{
            if(cont.contains(chr)){
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

            bool eof() const noexcept{ return eof_init || eof_ter; }
            bool found() const noexcept{ return !eof_init && !eof_ter; }

            operator bool() const noexcept{ return !eof_init;}
        };

        struct path_finder_idx : public path_finder{
            /*
             * Index of the first string in which delimited character was found
             * set to 0 by goUntil
             */
            size_t idx;

            path_finder_idx(bool i, bool t, size_t d) : path_finder{i, t}, idx{d} {}
        };

		Token(const adv_string_view<T> &main) : data{main}, b{data.select_begin()}, e{data.select_begin()}, end{data.select_end()} {
            data.verify();
        }

		bool end_reached() const noexcept {return e == end;}
		bool ended() const noexcept {return b == end;}
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

		Token<T> subToken() const{
            return Token<T>{share()};
        }
		/*
         * Steps the token pointer until it encounter a character contained in the argumets
         */
        template<typename... Ty>
		path_finder_idx goUp(const adv_string_view<Ty> &... delim){
			if(e == end)
				return path_finder_idx{true, false, 0};
            while(true){
                bool find = false;
                size_t idx = 0;
                auto chr = data.substring(e, end);
                rec_find(chr, find, idx, delim...);
                if(find)
                    return path_finder_idx{false, false, idx};
                if(data.select_next_eof(e))
                    break;
            }
            return path_finder_idx{false, true, 0};
		}

		/*
         * Steps the token pointer until it encounter a character listed in the argumets
         *
         * Arguments are now characters/decoded objects
         */
        template<typename... Ty>
		path_finder_idx goUp_ctype(const Ty &... cont){
			if(e == end)
				return path_finder_idx{true, false, 0};
            while(e != end){
                const ctype chr = data.get_char(e);
                bool find = false;
                size_t idx = 0;
                rec_find_ctype(chr, find, idx, cont...);
                if(find)
                    return path_finder_idx{false, false, idx};
                data.select_next(e);
            }
            return path_finder_idx{false, true, 0};
		}

		/*
         * Steps the token pointer until it encounter a character NOT contained in the argumet
         */
        template<typename Ty>
		path_finder goUntil(const adv_string_view<Ty> & delim){
			if(e == end)
				return path_finder{true, false};
            while(true){
                auto chr = data.substring(e, end);
                bool find = delim.containsChar(chr);
                if(!find)
                    return path_finder{false, false};
                if(data.select_next_eof(e))
                    break;
            }
            return path_finder{false, true};
		}
        template<typename Container>
		path_finder goUntil_container(const Container & delim){
			if(e == end)
				return path_finder{true, false};
            while(e != end){
                const ctype chr = data.get_char(e);
                bool find = delim.contains(chr);
                if(!find)
                    return path_finder{false, false};
                data.select_next(e);
            }
            return path_finder{false, true};
		}

		/*
         * Threat Token as a read_only character stream with respect the e placeholder
         */
        using enc = T;

        EncMetric_info<T> raw_format() const noexcept{
            return data.raw_format();
        }

        template<general_enctype S>
        uint char_read(tchar_pt<S> ptr, size_t l){
            assert_inv_rebase_pointer(ptr, data.raw_format());
            if(e == end)
                throw IOEOF{};
            uint gs = static_cast<uint>(data.size(e, 1));
            //Per sicurezza
            if(gs == 0)
                throw IOEOF{};

            if(l < gs)
                throw IOBufsmall{gs - static_cast<uint>(l)};
            copy_bytes(ptr.data(), e.data(), gs);
            step();
            return gs;
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
            if(gs == 0)
                throw IOEOF{};

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
