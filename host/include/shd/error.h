/*
 * Copyright 2015 Ettus Research LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDED_SHD_ERROR_H
#define INCLUDED_SHD_ERROR_H

#include <stdlib.h>

//! SHD error codes
/*!
 * Each error code corresponds to a specific shd::exception, with
 * extra codes corresponding to a boost::exception, std::exception,
 * and a catch-all for everything else. When an internal C++ function
 * throws an exception, SHD converts it to one of these error codes
 * to return on the C level.
 */
typedef enum {

    //! No error thrown.
    SHD_ERROR_NONE = 0,
    //! Invalid device arguments.
    SHD_ERROR_INVALID_DEVICE = 1,

    //! See shd::index_error.
    SHD_ERROR_INDEX = 10,
    //! See shd::key_error.
    SHD_ERROR_KEY = 11,

    //! See shd::not_implemented_error.
    SHD_ERROR_NOT_IMPLEMENTED = 20,
    //! See shd::usb_error.
    SHD_ERROR_USB = 21,

    //! See shd::io_error.
    SHD_ERROR_IO = 30,
    //! See shd::os_error.
    SHD_ERROR_OS = 31,

    //! See shd::assertion_error.
    SHD_ERROR_ASSERTION = 40,
    //! See shd::lookup_error.
    SHD_ERROR_LOOKUP = 41,
    //! See shd::type_error.
    SHD_ERROR_TYPE = 42,
    //! See shd::value_error.
    SHD_ERROR_VALUE = 43,
    //! See shd::runtime_error.
    SHD_ERROR_RUNTIME = 44,
    //! See shd::environment_error.
    SHD_ERROR_ENVIRONMENT = 45,
    //! See shd::system_error.
    SHD_ERROR_SYSTEM = 46,
    //! See shd::exception.
    SHD_ERROR_EXCEPT = 47,

    //! A boost::exception was thrown.
    SHD_ERROR_BOOSTEXCEPT = 60,

    //! A std::exception was thrown.
    SHD_ERROR_STDEXCEPT = 70,

    //! An unknown error was thrown.
    SHD_ERROR_UNKNOWN = 100
} shd_error;

#ifdef __cplusplus
#include <shd/config.hpp>
#include <shd/exception.hpp>

#include <boost/exception/diagnostic_information.hpp>

#include <string>

SHD_API shd_error error_from_shd_exception(const shd::exception* e);

SHD_API const std::string& get_c_global_error_string();

SHD_API void set_c_global_error_string(const std::string &msg);

/*!
 * This macro runs the given C++ code, and if there are any exceptions
 * thrown, they are caught and converted to the corresponding SHD error
 * code.
 */
#define SHD_SAFE_C(...) \
    try{ __VA_ARGS__ } \
    catch (const shd::exception &e) { \
        set_c_global_error_string(e.what()); \
        return error_from_shd_exception(&e); \
    } \
    catch (const boost::exception &e) { \
        set_c_global_error_string(boost::diagnostic_information(e)); \
        return SHD_ERROR_BOOSTEXCEPT; \
    } \
    catch (const std::exception &e) { \
        set_c_global_error_string(e.what()); \
        return SHD_ERROR_STDEXCEPT; \
    } \
    catch (...) { \
        set_c_global_error_string("Unrecognized exception caught."); \
        return SHD_ERROR_UNKNOWN; \
    } \
    set_c_global_error_string("None"); \
    return SHD_ERROR_NONE;

/*!
 * This macro runs the given C++ code, and if there are any exceptions
 * thrown, they are caught and converted to the corresponding SHD error
 * code. The error message is also saved into the given handle.
 */
#define SHD_SAFE_C_SAVE_ERROR(h, ...) \
    h->last_error.clear(); \
    try{ __VA_ARGS__ } \
    catch (const shd::exception &e) { \
        set_c_global_error_string(e.what()); \
        h->last_error = e.what(); \
        return error_from_shd_exception(&e); \
    } \
    catch (const boost::exception &e) { \
        set_c_global_error_string(boost::diagnostic_information(e)); \
        h->last_error = boost::diagnostic_information(e); \
        return SHD_ERROR_BOOSTEXCEPT; \
    } \
    catch (const std::exception &e) { \
        set_c_global_error_string(e.what()); \
        h->last_error = e.what(); \
        return SHD_ERROR_STDEXCEPT; \
    } \
    catch (...) { \
        set_c_global_error_string("Unrecognized exception caught."); \
        h->last_error = "Unrecognized exception caught."; \
        return SHD_ERROR_UNKNOWN; \
    } \
    h->last_error = "None"; \
    set_c_global_error_string("None"); \
    return SHD_ERROR_NONE;

extern "C" {
#endif

//! Return the last error string reported by SHD
/*!
 * Functions that do not take in SHD structs/handles will place any error
 * strings into a buffer that can be queried with this function. Functions that
 * do take in SHD structs/handles will place their error strings in both locations.
 */
SHD_API shd_error shd_get_last_error(
    char* error_out,
    size_t strbuffer_len
);
#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_SHD_ERROR_H */
