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
#pragma once

namespace sts{
    template<typename... Tlist>
    struct type_array;

    template<>
    struct type_array<>{
        static constexpr std::size_t size=0;
        static constexpr bool empty=true;
    };

    template<typename T, typename... Tlist>
    struct type_array<T, Tlist...>{
        static constexpr std::size_t size=sizeof...(Tlist)+1;
        static constexpr bool empty=false;
        using first = T;
        using next = type_array<Tlist...>;
    };

    template<typename T>
    struct is_tar_t : std::false_type{};

    template<typename... Tlist>
    struct is_tar_t<type_array<Tlist...>> : std::true_type{};

    template<typename T>
    concept is_type_array = is_tar_t<T>::value;

    template<typename T>
    concept type_array_not_empty = is_type_array<T> && (!T::empty);
    template<typename T>
    concept type_array_empty = is_type_array<T> && (T::empty);

    /*
     * Get element at some position
     */

    template<size_t N, typename T>
    struct access_ar;

    template<size_t N, typename T> requires type_array_empty<T>
    struct access_ar<N, T>{
        static_assert(!T::empty, "Out of bounds type array");
    };

    template<size_t N, typename T> requires type_array_not_empty<T> && (N == 0)
    struct access_ar<N, T>{
        using val = T::first;
    };

    template<size_t N, typename T> requires type_array_not_empty<T> && (N > 0)
    struct access_ar<N, T> : public access_ar <N-1, typename T::next>{
    };

    template<size_t N, is_type_array T>
    using get_type_at = access_ar<N, T>::val;

    template<typename T, is_type_array TL>
    constexpr bool tar_f_h()noexcept{
        if constexpr(TL::empty)
            return false;
        else if constexpr(std::same_as<T, typename TL::first>)
            return true;
        else
            return tar_f_h<T, typename TL::next>();
    }

    template<typename T, typename TL>
    concept is_in_type_array = is_type_array<TL> && tar_f_h<T, TL>();

}
