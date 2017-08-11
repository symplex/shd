//
// Copyright 2010-2011 Ettus Research LLC
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

#ifndef INCLUDED_SHD_UTILS_BYTESWAP_HPP
#define INCLUDED_SHD_UTILS_BYTESWAP_HPP

#include <shd/config.hpp>
#include <stdint.h>

/*! \file byteswap.hpp
 *
 * Provide fast byteswaping routines for 16, 32, and 64 bit integers,
 * by using the system's native routines/intrinsics when available.
 */
namespace shd{

    //! perform a byteswap on a 16 bit integer
    uint16_t byteswap(uint16_t);

    //! perform a byteswap on a 32 bit integer
    uint32_t byteswap(uint32_t);

    //! perform a byteswap on a 64 bit integer
    uint64_t byteswap(uint64_t);

    //! network to host: short, long, or long-long
    template<typename T> T ntohx(T);

    //! host to network: short, long, or long-long
    template<typename T> T htonx(T);

    //! worknet to host: short, long, or long-long
    //
    // The argument is assumed to be little-endian (i.e, the inverse
    // of typical network endianness).
    template<typename T> T wtohx(T);

    //! host to worknet: short, long, or long-long
    //
    // The return value is little-endian (i.e, the inverse
    // of typical network endianness).
    template<typename T> T htowx(T);

} //namespace shd

#include <shd/utils/byteswap.ipp>

#endif /* INCLUDED_SHD_UTILS_BYTESWAP_HPP */
