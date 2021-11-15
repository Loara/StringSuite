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
#include <filesystem>
#include <strsuite/io/enc_io_exc.hpp>
#include <strsuite/io/buffers.hpp>

namespace sts{

class file_base_api{
public:
    virtual void open(const std::filesystem::path &)=0;
    virtual bool is_open()=0;
    virtual void close()=0;
    virtual ~file_base_api();
};

class input_file : public virtual file_base_api{
public:
    virtual size_t read(byte *, size_t)=0;
};

class output_file : public virtual file_base_api{
public:
    virtual size_t write(const byte *, size_t)=0;
};

class IFile_wrapper : public basic_buffer<IFile_wrapper, true, false>{
private:
    input_file *file;
public:
    IFile_wrapper(input_file *p, byte *buffer, size_t dim) : basic_buffer<IFile_wrapper, true, false>{buffer, dim}, file{p} {}
    void open(const std::filesystem::path &p) {file->open(p);}
    bool is_open() {return file->is_open();}
    void close() {file->close();}
    size_t read_wrap(byte *b, size_t len) {return file->read(b, len);}

    void inc_size(uint i){
        if(this->rem < static_cast<size_t>(inc))
            throw IOEOF{};
        size_t wt, tu = static_cast<size_t>(inc);
        bool repeat;
        do{
            repeat=false;
            wt = file->read(this->base + this->las, this->rem);
            if(wt > 0){
                this->raw_las_step(wt);
            }
            if(wt < tu){
                tu -= wt;
                repeat = true;
            }
            else
                tu = 0;
        }
        while(repeat);
    }
};
class OFile_wrapper{
private:
    output_file *file;
public:
    static constexpr size_t buffer_size=64;
    IFile_wrapper(output_file *p) : file{p} {}
    ~IFile_wrapper(){delete file;}
    void open(const std::filesystem::path &p) {file->open(p);}
    bool is_open() {return file->is_open();}
    void close() {file->close();}
    size_t write_wrap(const byte *b, size_t len) {return file->write(b, len);}
};

/*
 * Open files with specified encoding, it opens it in binary mode
 */
template<general_enctype T>
CharIStream<T> open_ifile(const std::filesystem::path &, Encmetric_info<T>);
template<general_enctype T>
CharOStream<T> open_ofile(const std::filesystem::path &, Encmetric_info<T>);

template<strong_enctype T>
CharIStream<T> open_ifile(const std::filesystem::path &p) {return open_ifile(p, Encmetric_info<T>{});}
template<strong_enctype T>
CharOStream<T> open_ofile(const std::filesystem::path &p)  {return open_ofile(p, Encmetric_info<T>{});}
template<widenc T>
CharIStream<T> open_ifile(const std::filesystem::path &p, const EncMetric<typename T::ctype> *f) {return open_ifile(p, Encmetric_info<T>{f});}
template<widenc T>
CharOStream<T> open_ofile(const std::filesystem::path &p, const EncMetric<typename T::ctype> *f)  {return open_ofile(p, Encmetric_info<T>{f});}

}



