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

#ifndef INCLUDED_SHD_SMINI_SUBDEV_SPEC_H
#define INCLUDED_SHD_SMINI_SUBDEV_SPEC_H

#include <shd/config.h>
#include <shd/error.h>

#include <stdbool.h>

//! Subdevice specification
typedef struct {
    // Daughterboard slot name
    char* db_name;
    //! Subdevice name
    char* sd_name;
} shd_subdev_spec_pair_t;

#ifdef __cplusplus
#include <shd/smini/subdev_spec.hpp>
#include <string>

struct shd_subdev_spec_t {
    shd::smini::subdev_spec_t subdev_spec_cpp;
    std::string last_error;
};

extern "C" {
#else
struct shd_subdev_spec_t;
#endif

//! A C-level interface for working with a list of subdevice specifications
/*!
 * See shd::smini::subdev_spec_t for more details.
 *
 * NOTE: Using a handle before passing it into shd_subdev_spec_make() will result in
 * undefined behavior.
 */
typedef struct shd_subdev_spec_t* shd_subdev_spec_handle;

//! Safely destroy any memory created in the generation of a shd_subdev_spec_pair_t
SHD_API shd_error shd_subdev_spec_pair_free(
    shd_subdev_spec_pair_t *subdev_spec_pair
);

//! Check to see if two subdevice specifications are equal
SHD_API shd_error shd_subdev_spec_pairs_equal(
    const shd_subdev_spec_pair_t* first,
    const shd_subdev_spec_pair_t* second,
    bool *result_out
);

//! Create a handle for a list of subdevice specifications
SHD_API shd_error shd_subdev_spec_make(
    shd_subdev_spec_handle* h,
    const char* markup
);

//! Safely destroy a subdevice specification handle
/*!
 * NOTE: Using a handle after passing it into this function will result in
 * a segmentation fault.
 */
SHD_API shd_error shd_subdev_spec_free(
    shd_subdev_spec_handle* h
);

//! Check how many subdevice specifications are in this list
SHD_API shd_error shd_subdev_spec_size(
    shd_subdev_spec_handle h,
    size_t *size_out
);

//! Add a subdevice specification to this list
SHD_API shd_error shd_subdev_spec_push_back(
    shd_subdev_spec_handle h,
    const char* markup
);

//! Get the subdevice specification at the given index
SHD_API shd_error shd_subdev_spec_at(
    shd_subdev_spec_handle h,
    size_t num,
    shd_subdev_spec_pair_t *subdev_spec_pair_out
);

//! Get a string representation of the given list
SHD_API shd_error shd_subdev_spec_to_pp_string(
    shd_subdev_spec_handle h,
    char* pp_string_out,
    size_t strbuffer_len
);

//! Get a markup string representation of the given list
SHD_API shd_error shd_subdev_spec_to_string(
    shd_subdev_spec_handle h,
    char* string_out,
    size_t strbuffer_len
);

//! Get the last error recorded by the given handle
SHD_API shd_error shd_subdev_spec_last_error(
    shd_subdev_spec_handle h,
    char* error_out,
    size_t strbuffer_len
);

#ifdef __cplusplus
}

SHD_API shd::smini::subdev_spec_pair_t shd_subdev_spec_pair_c_to_cpp(
    const shd_subdev_spec_pair_t* subdev_spec_pair_c
);

SHD_API void shd_subdev_spec_pair_cpp_to_c(
    const shd::smini::subdev_spec_pair_t &subdev_spec_pair_cpp,
    shd_subdev_spec_pair_t *subdev_spec_pair_c
);
#endif

#endif /* INCLUDED_SHD_SMINI_SUBDEV_SPEC_H */
