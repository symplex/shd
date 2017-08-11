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

#ifndef INCLUDED_SHD_TYPES_RANGES_H
#define INCLUDED_SHD_TYPES_RANGES_H

#include <shd/config.h>
#include <shd/error.h>

#include <stdbool.h>
#include <stdlib.h>

//! Range of floating-point values
typedef struct {
    //! First value
    double start;
    //! Last value
    double stop;
    //! Granularity
    double step;
} shd_range_t;

#ifdef __cplusplus
#include <shd/types/ranges.hpp>
#include <string>

struct shd_meta_range_t {
    shd::meta_range_t meta_range_cpp;
    std::string last_error;
};

extern "C" {
#else
struct shd_meta_range_t;
#endif

//! C-level interface for dealing with a list of ranges
/*!
 * See shd::meta_range_t for more details.
 */
typedef struct shd_meta_range_t* shd_meta_range_handle;

//! Get a string representation of the given range
SHD_API shd_error shd_range_to_pp_string(
    const shd_range_t *range,
    char* pp_string_out,
    size_t strbuffer_len
);

//! Create a meta range handle
/*!
 * NOTE: Using a shd_meta_range_handle before passing it into this function will
 * result in undefined behavior.
 */
SHD_API shd_error shd_meta_range_make(
    shd_meta_range_handle* h
);

//! Destroy a meta range handle
/*!
 * NOTE: Using a shd_meta_range_handle after passing it into this function will
 * result in a segmentation fault.
 */
SHD_API shd_error shd_meta_range_free(
    shd_meta_range_handle* h
);

//! Get the overall start value for the given meta range
SHD_API shd_error shd_meta_range_start(
    shd_meta_range_handle h,
    double *start_out
);

//! Get the overall stop value for the given meta range
SHD_API shd_error shd_meta_range_stop(
    shd_meta_range_handle h,
    double *stop_out
);

//! Get the overall step value for the given meta range
SHD_API shd_error shd_meta_range_step(
    shd_meta_range_handle h,
    double *step_out
);

//! Clip the given value to a possible value in the given range
SHD_API shd_error shd_meta_range_clip(
    shd_meta_range_handle h,
    double value,
    bool clip_step,
    double *result_out
);

//! Get the number of ranges in the given meta range
SHD_API shd_error shd_meta_range_size(
    shd_meta_range_handle h,
    size_t *size_out
);

//! Add a range to the given meta range
SHD_API shd_error shd_meta_range_push_back(
    shd_meta_range_handle h,
    const shd_range_t *range
);

//! Get the range at the given index
SHD_API shd_error shd_meta_range_at(
    shd_meta_range_handle h,
    size_t num,
    shd_range_t *range_out
);

//! Get a string representation of the given meta range
SHD_API shd_error shd_meta_range_to_pp_string(
    shd_meta_range_handle h,
    char* pp_string_out,
    size_t strbuffer_len
);

//! Get the last error recorded by the underlying meta range
SHD_API shd_error shd_meta_range_last_error(
    shd_meta_range_handle h,
    char* error_out,
    size_t strbuffer_len
);

#ifdef __cplusplus
}

SHD_API shd::range_t shd_range_c_to_cpp(
    const shd_range_t *range_c
);

SHD_API void shd_range_cpp_to_c(
    const shd::range_t &range_cpp,
    shd_range_t *range_c
);
#endif

#endif /* INCLUDED_SHD_TYPES_RANGES_H */
