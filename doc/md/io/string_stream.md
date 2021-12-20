# `string_stream`

An **string_stream** is a simple string buffer that allow you to build new strings defined in `string_stream.hpp` header. Once you create the desired string you can obtain it with one of the following methods:

* `view()`: returns a view of underlying string buffer. **WARNING**: any buffer modification (for example appending new strings) invalidates all instantiated views. Use this function with extreme care;
* `move()`: moves the underlying buffer to a new `adv_string` object. After this operation the buffer will be empty;
* `allocate()`: allocates a new `adv_string` and copy buffer string to it. This consumes more resources than `move()` but preserves the buffer.

You can access `string_stream` both as a character input stream and as a character output stream. Also you can use it in order to receive/send characters from an input stream/to an output stream via `get_char` and `put_char` respectively and similiar functions, see `string_stream.hpp` header.
