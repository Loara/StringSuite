# Default Console encodings and streams

## `STS_IO_asv` and `IOenc`
UNIX systems by default works with UTF8 encoded strings whereas Windows uses UTF16 (little endian) encoding (not considering all the Windows codepages). StringSuite provides the macro `STS_IO_asv` in order to build UTF8 literals on UNIX systems and UTF16 literals on Windows.

StringSuite provides also the `IOenc` encoding type alias in order to receive any `STS_IO_asv` string view and to work with basyc system IO streams.

    adv_string_view<IOenc> u = STS_IO_asv("Hi");
    /*
     * equivalent to adv_string_view<UTF8> u = u8"Hi"_asv on UNIX systems
     * equivalent to adv_string_view<UTF16LE> u = u"Hi"_asv on Windows systems
     */

## Default stdin, stdout, stderr
You can access console `IOenc` encoded standard streams `stdin, stdout, stderr` by calling respectively `get_console_stdin()`, `get_console_stdout()`, `get_console_stderr()`. For all available operations see also `char_stream.hpp`, `nl_stream.hpp` file headers.
