//
// Copyright 2010-2012 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <shd/version.hpp>
#include <shd/utils/static.hpp>
#include <boost/version.hpp>
#include <iostream>

#ifndef SHD_DONT_PRINT_SYSTEM_INFO
SHD_STATIC_BLOCK(print_system_info){
    std::cout
        << BOOST_PLATFORM << "; "
        << BOOST_COMPILER << "; "
        << "Boost_" << BOOST_VERSION << "; "
        << "SHD_" << shd::get_version_string()
        << std::endl << std::endl
    ;
}
#endif

std::string shd::get_version_string(void){
    return "@SHD_VERSION@";
}

std::string shd::get_abi_string(void){
    return SHD_VERSION_ABI_STRING;
}
