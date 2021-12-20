#pragma once
/*
    This file is part of Encmetric.
    Copyright (C) 2021 Paolo De Donato.

    Encmetric is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Encmetric is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Encmetric. If not, see <http://www.gnu.org/licenses/>.
*/
#include <cstdio>
#include <strsuite/encmetric/byte_tools.hpp>
#include <strsuite/io/char_stream.hpp>
#include <strsuite/io/middle_enc.hpp>

namespace sts{

enum class file_type {read, write_trunc, write_append};

template<file_type fg>
class file_wrapper{
private:
    std::FILE *file;
    const char *compose_mode() const noexcept{
        if constexpr(fg == file_type::read)
            return "rb";
        else if constexpr(fg == write_trunc)
            return "wb";
        else
            return "ab";
    }

public:
    file_wrapper(const char *fname) : file{nullptr} {
        file = std::fopen(fname, compose_mode());
    }
    ~file_wrapper(){
        if(file != nullptr)
            std::fclose(file);
    }
    void close(){
        std::fclose(file);
        file = nullptr;
    }

    size_t read(byte *b, size_t siz) requires (fg == read){
        std::size_t ret = fread(b, 1, siz, file);
        if(std::feof(file))
            throw IOEOF{};
        else if(std::ferror(file))
            throw IOFail{};
        else return ret;
    }

    size_t write(const byte *b, size_t siz) requires (fg == write_append || fg == write_trunc){
        size_t ret = fwrite(b, 1, siz, file);
        if(std::feof(file))
            throw IOEOF{};
        else if(std::ferror(file))
            throw IOFail{};
        else return ret;
    }
};

}
