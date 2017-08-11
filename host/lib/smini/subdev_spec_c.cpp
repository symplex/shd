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

#include <shd/smini/subdev_spec.h>

#include <string.h>

shd_error shd_subdev_spec_pair_free(
    shd_subdev_spec_pair_t *subdev_spec_pair
){
    SHD_SAFE_C(
        if(subdev_spec_pair->db_name){
            free(subdev_spec_pair->db_name);
            subdev_spec_pair->db_name = NULL;
        }
        if(subdev_spec_pair->sd_name){
            free(subdev_spec_pair->sd_name);
            subdev_spec_pair->sd_name = NULL;
        }
    )
}

shd_error shd_subdev_spec_pairs_equal(
    const shd_subdev_spec_pair_t* first,
    const shd_subdev_spec_pair_t* second,
    bool *result_out
){
    SHD_SAFE_C(
        *result_out = (shd_subdev_spec_pair_c_to_cpp(first) ==
                       shd_subdev_spec_pair_c_to_cpp(second));
    )
}

shd_error shd_subdev_spec_make(
    shd_subdev_spec_handle* h,
    const char* markup
){
    SHD_SAFE_C(
        (*h) = new shd_subdev_spec_t;
        std::string markup_cpp(markup);
        if(!markup_cpp.empty()){
            (*h)->subdev_spec_cpp = shd::smini::subdev_spec_t(markup_cpp);
        }
    )
}

shd_error shd_subdev_spec_free(
    shd_subdev_spec_handle* h
){
    SHD_SAFE_C(
        delete (*h);
        (*h) = NULL;
    )
}

shd_error shd_subdev_spec_size(
    shd_subdev_spec_handle h,
    size_t *size_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *size_out = h->subdev_spec_cpp.size();
    )
}

shd_error shd_subdev_spec_push_back(
    shd_subdev_spec_handle h,
    const char* markup
){
    SHD_SAFE_C_SAVE_ERROR(h,
        h->subdev_spec_cpp.push_back(shd::smini::subdev_spec_pair_t(markup));
    )
}

shd_error shd_subdev_spec_at(
    shd_subdev_spec_handle h,
    size_t num,
    shd_subdev_spec_pair_t *subdev_spec_pair_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd_subdev_spec_pair_cpp_to_c(
            h->subdev_spec_cpp.at(num),
            subdev_spec_pair_out
        );
    )
}

shd_error shd_subdev_spec_to_pp_string(
    shd_subdev_spec_handle h,
    char* pp_string_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string pp_string_cpp = h->subdev_spec_cpp.to_pp_string();
        memset(pp_string_out, '\0', strbuffer_len);
        strncpy(pp_string_out, pp_string_cpp.c_str(), strbuffer_len);
    )
}

shd_error shd_subdev_spec_to_string(
    shd_subdev_spec_handle h,
    char* string_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string string_cpp = h->subdev_spec_cpp.to_string();
        memset(string_out, '\0', strbuffer_len);
        strncpy(string_out, string_cpp.c_str(), strbuffer_len);
    )
}

shd_error shd_subdev_spec_last_error(
    shd_subdev_spec_handle h,
    char* error_out,
    size_t strbuffer_len
){
    SHD_SAFE_C(
        memset(error_out, '\0', strbuffer_len);
        strncpy(error_out, h->last_error.c_str(), strbuffer_len);
    )
}

shd::smini::subdev_spec_pair_t shd_subdev_spec_pair_c_to_cpp(
    const shd_subdev_spec_pair_t *subdev_spec_pair_c
){
    return shd::smini::subdev_spec_pair_t(subdev_spec_pair_c->db_name,
                                         subdev_spec_pair_c->sd_name);
}

void shd_subdev_spec_pair_cpp_to_c(
    const shd::smini::subdev_spec_pair_t &subdev_spec_pair_cpp,
    shd_subdev_spec_pair_t *subdev_spec_pair_c
){
    subdev_spec_pair_c->db_name = strdup(subdev_spec_pair_cpp.db_name.c_str());
    subdev_spec_pair_c->sd_name = strdup(subdev_spec_pair_cpp.sd_name.c_str());
}
