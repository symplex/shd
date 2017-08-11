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

#include <shd/types/sensors.h>

#include <boost/lexical_cast.hpp>

#include <stdexcept>
#include <string.h>
#include <string>

shd_error shd_sensor_value_make_from_bool(
    shd_sensor_value_handle* h,
    const char* name,
    bool value,
    const char* utrue,
    const char* ufalse
){
    try{
        *h = new shd_sensor_value_t;
    }
    catch(...){
        return SHD_ERROR_UNKNOWN;
    }

    SHD_SAFE_C_SAVE_ERROR((*h),
        (*h)->sensor_value_cpp = new shd::sensor_value_t(name,
                                                         value,
                                                         utrue,
                                                         ufalse);
    )
}

shd_error shd_sensor_value_make_from_int(
    shd_sensor_value_handle* h,
    const char* name,
    int value,
    const char* unit,
    const char* formatter
){
    try{
        *h = new shd_sensor_value_t;
    }
    catch(...){
        return SHD_ERROR_UNKNOWN;
    }

    SHD_SAFE_C_SAVE_ERROR((*h),
        std::string fmt(formatter);
        if(fmt.empty()){
            (*h)->sensor_value_cpp = new shd::sensor_value_t(name,
                                                             value,
                                                             unit);
        }
        else{
            (*h)->sensor_value_cpp = new shd::sensor_value_t(name,
                                                             value,
                                                             unit,
                                                             fmt);
        }
    )
} 

shd_error shd_sensor_value_make_from_realnum(
    shd_sensor_value_handle* h,
    const char* name,
    double value,
    const char* unit,
    const char* formatter
){
    try{
        *h = new shd_sensor_value_t;
    }
    catch(...){
        return SHD_ERROR_UNKNOWN;
    }

    SHD_SAFE_C_SAVE_ERROR((*h),
        std::string fmt(formatter);
        if(fmt.empty()){
            (*h)->sensor_value_cpp = new shd::sensor_value_t(name,
                                                             value,
                                                             unit);
        }
        else{
            (*h)->sensor_value_cpp = new shd::sensor_value_t(name,
                                                             value,
                                                             unit,
                                                             fmt);
        }
    )
}

shd_error shd_sensor_value_make_from_string(
    shd_sensor_value_handle* h,
    const char* name,
    const char* value,
    const char* unit
){
    try{
        *h = new shd_sensor_value_t;
    }
    catch(...){
        return SHD_ERROR_UNKNOWN;
    }

    SHD_SAFE_C_SAVE_ERROR((*h),
        (*h)->sensor_value_cpp = new shd::sensor_value_t(name,
                                                         value,
                                                         unit);
    )
}

shd_error shd_sensor_value_free(
    shd_sensor_value_handle *h
){
    SHD_SAFE_C(
        delete (*h)->sensor_value_cpp;
        delete *h;
        *h = NULL;
    )
}

shd_error shd_sensor_value_to_bool(
    shd_sensor_value_handle h,
    bool *value_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *value_out = h->sensor_value_cpp->to_bool();
    )
}

shd_error shd_sensor_value_to_int(
    shd_sensor_value_handle h,
    int *value_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *value_out = h->sensor_value_cpp->to_int();
    )
}

shd_error shd_sensor_value_to_realnum(
    shd_sensor_value_handle h,
    double *value_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *value_out = h->sensor_value_cpp->to_real();
    )
}

shd_error shd_sensor_value_name(
    shd_sensor_value_handle h,
    char* name_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        memset(name_out, '\0', strbuffer_len);
        strncpy(name_out, h->sensor_value_cpp->name.c_str(), strbuffer_len);
    )
}

shd_error shd_sensor_value_value(
    shd_sensor_value_handle h,
    char* value_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        memset(value_out, '\0', strbuffer_len);
        strncpy(value_out, h->sensor_value_cpp->value.c_str(), strbuffer_len);
    )
}

shd_error shd_sensor_value_unit(
    shd_sensor_value_handle h,
    char* unit_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        memset(unit_out, '\0', strbuffer_len);
        strncpy(unit_out, h->sensor_value_cpp->unit.c_str(), strbuffer_len);
    )
}

shd_error shd_sensor_value_data_type(
    shd_sensor_value_handle h,
    shd_sensor_value_data_type_t *data_type_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *data_type_out = shd_sensor_value_data_type_t(h->sensor_value_cpp->type);
    )
}

shd_error shd_sensor_value_to_pp_string(
    shd_sensor_value_handle h,
    char* pp_string_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string pp_string_cpp = h->sensor_value_cpp->to_pp_string();
        memset(pp_string_out, '\0', strbuffer_len);
        strncpy(pp_string_out, pp_string_cpp.c_str(), strbuffer_len);
    )
}

shd_error shd_sensor_value_last_error(
    shd_sensor_value_handle h,
    char* error_out,
    size_t strbuffer_len
){
    SHD_SAFE_C(
        memset(error_out, '\0', strbuffer_len);
        strncpy(error_out, h->last_error.c_str(), strbuffer_len);
    )
}
