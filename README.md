# StringSuite
![GitHub release (latest by date)](https://img.shields.io/github/v/release/Loara/Encmetric?color=brightgreen)
![GitHub](https://img.shields.io/github/license/Loara/Encmetric?color=blue&label=License&style=plastic)
![GitHub top language](https://img.shields.io/github/languages/top/Loara/Encmetric?color=blue)
[![GitHub issues](https://img.shields.io/github/issues/Loara/Encmetric)](https://github.com/Loara/Encmetric/issues)

C++ library to manage strings and (almost) any kind of encoded data.

# License
Encmetric is written under the GNU Lesser General Public License (LGPL) version 3. For more informations see COPYING and COPYING.LESSER files

# Build and Install
To build the library you need cmake version 3.20 or later and a C++ compiler that supports C++20 concepts (for example `gcc` v. 10.2 or newer).

To build and install the library you can run these commands:

    cmake src/
    cmake --build .
    cmake --install .

## Arch Linux
On Arch Linux you can use package `stringsuite` in AUR, additional informations can be found at https://aur.archlinux.org/packages/stringsuite/.

# Encodings currently included in this library
* ASCII
* Latin1 / ISO-8859-1, ISO-8859-2
* UTF8, UTF16, UTF32
* KOI8-R, KOI8-U, KOI8-RU
* Base64
* Windows codepage 1252, 1250

Clearly you can write and use your own encoding classes.

# Basic usage
## String encodings
A string encoding class is simply a static class that manages how a string is encoded with that encoding. Some encoding classes provided with StringSuire are `ASCII`, `UTF8`, `UTF16BE` `UTF16LE`, `UTF32BE` `UTF32LE`, `Latin1`/`ISO_8859_1`, `ISO_8859_2`, `KOI8_R`, `KOI8_U`, `KOI8_RU`.

Usually encoding classes should be specified as template arguments of string classes, but you can decide todynamically specify an encoding by using the `WIDEchr` template argument. Remember that if you initialize any string with the `WIDEchr` template argument you should pass a dynamic pointer to `EncMetric<unicode>` class representing your encoding, this can be obtained via the `DynEncoding` template argument. For example a dynamic pointer of `UTF8` encofing can be obtained with

    const EncMetric<unicode> *utf8 = DynEncoding<UTF8>::instance();

## String dimensions
Encoded strings have two different types of lengths:

 * **size** is simply the number of bytes occupied by the encoded string, same value returned by `strlen` in C and `std::string::length()`, `std::string::size()` in C++
 * **length** instead is the number of characters encoded in your string.

For example consider the UTF8 encoded string `abè€`: it contains exactly 4 characters but if you run `strlen` or the `std::string::length()` usuallt they return 7. This difference can be explained by how UTF8 encodes these four characters:

1. character `a` is an ASCII character and so in UTF8 is encoded as `0x61`;
2. for the same reason `b` is encoded as `0x62`;
3. character `è` is not an ASCII character, so UTF8 encodes it by using 2 bytes: `0xc3 0xa8`;
4. also `€` is not an ASCII character, and its UTF8 encoding not uses 3 bytes: `0xe2 0x82 0xac`.

Then the size of string `abè€` is exactly 1+1+2+3=7. Strings in StringSuite, despite `std::string`, allow you to detect both the size and the length of any encoded string.

## Strings and string views
An **adv_string_view** object is simply a view of an existing character encoded string (like che C `const char *` strings) that doesn't own the pointed data, so copying an adv_string_view doesn't automatically copy also the underlying string. Instead an `adv_string` object is more similar to C++ `std::string` object: it allocates enough space in order to contain its string. Copying and initializying a new adv_string also copy the encoded string, so you should usually use adv_string_view instead of adv_string if you don't need to manipulate your strings.

You can initialize new strings and new string views with `alloc_string` and `new_string_view` functions respectively (or with their constructor if you use gcc 11.2 or later)

    adv_string_view<UTF8> a = new_string_view<UTF8>(u8"Hello");
    adv_string<UTF16LE> b = alloc_string<UTF16LE>(u"World");
    adv_string_view<WIDEchr> wide = new_string_view<WIDEchr>(U"Azz", DynEncoding<UTF32LE>::instance());
    adv_string_view<ASCII>{"ASCII"}; //with gcc>=11.2

You can perform all tha basic string operations on an `adv_string_view`/`adv_string` class, for more informations see their class definitions in `strsuite/encmetric/enc_string.hpp` header file.

## String buffer
An **adv_string_buf** is a simple string buffer that allow you to build new strings. It can also perform **encoding conversions** so you should use it in order to perform encoding conversions (for example UTF8 from an UTF16 string).

Once you create the desired string you can obtain it with one of the following methods:

* `view()`: returns a view of underlying string buffer. **WARNING**: any buffer modification (for example appending new strings) invalidates all instantiated views. Use this function with extreme care;
* `move()`: moves the underlying buffer to a new `adv_string` object. After this operation the buffer will be empty;
* `allocate()`: allocates a new `adv_string` and copy buffer string to it. This consumes more resources than `move()` but preserves the buffer.
