# StringSuite
![GitHub release (latest by date)](https://img.shields.io/github/v/release/Loara/Encmetric?color=brightgreen)
![GitHub](https://img.shields.io/github/license/Loara/Encmetric?color=blue&label=License&style=plastic)
![GitHub top language](https://img.shields.io/github/languages/top/Loara/Encmetric?color=blue)
[![GitHub issues](https://img.shields.io/github/issues/Loara/Encmetric)](https://github.com/Loara/Encmetric/issues)

C++ library to manage strings and (almost) any kind of encoded data.

# License
Encmetric is written under the GNU Lesser General Public License (LGPL) version 2.1. For more informations see LICENSE files

# Build and Install
To build the library you need cmake version 3.20 or later and a C++20 compiler.

To build and install the library

    cmake src/
    cmake --build .
    cmake --install .

# Encodings currently included in this library (v. 1.0)
* ASCII
* Latin1 / ISO-8859-1
* ISO-8859-2
* UTF8
* UTF16
* UTF32
* Base64
* Windows codepage 1252
* Windows codepage 1250

Clearly you can write and use your own encoding classes.
