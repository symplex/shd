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

#include <shd/smini/mboard_eeprom.h>

#include <shd/exception.hpp>

#include <string.h>

shd_error shd_mboard_eeprom_make(
    shd_mboard_eeprom_handle* h
){
    SHD_SAFE_C(
        *h = new shd_mboard_eeprom_t;
    )
}

shd_error shd_mboard_eeprom_free(
    shd_mboard_eeprom_handle* h
){
    SHD_SAFE_C(
        delete *h;
        *h = NULL;
    )
}

shd_error shd_mboard_eeprom_get_value(
    shd_mboard_eeprom_handle h,
    const char* key,
    char* value_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string value_cpp = h->mboard_eeprom_cpp.get(key);
        strncpy(value_out, value_cpp.c_str(), strbuffer_len);
    )
}

shd_error shd_mboard_eeprom_set_value(
    shd_mboard_eeprom_handle h,
    const char* key,
    const char* value
){
    SHD_SAFE_C_SAVE_ERROR(h,
        h->mboard_eeprom_cpp[key] = value;
    )
}

shd_error shd_mboard_eeprom_last_error(
    shd_mboard_eeprom_handle h,
    char* error_out,
    size_t strbuffer_len
){
    SHD_SAFE_C(
        memset(error_out, '\0', strbuffer_len);
        strncpy(error_out, h->last_error.c_str(), strbuffer_len);
    )
}
