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

#include <shd/types/ranges.h>

#include <string.h>

/*
 * shd::range_t
 */
shd_error shd_range_to_pp_string(
    const shd_range_t *range,
    char* pp_string_out,
    size_t strbuffer_len
){
    SHD_SAFE_C(
        shd::range_t range_cpp = shd_range_c_to_cpp(range);
        std::string pp_string_cpp = range_cpp.to_pp_string();

        memset(pp_string_out, '\0', strbuffer_len);
        strncpy(pp_string_out, pp_string_cpp.c_str(), strbuffer_len);
    )
}

shd::range_t shd_range_c_to_cpp(
    const shd_range_t *range_c
){
    return shd::range_t(range_c->start, range_c->stop, range_c->step);
}

void shd_range_cpp_to_c(
    const shd::range_t &range_cpp,
    shd_range_t *range_c
){
    range_c->start = range_cpp.start();
    range_c->stop = range_cpp.stop();
    range_c->step = range_cpp.step();
}

/*
 * shd::meta_range_t
 */
shd_error shd_meta_range_make(
    shd_meta_range_handle* h
){
    SHD_SAFE_C(
        (*h) = new shd_meta_range_t;
    )
}

shd_error shd_meta_range_free(
    shd_meta_range_handle* h
){
    SHD_SAFE_C(
        delete (*h);
        (*h) = NULL;
    )
}

shd_error shd_meta_range_start(
    shd_meta_range_handle h,
    double *start_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *start_out = h->meta_range_cpp.start();
    )
}

shd_error shd_meta_range_stop(
    shd_meta_range_handle h,
    double *stop_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *stop_out = h->meta_range_cpp.stop();
    )
}

shd_error shd_meta_range_step(
    shd_meta_range_handle h,
    double *step_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *step_out = h->meta_range_cpp.step();
    )
}

shd_error shd_meta_range_clip(
    shd_meta_range_handle h,
    double value,
    bool clip_step,
    double *result_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *result_out = h->meta_range_cpp.clip(value, clip_step);
    )
}

shd_error shd_meta_range_size(
    shd_meta_range_handle h,
    size_t *size_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *size_out = h->meta_range_cpp.size();
    )
}

shd_error shd_meta_range_push_back(
    shd_meta_range_handle h,
    const shd_range_t *range
){
    SHD_SAFE_C_SAVE_ERROR(h,
        h->meta_range_cpp.push_back(shd_range_c_to_cpp(range));
    )
}

shd_error shd_meta_range_at(
    shd_meta_range_handle h,
    size_t num,
    shd_range_t *range_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd_range_cpp_to_c(h->meta_range_cpp.at(num),
                           range_out);
    )
}

shd_error shd_meta_range_to_pp_string(
    shd_meta_range_handle h,
    char* pp_string_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string pp_string_cpp = h->meta_range_cpp.to_pp_string();
        memset(pp_string_out, '\0', strbuffer_len);
        strncpy(pp_string_out, pp_string_cpp.c_str(), strbuffer_len);
    )
}

shd_error shd_meta_range_last_error(
    shd_meta_range_handle h,
    char* error_out,
    size_t strbuffer_len
){
    SHD_SAFE_C(
        memset(error_out, '\0', strbuffer_len);
        strncpy(error_out, h->last_error.c_str(), strbuffer_len);
    )
}
