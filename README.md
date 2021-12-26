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
See [Included encodings](doc/md/encmetric/encods.md) for an up-to-date list of included encodings sort by library version.

# Basic documentation (work in progress)
You can take a look at our [index](doc/md/Index.md) for a quick introduction to `StringSuite` library. Additional informations can be took inside the source code.
