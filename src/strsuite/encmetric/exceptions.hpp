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

#include <strsuite/encmetric/base.hpp>
#include <exception>

namespace sts{

class encoding_error : public std::exception{
	private:
		const char *c;
	public:
		encoding_error(const char *ms) : c{ms} {}
		encoding_error() : encoding_error{"EncMetric error"} {}
		const char *what() const noexcept override {return c;}
};

class incorrect_encoding : public encoding_error{
    public:
        incorrect_encoding() : encoding_error{"The byte sequence doesn't represent any encoded character"} {}
        incorrect_encoding(const char *m) : encoding_error{m} {}
};

class buffer_small : public encoding_error{
	private:
		/*
			Minimum size required to encode next character, 0 if not determined
		*/
		uint mins;
	public:
		buffer_small(const char *ms) : encoding_error{ms}, mins{0} {}
		buffer_small() : encoding_error{"Buffer is too small"}, mins{0} {}
		buffer_small(uint ms) : encoding_error{"Buffer is too small"}, mins{ms} {}
		uint get_required_size() const noexcept{
			return mins;
		}
};

/*
 * Thrown when a strings doesn't fit entirely the assigned buffer
 */
class buffer_big : public encoding_error{
    public:
        buffer_big() : encoding_error{"Buffer is too big"} {}
        buffer_big(const char *m) : encoding_error{m} {}
};

}


