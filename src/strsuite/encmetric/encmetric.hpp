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
#include <strsuite/encmetric/encoding.hpp>

namespace sts{


template<typename tt>
class EncMetric{
	public:
		using ctype=tt;
		virtual ~EncMetric() {}
		virtual uint d_min_bytes() const noexcept=0;
		virtual uint d_chLen(const byte *, size_t) const=0;
		virtual validation_result d_validChar(const byte *, size_t) const noexcept =0;
		virtual tuple_ret<ctype> d_decode(const byte *, size_t) const =0;
		virtual uint d_encode(const ctype &, byte *, size_t) const =0;

		virtual bool d_has_max() const noexcept=0;
		virtual uint d_max_bytes() const=0;

		virtual bool d_fixed_size() const noexcept =0;
		virtual std::type_index index() const noexcept=0;

        virtual bool d_has_head() const noexcept=0;
        virtual uint d_head() const=0;
        /*
         * If it doesn't have enc_base must return nullptr
         */
        virtual const EncMetric<tt> *d_enc_base() const noexcept =0;
};



template<typename tt>
bool is_base_for_d(const EncMetric<tt> *a, const EncMetric<tt> *b) noexcept{
    if(a == nullptr || b == nullptr)
        return false;
    if(a->index() == b->index())
        return true;
    return is_base_for_d(a, b->d_enc_base());
}
template<typename tt>
bool is_extension_for_d(const EncMetric<tt> *a, const EncMetric<tt> *b) noexcept{
    return is_base_for_d(b, a);
}

template<typename tt>
int min_length(int nchr, const EncMetric<tt> *format) noexcept{
	return format->d_min_bytes() * nchr;
}

template<typename tt>
int max_length(uint nchr, const EncMetric<tt> *format){
	if(format->d_has_max())
		return format->d_max_bytes() * nchr;
	else
		throw encoding_error{"This encoding has no superior limit"};
}

/*
    Wrapper of an encoding T in order to save it in a class field of WIDENC classes

    WARNING: T will be never RAW
*/
template<strong_enctype T>
class DynEncoding : public EncMetric<typename T::ctype>{
	private:
		DynEncoding() noexcept {}
	public:
        static_assert(!enc_raw<T>, "EncMetric cannot hold a RAW encoding");
		using static_enc = T;
        using ctype = typename T::ctype;

		~DynEncoding() {}

		constexpr uint d_min_bytes() const noexcept {return static_enc::min_bytes();}
		constexpr bool d_has_max() const noexcept {return feat::has_max<T>::value;}
		constexpr uint d_max_bytes() const {
            if constexpr(feat::has_max<T>::value)
                return feat::has_max<T>::get_max();
            else
                throw encoding_error{"This encoding has no superior limit"};
        }
		uint d_chLen(const byte *b, size_t siz) const {return static_enc::chLen(b, siz);}
		validation_result d_validChar(const byte *b, size_t siz) const noexcept {return static_enc::validChar(b, siz);}
		std::type_index index() const noexcept {return std::type_index{typeid(T)};}

		tuple_ret<ctype> d_decode(const byte *by, size_t l) const {return static_enc::decode(by, l);}
		uint d_encode(const ctype &uni, byte *by, size_t l) const {return static_enc::encode(uni, by, l);}

		bool d_fixed_size() const noexcept {return feat::fixed_size<T>::value;}

        bool d_has_head() const noexcept{return feat::opt_head<T>::value;}
        uint d_head() const{
            if constexpr(feat::opt_head<T>::value)
                return feat::opt_head<T>::get_head();
            else
                throw encoding_error{"This encoding has no head/tail structure"};
        }

		static const EncMetric<ctype> *instance() noexcept{
			static DynEncoding<T> t{};
			return &t;
		}

        const EncMetric<ctype> *d_enc_base() const noexcept{
            if constexpr(has_alias<T>)
                return DynEncoding<typename T::enc_base>::instance();
            else
                return nullptr;
        }
};

/*
    Store information about used encoding
*/
template<general_enctype T>
class EncMetric_info{
	public:
		using ctype=typename T::ctype;
		constexpr EncMetric_info(const EncMetric_info<T> &) noexcept {}
		constexpr EncMetric_info() noexcept {}
		const EncMetric<ctype> *format() const noexcept {return DynEncoding<T>::instance();}

		constexpr uint min_bytes() const noexcept {return T::min_bytes();}

		constexpr bool has_max() const noexcept {return feat::has_max<T>::value;}
		constexpr uint max_bytes() const noexcept requires feat::has_max<T>::value {return feat::has_max<T>::get_max();}
		/*
         * Use only for runtime erroring
         */
		uint max_bytes() const requires (!feat::has_max<T>::value) {throw encoding_error{"This encoding has no superior limit"};}

		constexpr bool has_head() const noexcept {return feat::opt_head<T>::value;}
		constexpr uint head() const noexcept requires feat::opt_head<T>::value {return feat::opt_head<T>::get_head();}
		uint head() const requires (!feat::opt_head<T>::value) {throw encoding_error{"This encoding has no head/tail structure"};}

		constexpr bool is_fixed() const noexcept {return feat::fixed_size<T>::value;}

		uint chLen(const byte *b, size_t siz) const {return T::chLen(b, siz);}
		validation_result validChar(const byte *b, size_t l) const noexcept {return T::validChar(b, l);}
		[[nodiscard]] tuple_ret<ctype> decode(const byte *by, size_t l) const {return T::decode(by, l);}
		uint encode(const ctype &uni, byte *by, size_t l) const {return T::encode(uni, by, l);}
		std::type_index index() const noexcept {return DynEncoding<T>::index();}

		template<general_enctype S>
		constexpr bool equalTo(EncMetric_info<S>) const noexcept requires not_widenc<S>{
            return same_enc<S, T>;
        }
		template<general_enctype S>
		bool equalTo(EncMetric_info<S> o) const noexcept requires widenc<S>{
            return index() == o.index();
        }

        template<general_enctype S>
        void assert_same_enc([[maybe_unused]] EncMetric_info<S> o) const{
            if constexpr(strong_enctype<S>){
                static_assert(same_enc<T, S>, "Different encodings");
            }
            else{
                if(index() != o.index())
                    throw incorrect_encoding{"Different encodings"};
            }
        }

        template<general_enctype S> requires not_widenc<S>
        constexpr bool base_for(EncMetric_info<S>) const noexcept{
            static_assert(same_data<T, S>, "Inconvertible encodings");
            return is_base_for<T, S>;
        }
        template<general_enctype S> requires widenc<S>
        bool base_for(EncMetric_info<S> b) const noexcept{
            static_assert(same_data<T, S>, "Inconvertible encodings");
            return is_base_for_d(format(), b.format());
        }

        template<general_enctype S>
        void assert_base_for([[maybe_unused]] EncMetric_info<S> b) const{
            static_assert(same_data<T, S>, "Cannot convert these encodings");
            if constexpr(strong_enctype<S>){
                static_assert(is_base_for<T, S>, "Cannot convert these encodings");
            }
            else{
                if(!is_base_for_d(format(), b.format()))
                    throw incorrect_encoding{"Cannot convert these encodings"};
            }
        }
};

template<typename tt>
class EncMetric_info<WIDE<tt>>{
	private:
		const EncMetric<tt> *f;
	public:
		using ctype=tt;
		constexpr EncMetric_info(const EncMetric<tt> *format) noexcept : f{format} {}
		constexpr EncMetric_info(const EncMetric_info &info) noexcept : f{info.f} {}
		constexpr const EncMetric<tt> *format() const noexcept {return f;}

		uint min_bytes() const noexcept {return f->d_min_bytes();}

		bool has_max() const noexcept {return f->d_has_max();}
		uint max_bytes() const {return f->d_max_bytes();}

		bool is_fixed() const noexcept {return f->d_fixed_size();}

		bool has_head() const noexcept {return f->d_has_head();}
		uint head() const {return f->d_head();}

		uint chLen(const byte *b, size_t siz) const {return f->d_chLen(b, siz);}
		validation_result validChar(const byte *b, size_t l) const noexcept {return f->d_validChar(b, l);}
		[[nodiscard]] tuple_ret<ctype> decode(const byte *by, size_t l) const {return f->d_decode(by, l);}
		uint encode(const ctype &uni, byte *by, size_t l) const {return f->d_encode(uni, by, l);}
		std::type_index index() const noexcept {return f->index();}

		template<typename S>
		bool equalTo(EncMetric_info<S> o) const noexcept{
            static_assert(general_enctype_of<S, tt>, "Inconvertible types");
            return index() == o.index();
        }
        template<typename S>
        void assert_same_enc(EncMetric_info<S> o) const{
            static_assert(general_enctype_of<S, tt>, "Inconvertible types");
            if(index() != o.index())
                throw incorrect_encoding{"Different encodings"};
        }
        template<typename S>
        bool base_for(EncMetric_info<S> b) const noexcept{
            return is_base_for_d(format(), b.format());
        }

        template<typename S>
        void assert_base_for(EncMetric_info<S> b) const{
            static_assert(general_enctype_of<S, tt>, "Cannot convert these encodings");
            if(!is_base_for_d(format(), b.format()))
                throw incorrect_encoding{"Cannot convert these encodings"};
        }
};

}
