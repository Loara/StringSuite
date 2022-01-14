# Build options

You can specify cmake options in order to customize your build. Options are passed via `-D` arguments with this syntax:

<pre>
cmake -D<i>option1</i>=<i>ON/OFF</i> -D<i>option2</i>=<i>ON/OFF</i> ... <i>build_dir</i>
</pre>

for example in order to avoid installing header files you should execute

    cmake -S src -B build
    cmake -DINSTALL_HEADERS=OFF build
    cmake --build build
    cmake --install build

Options have to be specified before to run `cmake --build` command.

You can retrieve all the customizable options via `cmake -LH` command or by using cmake-gui program. Some of these options are:

## `LIBRARY_DIR`
Path (absolute or relative to `CMAKE_INSTALL_PREFIX`) where to install library files.

**Default**: System-dependent.

## `INSTALL_HEADERS`
If set to *ON* all the header files are installed with the library.

**Default**: ON.

## `HEADER_DIR`
Path (absolute or relative to `CMAKE_INSTALL_PREFIX`) where to install header files.

**Default**: System-dependent.

## `INSTALL_PKGCONFIG`
If set to *ON* a `.pc` file will be generated and installed in order to make StringSuite findable by `pkg-confing` utility via the `stringsuite` name.

**Default**: OFF.

## `PKGCONFIG_DIR`
Path (absolute or relative to `CMAKE_INSTALL_PREFIX`) where to install pkg-confing manifest files.

**Default**: System-dependent.
