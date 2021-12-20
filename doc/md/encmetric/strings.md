# Strings
An **adv_string_view** object is simply a view of an existing character encoded string (like che C `const char *` strings) that doesn't own the pointed data, so copying an adv_string_view doesn't automatically copy also the underlying string. Instead an `adv_string` object is more similar to C++ `std::string` object: it allocates enough space in order to contain its string. Copying and initializying a new adv_string also copy the encoded string, so you should usually use adv_string_view instead of adv_string if you don't need to manipulate your strings.

You can initialize new strings and new string views with `alloc_string` and `new_string_view` functions respectively (or with their constructor if you use gcc 11.2 or later)

    adv_string_view<UTF8> a = new_string_view<UTF8>(u8"Hello");
    adv_string<UTF16LE> b = alloc_string<UTF16LE>(u"World");
    adv_string_view<WIDEchr> wide = new_string_view<WIDEchr>(U"Azz", DynEncoding<UTF32LE>::instance());
    adv_string_view<ASCII>{"ASCII"}; //with gcc>=11.2

You can perform all tha basic string operations on an `adv_string_view`/`adv_string` class, for more informations see their class definitions in `strsuite/encmetric/enc_string.hpp` header file.

## String literals
In StrSuite you can build some UTF string views directly from string literals by using the `_asv` suffix. For example

* literal `u8"..."_asv` returns an UTF-8 encoded string view (`adv_string_view<UTF8>`);
* literal `u"..."_asv` returns an UTF16SYS encoded string where UTF16SYS may be UTF16LE or UTF16BE depending of endianess of current enfironment;
* literal `U"..."_asv` returns an UTF32SYS encoded string where UTF32SYS may be UTF32LE or UTF32BE depending of endianess of current enfironment.

**Notice** that while by default you can use string literal `"..."_asv` to build ASCII string views some compilers (for example gcc) have the ability of changing the encoding of these narrowed literals (via the option `-fexec-charset=` for example) and currently StringSuite is not able to detect this variation. Use always `u8` literals or use `STS_IO_asv` macro as explained in the sext section.
