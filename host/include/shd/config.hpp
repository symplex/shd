//
// Copyright 2010-2011,2014-2016 Ettus Research LLC
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

#ifndef INCLUDED_SHD_CONFIG_HPP
#define INCLUDED_SHD_CONFIG_HPP

#include <boost/config.hpp>

#ifdef BOOST_MSVC
// suppress warnings
//# pragma warning(push)
//# pragma warning(disable: 4511) // copy constructor can't not be generated
//# pragma warning(disable: 4512) // assignment operator can't not be generated
//# pragma warning(disable: 4100) // unreferenced formal parameter
//# pragma warning(disable: 4996) // <symbol> was declared deprecated
# pragma warning(disable: 4355) // 'this' : used in base member initializer list
//# pragma warning(disable: 4706) // assignment within conditional expression
# pragma warning(disable: 4251) // class 'A<T>' needs to have dll-interface to be used by clients of class 'B'
//# pragma warning(disable: 4127) // conditional expression is constant
//# pragma warning(disable: 4290) // C++ exception specification ignored except to ...
//# pragma warning(disable: 4180) // qualifier applied to function type has no meaning; ignored
# pragma warning(disable: 4275) // non dll-interface class ... used as base for dll-interface class ...
//# pragma warning(disable: 4267) // 'var' : conversion from 'size_t' to 'type', possible loss of data
//# pragma warning(disable: 4511) // 'class' : copy constructor could not be generated
# pragma warning(disable: 4250) // 'class' : inherits 'method' via dominance
# pragma warning(disable: 4200) // nonstandard extension used : zero-sized array in struct/union

// define logical operators
#include <ciso646>

// define ssize_t
#include <cstddef>
typedef ptrdiff_t ssize_t;

#endif //BOOST_MSVC

//define cross platform attribute macros
#if defined(BOOST_MSVC)
    #define SHD_EXPORT         __declspec(dllexport)
    #define SHD_IMPORT         __declspec(dllimport)
    #define SHD_INLINE         __forceinline
    #define SHD_FORCE_INLINE   __forceinline
    #define SHD_DEPRECATED     __declspec(deprecated)
    #define SHD_ALIGNED(x)     __declspec(align(x))
    #define SHD_UNUSED(x)      x
#elif defined(__MINGW32__)
    #define SHD_EXPORT         __declspec(dllexport)
    #define SHD_IMPORT         __declspec(dllimport)
    #define SHD_INLINE         inline
    #define SHD_FORCE_INLINE   inline
    #define SHD_DEPRECATED     __declspec(deprecated)
    #define SHD_ALIGNED(x)     __declspec(align(x))
    #define SHD_UNUSED(x)      x __attribute__((unused))
#elif defined(__GNUG__) && __GNUG__ >= 4
    #define SHD_EXPORT         __attribute__((visibility("default")))
    #define SHD_IMPORT         __attribute__((visibility("default")))
    #define SHD_INLINE         inline __attribute__((always_inline))
    #define SHD_FORCE_INLINE   inline __attribute__((always_inline))
    #define SHD_DEPRECATED     __attribute__((deprecated))
    #define SHD_ALIGNED(x)     __attribute__((aligned(x)))
    #define SHD_UNUSED(x)      x __attribute__((unused))
#elif defined(__clang__)
    #define SHD_EXPORT         __attribute__((visibility("default")))
    #define SHD_IMPORT         __attribute__((visibility("default")))
    #define SHD_INLINE         inline __attribute__((always_inline))
    #define SHD_FORCE_INLINE   inline __attribute__((always_inline))
    #define SHD_DEPRECATED     __attribute__((deprecated))
    #define SHD_ALIGNED(x)     __attribute__((aligned(x)))
    #define SHD_UNUSED(x)      x __attribute__((unused))
#else
    #define SHD_EXPORT
    #define SHD_IMPORT
    #define SHD_INLINE         inline
    #define SHD_FORCE_INLINE   inline
    #define SHD_DEPRECATED
    #define SHD_ALIGNED(x)
    #define SHD_UNUSED(x)      x
#endif

// Define API declaration macro
#ifdef SHD_DLL_EXPORTS
    #define SHD_API SHD_EXPORT
#else
    #define SHD_API SHD_IMPORT
#endif // SHD_DLL_EXPORTS
#ifdef SHD_RFNOC_ENABLED
    #define SHD_RFNOC_API SHD_API
#else
    #define SHD_RFNOC_API
#endif // SHD_RFNOC_ENABLED


// Platform defines for conditional parts of headers:
// Taken from boost/config/select_platform_config.hpp,
// however, we define macros, not strings for platforms.
#if (defined(linux) || defined(__linux) || defined(__linux__) || defined(__GLIBC__)) && !defined(_CRAYC) && !defined(__FreeBSD_kernel__) && !defined(__GNU__)
    #define SHD_PLATFORM_LINUX
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    #define SHD_PLATFORM_WIN32
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
    #define SHD_PLATFORM_MACOS
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__FreeBSD_kernel__)
    #define SHD_PLATFORM_BSD
#endif

// Define 'stringize' preprocessor macros. The stringize macro, XSTR, takes
// variable arguments so that it can deal with strings that contain commas.
// There are two different versions because MSVC handles this syntax a bit
// differently than other compilers.
#if defined(BOOST_MSVC)
    #define XSTR(x,...) #x
#else
    #define XSTR(x...) #x
#endif

#define STR(x) XSTR(x)

#endif /* INCLUDED_SHD_CONFIG_HPP */
