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

#include <strsuite/encmetric/endianess.hpp>

namespace sts{
    namespace q_param{
        template<typename>
        struct grab_member : public std::false_type{};

        template<typename T, typename ty>
        struct grab_member<ty T::*> : public std::true_type{
            using type = T;
            using member_type = ty;
        };

        template<typename U>
        concept is_member = grab_member<U>::value;

        template<auto a>
        using grab_member_T = typename grab_member<decltype(a)>::type;
        template<auto a>
        using grab_member_ty = typename grab_member<decltype(a)>::member_type;



        template<auto member, typename Enc = Endian_enc<false, grab_member_ty<member> > >
        struct enc_member{
            static_assert(is_member<decltype(member)>, "Must pass a valid pointer to member");

            using T = grab_member_T<member>;
            using ty = grab_member_ty<member>;

            static_assert(strong_enctype_of<Enc, ty>, "Must be a valid class encoding");

            static consteval uint g_min() noexcept{
                return Enc::min_bytes();
            }
            static uint g_clen(const byte *b, size_t s){
                return Enc::chLen(b, s);
            }
            static validation_result g_valid(const byte *b, size_t s) noexcept{
                return Enc::validChar(b, s);
            }
            static uint g_get(T *oj, const byte *b, size_t s){
                tuple_ret<ty> data = Enc::decode(b, s);
                oj->*member = get_chr_el(data);
                return get_len_el(data);
            }
            static uint g_set(const T *oj, byte *b, size_t s){
                return Enc::encode(oj->*member, b, s);
            }
        };

        template<auto member, auto func>
        struct noenc_member{
            static_assert(is_member<decltype(member)>, "Must pass a valid pointer to member");
            using T = grab_member_T<member>;

            static consteval uint g_min() noexcept{
                return 0;
            }
            static uint g_clen(const byte *, size_t){
                return 0;
            }
            static validation_result g_valid(const byte *, size_t ) noexcept{
                return validation_result{true, 0};
            }
            static uint g_get(T *oj, const byte *, size_t ){
                oj->*member = func();
                return 0;
            }
            static uint g_set(const T *, byte *, size_t){
                return 0;
            }
        };

        template<size_t num, size_t N =4>
        struct id_code{
            static consteval uint g_min() noexcept{
                return N;
            }
            static uint g_clen(const byte *, size_t){
                return N;
            }
            static validation_result g_valid(const byte *b, size_t l) noexcept{
                if(l < N)
                    return validation_result{false, 0};
                size_t val = Endian_enc<false, size_t, N>::decode_direct(b, l);
                return validation_result{val == num, N};
            }
            static uint g_get(const byte *b, size_t l){
                size_t val = Endian_enc<false, size_t, N>::decode_direct(b, l);
                if(val != num)
                    throw sts::incorrect_encoding{"Invalid control code inside encoded object"};
                return N;
            }
            static uint g_set(byte *b, size_t l){
                return Endian_enc<false, size_t, N>::encode(num, b, l);
            }
        };

        template<typename>
        inline constexpr bool is_id_code = false;

        template<size_t a, size_t b>
        inline constexpr bool is_id_code<id_code<a, b>> = true;

        template<typename Par, typename T>
        concept quick_enc_parameter = is_id_code<Par> || std::same_as<typename Par::T, T>;

        template<typename, typename...>
        struct quick_enc_0;

        template<typename T>
        struct quick_enc_0<T>{
            static consteval uint g_min() noexcept{
                return 0;
            }
            static uint g_clen(const byte *, size_t){
                return 0;
            }
            static validation_result g_valid(const byte *, size_t ) noexcept{
                return validation_result{true, 0};
            }
            static uint g_get(T *, const byte *, size_t ){
                return 0;
            }
            static uint g_set(const T *, byte *, size_t){
                return 0;
            }
        };

        template<typename T, typename Par, typename... Pars>
        struct quick_enc_0<T, Par, Pars...>{
            static_assert(quick_enc_parameter<Par, T>, "Pointer to member belonging to a different class");

            using next = quick_enc_0<T, Pars...>;

            static consteval uint g_min() noexcept{
                return Par::g_min() + next::g_min();
            }
            static uint g_clen(const byte *b, size_t s){
                uint get_siz = Par::g_clen(b, s);
                if(s < get_siz)
                    throw buffer_small{get_siz - static_cast<uint>(s)};
                return get_siz + next::g_clen(b + get_siz, s - get_siz);
            }
            static validation_result g_valid(const byte *b, size_t s) noexcept{
                validation_result valid = Par::g_valid(b, s);
                if(!valid)
                    return validation_result{false, 0};
                else if (valid.get() > s)
                    return validation_result{false, 0};

                validation_result n = next::g_valid(b + valid.get(), s - valid.get());
                if(!n)
                    return validation_result{false, 0};
                else
                    return validation_result{true, valid.get() + n.get()};
            }
            static uint g_get(T *oj, const byte *b, size_t s){
                uint inc = 0;
                if constexpr(is_id_code<Par>)
                    inc = Par::g_get(b, s);
                else
                    inc = Par::g_get(oj, b, s);
                return inc + next::g_get(oj, b+inc, s-inc);
            }
            static uint g_set(const T *oj, byte *b, size_t s){
                uint inc = 0;
                if constexpr(is_id_code<Par>)
                    inc = Par::g_set(b, s);
                else
                    inc = Par::g_set(oj, b, s);
                return inc + next::g_set(oj, b+inc, s-inc);
            }
        };

    }

    template<typename T, typename... Pars>
    struct quick_serialize{
    private:
        using my = q_param::quick_enc_0<T, Pars...>;
    public:
        using ctype = T;
        static consteval uint min_bytes() noexcept{
            return my::g_min();
        }
        static uint chLen(const byte *b, size_t s){
            return my::g_clen(b, s);
        }
        static validation_result validChar(const byte *b, size_t s) noexcept{
            return my::g_valid(b, s);
        }
        static tuple_ret<T> decode(const byte *b, size_t s) requires std::is_default_constructible_v<T> {
            T ret{};
            uint rsiz = my::g_get(&ret, b, s);
            return tuple_ret{rsiz, std::move(ret)};
        }
        static uint encode(const T &o, byte *b, size_t s){
            return my::g_set(&o, b, s);
        }
    };
}
