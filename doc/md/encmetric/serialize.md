# Serialization

## Why there is not any `serialize_traits` class type?

We won't provide a general-purpose encoding class to serialize arbitrary objects for these reasons:

* An object can be serialized in multiple ways and we don't want to prefer a mechanism rather than another;
* Ad-hoc serialization algorithms for each type can considerably reduce size and be more efficient;
* Sometimes you don't want to memorize all data member, and a custom algoritm allow you to hide those informations and recover them during decoding.

So if you want to serialize an object of type `T` you have to create an encoding class with `ctype=T` and define the static member functions `min_bytes`, `chLen`, `decode` and `encode`.

## But I want to serialize a simple class without using any optimization and ad-hoc algorithm.

In that case you can use the `quick_serialize<typename T, typename... Param>` template class in order to create a quick encoding class for type `T`. It's recomended to aliasing it in order to avoid writing all the template parameter each times:

    using T_serialize = sts::quick_serialize<T, ...>;

Param `T` is the type you want to rerialize, each `Param` is a class template argument saying how data member should be encoded/decoded. Actually only these parameter types are allowed:

`sts::q_param::enc_member<typename T, typename ty, ty T::* member, strong_enctype Enc = sts::Endian_enc<false, ty>>`

Encode and decode the member `member` of type `ty` by using the `Enc` encoding.

`sts::q_param::noenc_member<typename T, typename ty, ty T::* member, ty (*func)()>`

Doesn't encode `member`, instead when decoding initialize `member` with `func()`.
