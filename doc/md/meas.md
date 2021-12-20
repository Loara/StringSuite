# Size and length

Every string has two different parameter in order to measure its length:

 * **size** is simply the number of bytes occupied by the encoded string, same value returned by `strlen` in C and `std::string::length()`, `std::string::size()` in C++
 * **length** instead is the number of characters encoded in your string.

For example consider the UTF8 encoded string `abè€`: it contains exactly 4 characters but if you run `strlen` or the `std::string::length()` usuallt they return 7. This difference can be explained by how UTF8 encodes these four characters:

1. character `a` is an ASCII character and so in UTF8 is encoded as `0x61`;
2. for the same reason `b` is encoded as `0x62`;
3. character `è` is not an ASCII character, so UTF8 encodes it by using 2 bytes: `0xc3 0xa8`;
4. also `€` is not an ASCII character, and its UTF8 encoding not uses 3 bytes: `0xe2 0x82 0xac`.

Then the size of string `abè€` is exactly 1+1+2+3=7. Strings in StringSuite, despite `std::string`, allow you to detect both the size and the length of any encoded string.

These parameter are always equal on single byte encoding like ASCII or ISO 8859-1, so the majority of legacy code continues to confuse size with length and so facing many problems with multibyte encodings like UTF8. In StringSuite these two concepts are distinct so tou can find different functions in order to calculate size and length of an encoded string.
