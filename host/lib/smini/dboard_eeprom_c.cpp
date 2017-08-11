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

#include <shd/smini/dboard_eeprom.h>
#include <shd/error.h>

#include <boost/lexical_cast.hpp>

#include <string.h>

shd_error shd_dboard_eeprom_make(
    shd_dboard_eeprom_handle* h
){
    SHD_SAFE_C(
        *h = new shd_dboard_eeprom_t;
    )
}

shd_error shd_dboard_eeprom_free(
    shd_dboard_eeprom_handle* h
){
    SHD_SAFE_C(
        delete *h;
        *h = NULL;
    )
}

shd_error shd_dboard_eeprom_get_id(
    shd_dboard_eeprom_handle h,
    char* id_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string dboard_id_cpp = h->dboard_eeprom_cpp.id.to_string();
        strncpy(id_out, dboard_id_cpp.c_str(), strbuffer_len);
    )
}

shd_error shd_dboard_eeprom_set_id(
    shd_dboard_eeprom_handle h,
    const char* id
){
    SHD_SAFE_C_SAVE_ERROR(h,
        h->dboard_eeprom_cpp.id = shd::smini::dboard_id_t::from_string(id);
    )
}

shd_error shd_dboard_eeprom_get_serial(
    shd_dboard_eeprom_handle h,
    char* id_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string dboard_serial_cpp = h->dboard_eeprom_cpp.serial;
        strncpy(id_out, dboard_serial_cpp.c_str(), strbuffer_len);
    )
}

shd_error shd_dboard_eeprom_set_serial(
    shd_dboard_eeprom_handle h,
    const char* serial
){
    SHD_SAFE_C_SAVE_ERROR(h,
        h->dboard_eeprom_cpp.serial = serial;
    )
}

shd_error shd_dboard_eeprom_get_revision(
    shd_dboard_eeprom_handle h,
    int* revision_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *revision_out = boost::lexical_cast<int>(h->dboard_eeprom_cpp.revision);
    )
}

shd_error shd_dboard_eeprom_set_revision(
    shd_dboard_eeprom_handle h,
    int revision
){
    SHD_SAFE_C_SAVE_ERROR(h,
        h->dboard_eeprom_cpp.revision = boost::lexical_cast<std::string>(revision);
    )
}

shd_error shd_dboard_eeprom_last_error(
    shd_dboard_eeprom_handle h,
    char* error_out,
    size_t strbuffer_len
){
    SHD_SAFE_C(
        memset(error_out, '\0', strbuffer_len);
        strncpy(error_out, h->last_error.c_str(), strbuffer_len);
    )
}
