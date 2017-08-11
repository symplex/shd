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

#ifndef INCLUDED_SHD_SMINI_DBOARD_EEPROM_H
#define INCLUDED_SHD_SMINI_DBOARD_EEPROM_H

#include <shd/config.h>
#include <shd/error.h>

#ifdef __cplusplus
#include <shd/smini/dboard_eeprom.hpp>
#include <string>

struct shd_dboard_eeprom_t {
    shd::smini::dboard_eeprom_t dboard_eeprom_cpp;
    std::string last_error;
};

extern "C" {
#else
struct shd_dboard_eeprom_t;
#endif

//! A C-level interface for interacting with a daughterboard EEPROM
/*!
 * See shd::smini::dboard_eeprom_t for more details.
 *
 * NOTE: Using a handle before passing it into shd_dboard_eeprom_make() will
 * result in undefined behavior.
 */
typedef struct shd_dboard_eeprom_t* shd_dboard_eeprom_handle;

//! Create handle for a SMINI daughterboard EEPROM
SHD_API shd_error shd_dboard_eeprom_make(
    shd_dboard_eeprom_handle* h
);

//! Safely destroy the given handle
/*!
 * NOTE: Using a handle after passing it into this function will result in
 * a segmentation fault.
 */
SHD_API shd_error shd_dboard_eeprom_free(
    shd_dboard_eeprom_handle* h
);

//! Get the ID associated with the given daughterboard as a string hex representation
SHD_API shd_error shd_dboard_eeprom_get_id(
    shd_dboard_eeprom_handle h,
    char* id_out,
    size_t strbuffer_len
);

//! Set the daughterboard ID using a string hex representation
SHD_API shd_error shd_dboard_eeprom_set_id(
    shd_dboard_eeprom_handle h,
    const char* id
);

//! Get the daughterboard's serial
SHD_API shd_error shd_dboard_eeprom_get_serial(
    shd_dboard_eeprom_handle h,
    char* serial_out,
    size_t strbuffer_len
);

//! Set the daughterboard's serial
SHD_API shd_error shd_dboard_eeprom_set_serial(
    shd_dboard_eeprom_handle h,
    const char* serial
);

//! Get the daughterboard's revision (not always present)
SHD_API shd_error shd_dboard_eeprom_get_revision(
    shd_dboard_eeprom_handle h,
    int* revision_out
);

//! Set the daughterboard's revision
SHD_API shd_error shd_dboard_eeprom_set_revision(
    shd_dboard_eeprom_handle h,
    int revision
);

//! Get the last error reported by the handle
SHD_API shd_error shd_dboard_eeprom_last_error(
    shd_dboard_eeprom_handle h,
    char* error_out,
    size_t strbuffer_len
);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_SHD_SMINI_DBOARD_EEPROM_H */
