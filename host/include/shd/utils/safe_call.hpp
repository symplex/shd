//
// Copyright 2011 Ettus Research LLC
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

#ifndef INCLUDED_SHD_UTILS_SAFE_CALL_HPP
#define INCLUDED_SHD_UTILS_SAFE_CALL_HPP

#include <shd/config.hpp>
#include <shd/exception.hpp>
#include <shd/utils/log.hpp>

//! helper macro for safe call to produce warnings
#define _SHD_SAFE_CALL_WARNING(code, what) SHD_LOGV(rarely) << \
    SHD_THROW_SITE_INFO("Exception caught in safe-call.") + #code + " -> " + what \
;

/*!
 * A safe-call catches all exceptions thrown by code,
 * and creates a verbose warning about the exception.
 * Usage: SHD_SAFE_CALL(some_code_to_call();)
 * \param code the block of code to call safely
 */
#define SHD_SAFE_CALL(code) \
    try{code} \
    catch(const std::exception &e){ \
        _SHD_SAFE_CALL_WARNING(code, e.what()); \
    } \
    catch(...){ \
        _SHD_SAFE_CALL_WARNING(code, "unknown exception"); \
    }

#endif /* INCLUDED_SHD_UTILS_SAFE_CALL_HPP */
