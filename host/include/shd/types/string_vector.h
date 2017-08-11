/*
 * Copyright 2015 Ettus Research
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

#ifndef INCLUDED_SHD_TYPES_STRING_VECTOR_H
#define INCLUDED_SHD_TYPES_STRING_VECTOR_H

#include <shd/config.h>
#include <shd/error.h>

#include <stdlib.h>

#ifdef __cplusplus
#include <string>
#include <vector>

struct shd_string_vector_t {
    std::vector<std::string> string_vector_cpp;
    std::string last_error;
};

extern "C" {
#else
//! C-level read-only interface for interacting with a string vector
struct shd_string_vector_t;
#endif

typedef struct shd_string_vector_t shd_string_vector_t;

typedef shd_string_vector_t* shd_string_vector_handle;

//! Instantiate a string_vector handle.
SHD_API shd_error shd_string_vector_make(
    shd_string_vector_handle *h
);

//! Safely destroy a string_vector handle.
SHD_API shd_error shd_string_vector_free(
    shd_string_vector_handle *h
);

//! Add a string to the list
SHD_API shd_error shd_string_vector_push_back(
    shd_string_vector_handle *h,
    const char* value
);

//! Get the string at the given index
SHD_API shd_error shd_string_vector_at(
    shd_string_vector_handle h,
    size_t index,
    char* value_out,
    size_t strbuffer_len
);

//! Get the number of strings in this list
SHD_API shd_error shd_string_vector_size(
    shd_string_vector_handle h,
    size_t *size_out
);

//! Get the last error reported by the underlying object
SHD_API shd_error shd_string_vector_last_error(
    shd_string_vector_handle h,
    char* error_out,
    size_t strbuffer_len
);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_SHD_TYPES_STRING_VECTOR_H */
