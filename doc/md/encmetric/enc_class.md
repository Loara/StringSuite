# Encoding classes

An encoding class is simply a static class that manages how a string is encoded with that encoding. Some encoding classes provided with StringSuire are `ASCII`, `UTF8`, `UTF16BE` `UTF16LE`, `UTF32BE` `UTF32LE`, `Latin1`/`ISO_8859_1`, `ISO_8859_2`, `KOI8_R`, `Shift_JIS`, ...

Encoding classes are not forced to work with character strings, but can be defined for every serializable object that can be stored sequentially in a byte stream. In order to create an encoding class for a serializable object these assumptions must be satisfied:

1. the object must be encoded in the same manner regardless of its position inside a string;
2. given an arbitrary sequence of bytes it's always possible to determine if a character is encoded inside and decode it without using any external information.

## Required fields

Any encoding class has to define these types and static members in order to be recognized as encoding class:

* a `ctype` type representing the type should be encoded, for encodings compatible with Unicode this should be `sts::unicode`;
* a `consteval sts::uint min_bytes() noexcept` static member function (where `sts::uint = unsigned int`) representing the minimum number of bytes of any encoded character (not necessarly the minimum number required to decode its size);
* a `sts::uint chLen(const sts::byte *ptr, size_t dim)` static member function returning the size of the first character encoded in the buffer pointed by `ptr` with size `dim`. If the data size is too small this function must throw an `sts::buffer_small` exception, instead if is impossible to determine the size because the data is invalid an `sts::incorrect_encoding` must be thrown;
* a `sts::validation_result validChar(const sts::byte *ptr, size_t dim) noexcept` static member function that detect if `ptr` points to a valid encoded character. If yes then it returns `true` and the size of encoded character (packed inside `sts::validation_result{bool, const sts::uint &}`), otherwise should return `false`;
* a `sts::tuple_ret<ctype> decode(const sts::byte *ptr, size_t dim)` static member function to decode the character encoded in `ptr`, it returns both the decoded character and its size inside a `sts::tuple_ret<ctype>` object;
* a `sts::uint encode(const ctype &chr, sts::byte *ptr, size_t dim)` static member function to encode character `chr` on `ptr`, it returns the size of encoded character `chr`.

You can use the `sts::strong_enctype` concept in order to determine if a class is an encoding class too.

## Optional fields

* an `enc_base` type representing the base encoding. An encoding *A* is a base encoding for encoding *B* if and only if every string encoded with *A* is also a valid string encoded by *B* with the same meaning, or equivalently if you encode any character *x* with *A* and you decode it with *B* you should always obtain *x* again.
* a `consteval sts::uint max_bytes() noexcept` static member function returning the maximal size of a single encoded character. If `min_bytes() == max_bytes()` then that encoding is a *fixed-size encoding* and all encoded characters will have the same size.
