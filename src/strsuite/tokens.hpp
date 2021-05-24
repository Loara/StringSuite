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

namespace sts{

/*
 * A simple class useful to divide a string into tokens
 */
template<general_enctype T>
class Token{
	private:
		const_tchar_pt<T> s, e, end;
	public:
		Token(adv_string_view<T> main) : s{main.begin()}, e{main.begin()}, end{main.end()} {}
		/*
         * True if there isn't any token to parse
         */
		bool eof() const noexcept {return s == end;}
		/*
         * Flushes token pointers
         */
		void flush() noexcept {s = e;}
		/*
         * Step token parser by one character
         */
		bool step() {
			if(e == end)
				return false;
			e.next();
			return true;
		}
		/*
         * Share a view of current token
         */
		adv_string_view<T> share() const {return adv_string_view<T>(s, e-s, e-s);}
		/*
         * Steps the token pointer until it encounter a character contained in the argumet
         */
		bool goUp(const adv_string_view<T> &delim){
			if(e == end)
				return false;
			while(!delim.containsChar(e)){
				e.next();
				if(e == end)
					return false;
			}
			return true;
		}
		/*
         * Steps the token pointer until it encounter a character NOT contained in the argumet
         */
		bool goUntil(const adv_string_view<T> &delim){
			if(e == end)
				return false;
			while(delim.containsChar(e)){
				e.next();
				if(e == end)
					return false;
			}
			return true;
		}
		/*
         * Get a view of token delimited by any delimiter character passed in the argument
         */
		adv_string_view<T> proceed(const adv_string_view<T> &delim){
            goUntil(delim);
			flush();
			goUp(delim);
			adv_string_view<T> ret = share();
			return ret;
		}
};

}
