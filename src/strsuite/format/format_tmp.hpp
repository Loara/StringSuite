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
/*
 * Temporary header in order to implement formatting strings
 * Try to do not use it
 */

#include <strsuite/io/string_stream.hpp>
#include <strsuite/io/tokens.hpp>
#include <strsuite/encmetric/enc_string.hpp>
#include <strsuite/format/integral_format.hpp>

#include <cstdio>
#include <set>

namespace sts{
    template<typename OStream, typename... Args>
    void tmp_format(OStream &out, const char *str, Args &&... a){
        char buffer[50];
        int ret = std::snprintf(buffer, 50, str, std::forward<Args>(a)...);
        if(ret <= 0)
            return;
        adv_string_view<ASCII> cvr{buffer, (uint)ret};
        cvr.verify();
        out.string_write(cvr);
    }

    template<general_enctype T, typename Formatter>
    class adv_formatter{
        static_assert(std::same_as<typename T::ctype, unicode>, "Not a string encoding");
    private:
        EncMetric_info<T> info;
        Formatter f;
        std::pmr::memory_resource *res;

        template<typename Arg, typename... Args>
        void dispatch_call(string_stream<T> &, size_t, const adv_string_view<T> &, Arg &&, Args &&...);
        void dispatch_call(string_stream<T> &, size_t, const adv_string_view<T> &);

        template<typename placeholder>
        size_t get_id(placeholder &, const adv_string_view<T> &);
    public:
        template<typename U>
        adv_formatter(U &&, EncMetric_info<T>, std::pmr::memory_resource * = std::pmr::get_default_resource());

        template<typename U>
        adv_formatter(U && ff, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()) requires strong_enctype<T> : adv_formatter{std::forward<U>(ff), EncMetric_info<T>{}, alloc} {}

        template<typename U>
        adv_formatter(U && ff, const EncMetric<typename T::ctype> *f, std::pmr::memory_resource *alloc = std::pmr::get_default_resource()) requires widenc<T> : adv_formatter{std::forward<U>(ff), EncMetric_info<T>{f}, alloc} {}

        template<general_enctype S, typename... Args>
        adv_string<T> format(const adv_string_view<S> &, Args &&...);
    };

#include <strsuite/format/format_tmp.tpp>
}
