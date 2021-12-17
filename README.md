# StringSuite
![GitHub release (latest by date)](https://img.shields.io/github/v/release/Loara/StringSuite?color=brightgreen&style=for-the-badge)
![GitHub](https://img.shields.io/github/license/Loara/StringSuite?color=brightgreen&style=for-the-badge)
![GitHub](https://img.shields.io/static/v1?label=C%2B%2B&message=20%2B&color=blue&style=for-the-badge)
![GitHub](https://img.shields.io/static/v1?label=Platforms&message=Linux%2C%20OSX%2C%20Windows&color=blue&style=for-the-badge)
![GitHub issues](https://img.shields.io/github/issues/Loara/StringSuite?style=for-the-badge)

C++ library to manage strings and (almost) any kind of encoded data.

# License
Encmetric is written under the GNU Lesser General Public License (LGPL) version 3. For more informations see COPYING and COPYING.LESSER files

# Build and Install
To build the library you need cmake version 3.20 or later and a C++ compiler that supports C++20 concepts (for example `gcc` v. 11.2 or newer).

To build and install the library you can run these commands:

    cmake -S src -B build
    cmake --build build
    cmake --install build

## Arch Linux
On Arch Linux you can use package `stringsuite` in AUR, additional informations can be found at https://aur.archlinux.org/packages/stringsuite/.

# Encodings currently included in this library
* ASCII
* Latin1 / ISO-8859-1, ISO-8859-2
* UTF8, UTF16, UTF32
* KOI8-R, KOI8-U, KOI8-RU
* Base64
* Windows codepages 1250, 1251, 1252
* EUC-JP, Shift-JIS (experimental)

Clearly you can write and use your own encoding classes.

# Basic usage
## `encmetric` subset
Include `strsuite/encmetric.hpp` header in order to use this module.
### String encodings
A string encoding class is simply a static class that manages how a string is encoded with that encoding. Some encoding classes provided with StringSuire are `ASCII`, `UTF8`, `UTF16BE` `UTF16LE`, `UTF32BE` `UTF32LE`, `Latin1`/`ISO_8859_1`, `ISO_8859_2`, `KOI8_R`, `KOI8_U`, `KOI8_RU`.

Usually encoding classes should be specified as template arguments of string classes, but you can decide todynamically specify an encoding by using the `WIDEchr` template argument. Remember that if you initialize any string with the `WIDEchr` template argument you should pass a dynamic pointer to `EncMetric<unicode>` class representing your encoding, this can be obtained via the `DynEncoding` template argument. For example a dynamic pointer of `UTF8` encofing can be obtained with

    const EncMetric<unicode> *utf8 = DynEncoding<UTF8>::instance();

### String dimensions
Encoded strings have two different types of lengths:

 * **size** is simply the number of bytes occupied by the encoded string, same value returned by `strlen` in C and `std::string::length()`, `std::string::size()` in C++
 * **length** instead is the number of characters encoded in your string.

For example consider the UTF8 encoded string `abè€`: it contains exactly 4 characters but if you run `strlen` or the `std::string::length()` usuallt they return 7. This difference can be explained by how UTF8 encodes these four characters:

1. character `a` is an ASCII character and so in UTF8 is encoded as `0x61`;
2. for the same reason `b` is encoded as `0x62`;
3. character `è` is not an ASCII character, so UTF8 encodes it by using 2 bytes: `0xc3 0xa8`;
4. also `€` is not an ASCII character, and its UTF8 encoding not uses 3 bytes: `0xe2 0x82 0xac`.

Then the size of string `abè€` is exactly 1+1+2+3=7. Strings in StringSuite, despite `std::string`, allow you to detect both the size and the length of any encoded string.

### Strings and string views
An **adv_string_view** object is simply a view of an existing character encoded string (like che C `const char *` strings) that doesn't own the pointed data, so copying an adv_string_view doesn't automatically copy also the underlying string. Instead an `adv_string` object is more similar to C++ `std::string` object: it allocates enough space in order to contain its string. Copying and initializying a new adv_string also copy the encoded string, so you should usually use adv_string_view instead of adv_string if you don't need to manipulate your strings.

You can initialize new strings and new string views with `alloc_string` and `new_string_view` functions respectively (or with their constructor if you use gcc 11.2 or later)

    adv_string_view<UTF8> a = new_string_view<UTF8>(u8"Hello");
    adv_string<UTF16LE> b = alloc_string<UTF16LE>(u"World");
    adv_string_view<WIDEchr> wide = new_string_view<WIDEchr>(U"Azz", DynEncoding<UTF32LE>::instance());
    adv_string_view<ASCII>{"ASCII"}; //with gcc>=11.2

You can perform all tha basic string operations on an `adv_string_view`/`adv_string` class, for more informations see their class definitions in `strsuite/encmetric/enc_string.hpp` header file.

### String literals
In StrSuite you can build some UTF string views directly from string literals by using the `_asv` suffix. For example

* literal `u8"..."_asv` returns an UTF-8 encoded string view (`adv_string_view<UTF8>`);
* literal `u"..."_asv` returns an UTF16SYS encoded string where UTF16SYS may be UTF16LE or UTF16BE depending of endianess of current enfironment;
* literal `U"..."_asv` returns an UTF32SYS encoded string where UTF32SYS may be UTF32LE or UTF32BE depending of endianess of current enfironment.

**Notice** that while by default you can use string literal `"..."_asv` to build ASCII string views some compilers (for example gcc) have the ability of changing the encoding of these narrowed literals (via the option `-fexec-charset=` for example) and currently StringSuite is not able to detect this variation. Use always `u8` literals or use `STS_IO_asv` macro as explained in the sext section.

## `io` subset
Include `strsuite/io.hpp` header in order to use this module.
### `STS_IO_asv` and `IOenc`
UNIX systems by default works with UTF8 encoded strings whereas Windows uses UTF16 (little endian) encoding (not considering all the Windows codepages). StringSuite provides the macro `STS_IO_asv` in order to build UTF8 literals on UNIX systems and UTF16 literals on Windows.

StringSuite provides also the `IOenc` encoding type alias in order to receive any `STS_IO_asv` string view and to work with basyc system IO streams.

    adv_string_view<IOenc> u = STS_IO_asv("Hi");
    /*
     * equivalent to adv_string_view<UTF8> u = u8"Hi"_asv on UNIX systems
     * equivalent to adv_string_view<UTF16LE> u = u"Hi"_asv on Windows systems
     */

### Default stdin, stdout, stderr
You can access console `IOenc` encoded standard streams `stdin, stdout, stderr` by calling respectively `get_console_stdin()`, `get_console_stdout()`, `get_console_stderr()`. For all available operations see also `char_stream.hpp`, `nl_stream.hpp` file headers.

### `string_stream`
An **string_stream** is a simple string buffer that allow you to build new strings defined in `string_stream.hpp` header. Once you create the desired string you can obtain it with one of the following methods:

* `view()`: returns a view of underlying string buffer. **WARNING**: any buffer modification (for example appending new strings) invalidates all instantiated views. Use this function with extreme care;
* `move()`: moves the underlying buffer to a new `adv_string` object. After this operation the buffer will be empty;
* `allocate()`: allocates a new `adv_string` and copy buffer string to it. This consumes more resources than `move()` but preserves the buffer.

You can access `string_stream` both as a character input stream and as a character output stream. Also you can use it in order to receive/send characters from an input stream/to an output stream via `get_char` and `put_char` respectively and similiar functions, see `string_stream.hpp` header.
