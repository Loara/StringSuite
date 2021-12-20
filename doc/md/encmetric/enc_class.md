# Encoding classes

An encoding class is simply a static class that manages how a string is encoded with that encoding. Some encoding classes provided with StringSuire are `ASCII`, `UTF8`, `UTF16BE` `UTF16LE`, `UTF32BE` `UTF32LE`, `Latin1`/`ISO_8859_1`, `ISO_8859_2`, `KOI8_R`, `Shift_JIS`, ...

Encoding classes are not forced to work with character strings, but can be defined for every serializable object that can be stored sequentially in a byte stream. In order to create an encoding class for a serializable object these assumptions must be satisfied:

1. the object must be encoded in the same manner regardless of its position inside a string;
2. exists an universal integer *n* such that the size of any encoded object is always greater or equal to *n* and in general the size should be determined only by using its first *n* bytes.

Any encoding class has to define these types and static members in order to be recognized as encoding class:

* a `ctype` type representing the type should be encoded, for encodings compatible with Unicode this should be `sts::unicode`;
* a `consteval min_bytes` static member function without arguments and returning `sts::uint` (equal to `unsigned int`) representing the minimum number of bytes required to determine the size of any encoded object;
* a `chLen` static member function accepting `const sts::byte *` and `size_t` as arguments, representing respectively a pointer to encoded data and the size of such data in bytes, and returning `sts::uint` representing the size of the first encoded object pointed by pointer argument. If the data size is too small this function must throw an `sts::buffer_small` exception, instead if is impossible to determine the size because the data is invalid an `sts::incorrect_encoding` must be thrown;
