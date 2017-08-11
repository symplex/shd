//
// Copyright 2015 Ettus Research LLC
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

#include <shd/error.h>
#include <shd/exception.hpp>
#include <shd/utils/static.hpp>

#include <boost/thread/mutex.hpp>

#include <cstring>

#define MAP_TO_ERROR(exception_type, error_type) \
    if (dynamic_cast<const shd::exception_type*>(e)) return error_type;

shd_error error_from_shd_exception(const shd::exception* e){
    MAP_TO_ERROR(index_error,           SHD_ERROR_INDEX)
    MAP_TO_ERROR(key_error,             SHD_ERROR_KEY)
    MAP_TO_ERROR(not_implemented_error, SHD_ERROR_NOT_IMPLEMENTED)
    MAP_TO_ERROR(usb_error,             SHD_ERROR_USB)
    MAP_TO_ERROR(io_error,              SHD_ERROR_IO)
    MAP_TO_ERROR(os_error,              SHD_ERROR_OS)
    MAP_TO_ERROR(assertion_error,       SHD_ERROR_ASSERTION)
    MAP_TO_ERROR(lookup_error,          SHD_ERROR_LOOKUP)
    MAP_TO_ERROR(type_error,            SHD_ERROR_TYPE)
    MAP_TO_ERROR(value_error,           SHD_ERROR_VALUE)
    MAP_TO_ERROR(runtime_error,         SHD_ERROR_RUNTIME)
    MAP_TO_ERROR(environment_error,     SHD_ERROR_ENVIRONMENT)
    MAP_TO_ERROR(system_error,          SHD_ERROR_SYSTEM)

    return SHD_ERROR_EXCEPT;
}

// Store the error string in a single place in library
SHD_SINGLETON_FCN(std::string, _c_global_error_string)

static boost::mutex _error_c_mutex;

const std::string& get_c_global_error_string(){
    boost::mutex::scoped_lock lock(_error_c_mutex);
    return _c_global_error_string();
}

void set_c_global_error_string(
    const std::string &msg
){
    boost::mutex::scoped_lock lock(_error_c_mutex);
    _c_global_error_string() = msg;
}

shd_error shd_get_last_error(
    char* error_out,
    size_t strbuffer_len
){
    try{
        memset(error_out, '\0', strbuffer_len);
        strncpy(error_out, _c_global_error_string().c_str(), strbuffer_len);
    }
    catch(...){
        return SHD_ERROR_UNKNOWN;
    }
    return SHD_ERROR_NONE;
}
