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

/* C-Interface for multi_smini_clock */

#include <shd/utils/static.hpp>
#include <shd/smini_clock/multi_smini_clock.hpp>

#include <shd/smini_clock/smini_clock.h>

#include <boost/foreach.hpp>
#include <boost/thread/mutex.hpp>

#include <string.h>
#include <map>

/****************************************************************************
 * Registry / Pointer Management
 ***************************************************************************/
/* Public structs */
struct shd_smini_clock {
    size_t smini_clock_index;
    std::string last_error;
};

/* Not public: We use this for our internal registry */
struct smini_clock_ptr {
    shd::smini_clock::multi_smini_clock::sptr ptr;
    static size_t smini_clock_counter;
};
size_t smini_clock_ptr::smini_clock_counter = 0;
typedef struct smini_clock_ptr smini_clock_ptr;
/* Prefer map, because the list can be discontiguous */
typedef std::map<size_t, smini_clock_ptr> smini_clock_ptrs;

SHD_SINGLETON_FCN(smini_clock_ptrs, get_smini_clock_ptrs);
/* Shortcut for accessing the underlying SMINI clock sptr from a shd_smini_clock_handle* */
#define SMINI_CLOCK(h_ptr) (get_smini_clock_ptrs()[h_ptr->smini_clock_index].ptr)

/****************************************************************************
 * Generate / Destroy API calls
 ***************************************************************************/
static boost::mutex _smini_clock_find_mutex;
shd_error shd_smini_clock_find(
    const char* args,
    shd_string_vector_t *devices_out
){
    SHD_SAFE_C(
        boost::mutex::scoped_lock lock(_smini_clock_find_mutex);

        shd::device_addrs_t devs = shd::device::find(std::string(args), shd::device::CLOCK);
        devices_out->string_vector_cpp.clear();
        BOOST_FOREACH(const shd::device_addr_t &dev, devs){
            devices_out->string_vector_cpp.push_back(dev.to_string());
        }
    )
}

static boost::mutex _smini_clock_make_mutex;
shd_error shd_smini_clock_make(
    shd_smini_clock_handle *h,
    const char *args
){
    SHD_SAFE_C(
        boost::mutex::scoped_lock lock(_smini_clock_make_mutex);

        size_t smini_clock_count = smini_clock_ptr::smini_clock_counter;
        smini_clock_ptr::smini_clock_counter++;

        // Initialize SMINI Clock
        shd::device_addr_t device_addr(args);
        smini_clock_ptr P;
        P.ptr = shd::smini_clock::multi_smini_clock::make(device_addr);

        // Dump into registry
        get_smini_clock_ptrs()[smini_clock_count] = P;

        // Update handle
        (*h) = new shd_smini_clock;
        (*h)->smini_clock_index = smini_clock_count;
    )
}

static boost::mutex _smini_clock_free_mutex;
shd_error shd_smini_clock_free(
    shd_smini_clock_handle *h
){
    SHD_SAFE_C(
        boost::mutex::scoped_lock lock(_smini_clock_free_mutex);

        if(!get_smini_clock_ptrs().count((*h)->smini_clock_index)){
            return SHD_ERROR_INVALID_DEVICE;
        }

        get_smini_clock_ptrs().erase((*h)->smini_clock_index);
        delete *h;
        *h = NULL;
    )
}

shd_error shd_smini_clock_last_error(
    shd_smini_clock_handle h,
    char* error_out,
    size_t strbuffer_len
){
    SHD_SAFE_C(
        memset(error_out, '\0', strbuffer_len);
        strncpy(error_out, h->last_error.c_str(), strbuffer_len);
    )
}

shd_error shd_smini_clock_get_pp_string(
    shd_smini_clock_handle h,
    char* pp_string_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        memset(pp_string_out, '\0', strbuffer_len);
        strncpy(pp_string_out, SMINI_CLOCK(h)->get_pp_string().c_str(), strbuffer_len);
    )
}

shd_error shd_smini_clock_get_num_boards(
    shd_smini_clock_handle h,
    size_t *num_boards_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *num_boards_out = SMINI_CLOCK(h)->get_num_boards();
    )
}

shd_error shd_smini_clock_get_time(
    shd_smini_clock_handle h,
    size_t board,
    uint32_t *clock_time_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *clock_time_out = SMINI_CLOCK(h)->get_time(board);
    )
}

shd_error shd_smini_clock_get_sensor(
    shd_smini_clock_handle h,
    const char* name,
    size_t board,
    shd_sensor_value_handle *sensor_value_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        delete (*sensor_value_out)->sensor_value_cpp;
        (*sensor_value_out)->sensor_value_cpp = new shd::sensor_value_t(SMINI_CLOCK(h)->get_sensor(name, board));
    )
}

shd_error shd_smini_clock_get_sensor_names(
    shd_smini_clock_handle h,
    size_t board,
    shd_string_vector_handle *sensor_names_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        (*sensor_names_out)->string_vector_cpp = SMINI_CLOCK(h)->get_sensor_names(board);
    )
}
