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

#ifndef INCLUDED_SHD_SMINI_CLOCK_H
#define INCLUDED_SHD_SMINI_CLOCK_H

#include <shd/config.h>
#include <shd/error.h>
#include <shd/types/sensors.h>
#include <shd/types/string_vector.h>

#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/****************************************************************************
 * Public Datatypes for SMINI clock
 ***************************************************************************/
struct shd_smini_clock;

//! A C-level interface for interacting with an Ettus Research clock device
/*!
 * See shd::smini_clock::multi_smini_clock for more details.
 *
 * NOTE: Attempting to use a handle before passing it into shd_smini_clock_make()
 * will result in undefined behavior.
 */
typedef struct shd_smini_clock* shd_smini_clock_handle;

/****************************************************************************
 * Make / Free API calls
 ***************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

//! Find all connected clock devices.
/*!
 * See shd::device::find() for more details.
 */
SHD_API shd_error shd_smini_clock_find(
    const char* args,
    shd_string_vector_t *devices_out
);

//! Create a clock handle.
/*!
 * \param h The handle
 * \param args Device args (e.g. "addr=192.168.10.3")
 */
SHD_API shd_error shd_smini_clock_make(
    shd_smini_clock_handle *h,
    const char *args
);

//! Safely destroy the clock object underlying the handle.
/*!
 * Note: After calling this, usage of h may cause segmentation faults.
 * However, multiple calling of shd_smini_free() is safe.
 */
SHD_API shd_error shd_smini_clock_free(
    shd_smini_clock_handle *h
);

//! Get last error
SHD_API shd_error shd_smini_clock_last_error(
    shd_smini_clock_handle h,
    char* error_out,
    size_t strbuffer_len
);

//! Get board information in a nice output
SHD_API shd_error shd_smini_clock_get_pp_string(
    shd_smini_clock_handle h,
    char* pp_string_out,
    size_t strbuffer_len
);

//! Get number of boards
SHD_API shd_error shd_smini_clock_get_num_boards(
    shd_smini_clock_handle h,
    size_t *num_boards_out
);

//! Get time
SHD_API shd_error shd_smini_clock_get_time(
    shd_smini_clock_handle h,
    size_t board,
    uint32_t *clock_time_out
);

//! Get sensor
SHD_API shd_error shd_smini_clock_get_sensor(
    shd_smini_clock_handle h,
    const char* name,
    size_t board,
    shd_sensor_value_handle *sensor_value_out
);

//! Get sensor names
SHD_API shd_error shd_smini_clock_get_sensor_names(
    shd_smini_clock_handle h,
    size_t board,
    shd_string_vector_handle *sensor_names_out
);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_SHD_SMINI_CLOCK_H */
