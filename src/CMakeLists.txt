cmake_minimum_required(VERSION 3.20)

project(StringSuite VERSION 3.1 DESCRIPTION "Library to manage encoded strings" HOMEPAGE_URL "https://github.com/Loara/StringSuite" LANGUAGES CXX)

include (GNUInstallDirs)

#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED True)
#see later

add_library(lang_req INTERFACE)

#require C++20
target_compile_features(lang_req INTERFACE cxx_std_20)

#enable warnings
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
set(on_win "$<PLATFORM_ID:Windows>")

#options
set(LIBRARY_DIR "${CMAKE_INSTALL_LIBDIR}" CACHE PATH "Library directory")

option(INSTALL_HEADERS "Install header files" ON)
set(HEADER_DIR "${CMAKE_INSTALL_INCLUDEDIR}" CACHE PATH "C++ header files directory")

option(INSTALL_PKGCONFIG "Generate and install pkg-config manifest file" OFF)
set(PKGCONFIG_DIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig" CACHE PATH "Dpkg-config manifest files directory")

target_compile_options(lang_req INTERFACE
  "$<${gcc_like_cxx}:$<$<CONFIG:Debug>:-g;-Wall;-Wextra;-pedantic;-Werror>$<$<CONFIG:Release>:-O2>>"
  "$<${msvc_cxx}:$<$<CONFIG:Debug>:/Wall;/WX>>"
)

#if(CMAKE_SYSTEM_NAME MATCHES Win)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	configure_file(win_enc_io_core.cpp.in sys_enc_io_core.cpp COPYONLY)
else()
	configure_file(linux_enc_io_core.cpp.in sys_enc_io_core.cpp COPYONLY)
endif()

add_library(strsuite STATIC
    byte_tools.cpp
    basic_ptr.cpp
    encoding.cpp
    utf8_enc.cpp
    enc_c.cpp
    utf32_enc.cpp
    utf16_enc.cpp
    iso8859_enc.cpp
    koi8.cpp
    win_codepages.cpp
    sys_enc_io_core.cpp
    base64.cpp
    jis.cpp)

target_link_libraries(strsuite PUBLIC lang_req)

#headers
target_include_directories(strsuite PUBLIC "${PROJECT_SOURCE_DIR}")
target_compile_definitions(strsuite PRIVATE "$<${on_win}:using_windows>")

#install
install(TARGETS strsuite ARCHIVE DESTINATION ${LIBRARY_DIR})

if(INSTALL_PKGCONFIG)

configure_file(stringsuite.pc.in stringsuite.pc @ONLY)
install(FILES "${CMAKE_BINARY_DIR}/stringsuite.pc" DESTINATION ${PKGCONFIG_DIR})

endif()

if(INSTALL_HEADERS)

install(FILES "strsuite/encmetric/all_enc.hpp"
    "strsuite/encmetric/ascii_extensions.hpp"
    "strsuite/encmetric/config.hpp"
    "strsuite/encmetric/base.hpp"
    "strsuite/encmetric/base64.hpp"
    "strsuite/encmetric/endianess.hpp"
    "strsuite/encmetric/fixed_point.hpp"
    "strsuite/encmetric/basic_ptr.hpp"
    "strsuite/encmetric/byte_tools.hpp"
    "strsuite/encmetric/chite.hpp"
    "strsuite/encmetric/enc_c_0.hpp"
    "strsuite/encmetric/enc_c.hpp"
    "strsuite/encmetric/enc_string.hpp"
    "strsuite/encmetric/dynstring.hpp"
    "strsuite/encmetric/encoding.hpp"
    "strsuite/encmetric/features.hpp"
    "strsuite/encmetric/encmetric.hpp"
    "strsuite/encmetric/exceptions.hpp"
    "strsuite/encmetric/iso8859_enc.hpp"
    "strsuite/encmetric/koi8.hpp"
    "strsuite/encmetric/utf8_enc.hpp"
    "strsuite/encmetric/utf16_enc_0.hpp"
    "strsuite/encmetric/utf16_enc.hpp"
    "strsuite/encmetric/utf32_enc_0.hpp"
    "strsuite/encmetric/utf32_enc.hpp"
    "strsuite/encmetric/jis.hpp"
    "strsuite/encmetric/win_codepages.hpp"
    "strsuite/encmetric/type_array.hpp"
    "strsuite/encmetric/serialize.hpp" DESTINATION ${HEADER_DIR}/strsuite/encmetric)

install(FILES "strsuite/io/enc_io_core.hpp"
    "strsuite/io/enc_io_buffer.hpp"
    "strsuite/io/enc_io_exc.hpp"
    "strsuite/io/char_stream.hpp"
    "strsuite/io/string_stream.hpp"
    "strsuite/io/nl_stream.hpp"
    "strsuite/io/buffers.hpp"
    "strsuite/io/cio_stream.hpp"
    "strsuite/io/tokens.hpp" DESTINATION ${HEADER_DIR}/strsuite/io)

install(FILES "strsuite/format/integral_format.hpp"
    "strsuite/format/format_tmp.hpp" DESTINATION ${HEADER_DIR}/strsuite/format)


install(FILES "strsuite/encmetric/chite.tpp"
    "strsuite/encmetric/enc_string.tpp"
    "strsuite/encmetric/dynstring.tpp" DESTINATION ${HEADER_DIR}/strsuite/encmetric)

install(FILES "strsuite/io/nl_stream.tpp"
    "strsuite/io/string_stream.tpp" DESTINATION ${HEADER_DIR}/strsuite/io)

install(FILES "strsuite/format/integral_format.tpp"
    "strsuite/format/format_tmp.tpp" DESTINATION ${HEADER_DIR}/strsuite/format)


install(FILES "strsuite/encmetric.hpp" DESTINATION ${HEADER_DIR}/strsuite)
install(FILES "strsuite/io.hpp" DESTINATION ${HEADER_DIR}/strsuite)
install(FILES "strsuite/format.hpp" DESTINATION ${HEADER_DIR}/strsuite)

endif()
