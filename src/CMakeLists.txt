cmake_minimum_required(VERSION 3.20)

project(StringSuite VERSION 2 DESCRIPTION "Library to manage encoded strings" LANGUAGES CXX)

#require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#windows test
if(CMAKE_SYSTEM_NAME MATCHES Win)
	set(using_windows ON)
	configure_file(win_enc_io_core.cpp.in enc_io_core.cpp)
else()
	set(using_windows OFF)
	configure_file(linux_enc_io_core.cpp.in enc_io_core.cpp)
endif()  

configure_file(strsuite/encmetric/config.hpp.in strsuite/encmetric/config.hpp)

add_library(strsuite
    basic_ptr.cpp
    encoding.cpp
    utf8_enc.cpp
    enc_c.cpp
    utf32_enc.cpp
    utf16_enc.cpp
    iso8859_enc.cpp
    koi8.cpp
    win_codepages.cpp
    enc_io.cpp
    enc_io_core.cpp
    base64.cpp)

#headers
target_include_directories(strsuite PUBLIC "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")

#install
install(TARGETS strsuite DESTINATION lib)
install(FILES "strsuite/encmetric/all_enc.hpp"
    "strsuite/encmetric/ascii_extensions.hpp"
    "strsuite/encmetric/base.hpp"
    "strsuite/encmetric/base64.hpp"
    "strsuite/encmetric/endianess.hpp"
    "strsuite/encmetric/basic_ptr.hpp"
    "strsuite/encmetric/byte_tools.hpp"
    "strsuite/encmetric/chite.hpp"
    "strsuite/encmetric/enc_c_0.hpp"
    "strsuite/encmetric/enc_c.hpp"
    "strsuite/encmetric/enc_io_core.hpp"
    "strsuite/encmetric/enc_io.hpp"
    "strsuite/encmetric/enc_string.hpp"
    "strsuite/encmetric/stringbuild.hpp"
    "strsuite/encmetric/encoding.hpp"
    "strsuite/encmetric/exceptions.hpp"
    "strsuite/encmetric/iso8859_enc.hpp"
    "strsuite/encmetric/koi8.hpp"
    "strsuite/encmetric/utf8_enc.hpp"
    "strsuite/encmetric/utf16_enc_0.hpp"
    "strsuite/encmetric/utf16_enc.hpp"
    "strsuite/encmetric/utf32_enc_0.hpp"
    "strsuite/encmetric/utf32_enc.hpp"
    "strsuite/encmetric/win_codepages.hpp" DESTINATION include/strsuite/encmetric)

install(FILES "strsuite/encmetric/chite.tpp"
    "strsuite/encmetric/enc_string.tpp"
    "strsuite/encmetric/stringbuild.tpp" DESTINATION include/strsuite/encmetric)

install(FILES "strsuite/encmetric.hpp" DESTINATION include/strsuite)
install(FILES "strsuite/tokens.hpp" DESTINATION include/strsuite)

install(FILES "${PROJECT_BINARY_DIR}/strsuite/encmetric/config.hpp" DESTINATION include/strsuite/encmetric)
