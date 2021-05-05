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
#include <fstream>
#include <iostream>
using namespace std;
/*
 * Estimate the encoding used in your system to represents strings
 */

int main(int argn, const char **args){
    if(argn < 2)
        return 1;
    const char *fname = args[1];
    fstream out{fname, ios_base::out | ios_base::trunc};
    out << "#pragma once" << endl << endl;
    out <<"/*" << endl;
    out << "This file is part of Encmetric." << endl;
    out << "Copyright (C) 2021 Paolo De Donato." << endl << endl;
    out << "Encmetric is free software: you can redistribute it and/or modify" << endl;
    out << "it under the terms of the GNU Lesser General Public License as published by" << endl;
    out << "the Free Software Foundation, either version 3 of the License, or" << endl;
    out << "(at your option) any later version." << endl << endl;
    out << "Encmetric is distributed in the hope that it will be useful," << endl;
    out << "but WITHOUT ANY WARRANTY; without even the implied warranty of" << endl;
    out << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" << endl;
    out << "GNU Lesser General Public License for more details." << endl << endl;
    out << "You should have received a copy of the GNU Lesser General Public License" << endl;
    out << "along with Encmetric. If not, see <http://www.gnu.org/licenses/>." << endl;
    out << "*/" << endl << endl;
    out << "#include<strsuite/encmetric/all_enc.hpp>" << endl << endl;
    out << "namespace sts{" << endl;

    const char *t = "\U00010393\0\0\0\0";
    if(t[0] == '\xf0' && t[1] == '\x90' && t[2] == '\x8e' && t[3] == '\x93')
        out << "using CENC=UTF8;" << endl;
    else if(t[0] == '\xd8' && t[1] == '\x00' && t[2] == '\xdf' && t[3] == '\x93')
        out << "using CENC=UTF16BE;" << endl;
    else if(t[0] == '\x00' && t[1] == '\xd8' && t[2] == '\x93' && t[3] == '\xdf')
        out << "using CENC=UTF16LE;" << endl;
    else if(t[0] == '\x00' && t[1] == '\x01' && t[2] == '\x03' && t[3] == '\x93')
        out << "using CENC=UTF32BE;" << endl;
    else if(t[0] == '\x93' && t[1] == '\x03' && t[2] == '\x01' && t[3] == '\x00')
        out << "using CENC=UTF32LE;" << endl;
    std::uint_least16_t inte = 0xa;
    const char *acc = reinterpret_cast<const char *>(&inte);
    if(acc[0] == '\xa')
        out << "inline constexpr bool bend = false;" << endl;
    else
        out << "inline constexpr bool bend = true;" << endl;
    out << "}" << endl;
    out.close();
    return 0;
}
