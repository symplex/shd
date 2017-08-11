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

#ifndef INCLUDED_SHD_UTILS_BYTESWAP_IPP
#define INCLUDED_SHD_UTILS_BYTESWAP_IPP

/***********************************************************************
 * Platform-specific implementation details for byteswap below:
 **********************************************************************/
#if defined(BOOST_MSVC) //http://msdn.microsoft.com/en-us/library/a3140177%28VS.80%29.aspx
    #include <cstdlib>

    SHD_INLINE uint16_t shd::byteswap(uint16_t x){
        return _byteswap_ushort(x);
    }

    SHD_INLINE uint32_t shd::byteswap(uint32_t x){
        return _byteswap_ulong(x);
    }

    SHD_INLINE uint64_t shd::byteswap(uint64_t x){
        return _byteswap_uint64(x);
    }

#elif defined(__GNUC__) && __GNUC__ >= 4 && __GNUC_MINOR__ >= 3

    SHD_INLINE uint16_t shd::byteswap(uint16_t x){
        return (x>>8) | (x<<8); //DNE return __builtin_bswap16(x);
    }

    SHD_INLINE uint32_t shd::byteswap(uint32_t x){
        return __builtin_bswap32(x);
    }

    SHD_INLINE uint64_t shd::byteswap(uint64_t x){
        return __builtin_bswap64(x);
    }

#elif defined(SHD_PLATFORM_MACOS)
    #include <libkern/OSByteOrder.h>

    SHD_INLINE uint16_t shd::byteswap(uint16_t x){
        return OSSwapInt16(x);
    }

    SHD_INLINE uint32_t shd::byteswap(uint32_t x){
        return OSSwapInt32(x);
    }

    SHD_INLINE uint64_t shd::byteswap(uint64_t x){
        return OSSwapInt64(x);
    }

#elif defined(SHD_PLATFORM_LINUX)
    #include <byteswap.h>

    SHD_INLINE uint16_t shd::byteswap(uint16_t x){
        return bswap_16(x);
    }

    SHD_INLINE uint32_t shd::byteswap(uint32_t x){
        return bswap_32(x);
    }

    SHD_INLINE uint64_t shd::byteswap(uint64_t x){
        return bswap_64(x);
    }

#else //http://www.koders.com/c/fidB93B34CD44F0ECF724F1A4EAE3854BA2FE692F59.aspx

    SHD_INLINE uint16_t shd::byteswap(uint16_t x){
        return (x>>8) | (x<<8);
    }

    SHD_INLINE uint32_t shd::byteswap(uint32_t x){
        return (uint32_t(shd::byteswap(uint16_t(x&0xfffful)))<<16) | (shd::byteswap(uint16_t(x>>16)));
    }

    SHD_INLINE uint64_t shd::byteswap(uint64_t x){
        return (uint64_t(shd::byteswap(uint32_t(x&0xffffffffull)))<<32) | (shd::byteswap(uint32_t(x>>32)));
    }

#endif

/***********************************************************************
 * Define the templated network to/from host conversions
 **********************************************************************/
#include <boost/detail/endian.hpp>

namespace shd {

template<typename T> SHD_INLINE T ntohx(T num){
    #ifdef BOOST_BIG_ENDIAN
        return num;
    #else
        return shd::byteswap(num);
    #endif
}

template<typename T> SHD_INLINE T htonx(T num){
    #ifdef BOOST_BIG_ENDIAN
        return num;
    #else
        return shd::byteswap(num);
    #endif
}

template<typename T> SHD_INLINE T wtohx(T num){
    #ifdef BOOST_BIG_ENDIAN
        return shd::byteswap(num);
    #else
        return num;
    #endif
}

template<typename T> SHD_INLINE T htowx(T num){
    #ifdef BOOST_BIG_ENDIAN
        return shd::byteswap(num);
    #else
        return num;
    #endif
}

} /* namespace shd */

#endif /* INCLUDED_SHD_UTILS_BYTESWAP_IPP */
