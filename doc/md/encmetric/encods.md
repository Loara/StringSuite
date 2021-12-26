# Included encodings

StringSuite provides additional encoding classes for the most common encoding. These classes are listed below.

## Encodings available from version `3.0`

* Integer encodings (`sts::Endian_enc<bool big_endian, typename integer_type>`);
* ASCII (`sts::ASCII`);
* UTF encodings (`sts::UTF8`, `sts::UTF16BE`, `sts::UTF16LE`, `sts::UTF32BE`, `sts::UTF32LE`) and system-endianess aliases (`sts::UTF16SYS`, `sts::UTF32SYS`);
* ISO encodings (`sts::ISO_8859_1`/`sts::Latin1`, `sts::ISO_8859_2`);
* KOI8 encodings (`sts::KOI8_R`, `sts::KOI8_U`, `sts::KOI8_RU`);
* Windows codepages (`sts::Win_1250`, `sts::Win_1251`, `sts::Win_1252`).

### Not Unicode encodings

* JIS encodings (`sts::EUC_JP`, `sts::SHIFT_JIS`) using `ctype=sts::jisx_213`;
* Integer, fixed and floating point number encodings with Little and Big endian;
* Base64 encoding (experimental).
