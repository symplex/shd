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

#include <shd/types/metadata.h>

#include <shd/types/time_spec.hpp>

#include <string.h>

/*
 * RX metadata
 */

shd_error shd_rx_metadata_make(
    shd_rx_metadata_handle* handle
){
    SHD_SAFE_C(
        *handle = new shd_rx_metadata_t;
    )
}

shd_error shd_rx_metadata_free(
    shd_rx_metadata_handle* handle
){
    SHD_SAFE_C(
        delete *handle;
        *handle = NULL;
    )
}

shd_error shd_rx_metadata_has_time_spec(
    shd_rx_metadata_handle h,
    bool *result_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
         *result_out = h->rx_metadata_cpp.has_time_spec;
    )
}

shd_error shd_rx_metadata_time_spec(
    shd_rx_metadata_handle h,
    time_t *full_secs_out,
    double *frac_secs_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::time_spec_t time_spec_cpp = h->rx_metadata_cpp.time_spec;
        *full_secs_out = time_spec_cpp.get_full_secs();
        *frac_secs_out = time_spec_cpp.get_frac_secs();
    )
}

shd_error shd_rx_metadata_more_fragments(
    shd_rx_metadata_handle h,
    bool *result_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *result_out = h->rx_metadata_cpp.more_fragments;
    )
}

shd_error shd_rx_metadata_fragment_offset(
    shd_rx_metadata_handle h,
    size_t *fragment_offset_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *fragment_offset_out = h->rx_metadata_cpp.fragment_offset;
    )
}

shd_error shd_rx_metadata_start_of_burst(
    shd_rx_metadata_handle h,
    bool *result_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *result_out = h->rx_metadata_cpp.start_of_burst;
    )
}

shd_error shd_rx_metadata_end_of_burst(
    shd_rx_metadata_handle h,
    bool *result_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *result_out = h->rx_metadata_cpp.end_of_burst;
    )
}

shd_error shd_rx_metadata_out_of_sequence(
    shd_rx_metadata_handle h,
    bool *result_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *result_out = h->rx_metadata_cpp.out_of_sequence;
    )
}

shd_error shd_rx_metadata_to_pp_string(
    shd_rx_metadata_handle h,
    char* pp_string_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string pp_string_cpp = h->rx_metadata_cpp.to_pp_string();
        memset(pp_string_out, '\0', strbuffer_len);
        strncpy(pp_string_out, pp_string_cpp.c_str(), strbuffer_len);
    )
}

shd_error shd_rx_metadata_error_code(
    shd_rx_metadata_handle h,
    shd_rx_metadata_error_code_t *error_code_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *error_code_out = shd_rx_metadata_error_code_t(h->rx_metadata_cpp.error_code);
    )
}

shd_error shd_rx_metadata_strerror(
    shd_rx_metadata_handle h,
    char* strerror_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string strerror_cpp = h->rx_metadata_cpp.strerror();
        memset(strerror_out, '\0', strbuffer_len);
        strncpy(strerror_out, strerror_cpp.c_str(), strbuffer_len);
    )
}

shd_error shd_rx_metadata_last_error(
    shd_rx_metadata_handle h,
    char* error_out,
    size_t strbuffer_len
){
    SHD_SAFE_C(
        memset(error_out, '\0', strbuffer_len);
        strncpy(error_out, h->last_error.c_str(), strbuffer_len);
    )
}

/*
 * TX metadata
 */

shd_error shd_tx_metadata_make(
    shd_tx_metadata_handle* handle,
    bool has_time_spec,
    time_t full_secs,
    double frac_secs,
    bool start_of_burst,
    bool end_of_burst
){
    SHD_SAFE_C(
        *handle = new shd_tx_metadata_t;
        (*handle)->tx_metadata_cpp.has_time_spec = has_time_spec;
        if(has_time_spec){
            (*handle)->tx_metadata_cpp.time_spec = shd::time_spec_t(full_secs, frac_secs);
        }
        (*handle)->tx_metadata_cpp.start_of_burst = start_of_burst;
        (*handle)->tx_metadata_cpp.end_of_burst = end_of_burst;
    )
}

shd_error shd_tx_metadata_free(
    shd_tx_metadata_handle* handle
){
    SHD_SAFE_C(
        delete *handle;
        *handle = NULL;
    )
}

shd_error shd_tx_metadata_has_time_spec(
    shd_tx_metadata_handle h,
    bool *result_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
         *result_out = h->tx_metadata_cpp.has_time_spec;
    )
}

shd_error shd_tx_metadata_time_spec(
    shd_tx_metadata_handle h,
    time_t *full_secs_out,
    double *frac_secs_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::time_spec_t time_spec_cpp = h->tx_metadata_cpp.time_spec;
        *full_secs_out = time_spec_cpp.get_full_secs();
        *frac_secs_out = time_spec_cpp.get_frac_secs();
    )
}

shd_error shd_tx_metadata_start_of_burst(
    shd_tx_metadata_handle h,
    bool *result_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *result_out = h->tx_metadata_cpp.start_of_burst;
    )
}

shd_error shd_tx_metadata_end_of_burst(
    shd_tx_metadata_handle h,
    bool *result_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *result_out = h->tx_metadata_cpp.end_of_burst;
    )
}

shd_error shd_tx_metadata_last_error(
    shd_tx_metadata_handle h,
    char* error_out,
    size_t strbuffer_len
){
    SHD_SAFE_C(
        memset(error_out, '\0', strbuffer_len);
        strncpy(error_out, h->last_error.c_str(), strbuffer_len);
    )
}

/*
 * Async metadata
 */

shd_error shd_async_metadata_make(
    shd_async_metadata_handle* handle
){
    SHD_SAFE_C(
        *handle = new shd_async_metadata_t;
    )
}

shd_error shd_async_metadata_free(
    shd_async_metadata_handle* handle
){
    SHD_SAFE_C(
        delete *handle;
        *handle = NULL;
    )
}

shd_error shd_async_metadata_channel(
    shd_async_metadata_handle h,
    size_t *channel_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *channel_out = h->async_metadata_cpp.channel;
    )
}

shd_error shd_async_metadata_has_time_spec(
    shd_async_metadata_handle h,
    bool *result_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
         *result_out = h->async_metadata_cpp.has_time_spec;
    )
}

shd_error shd_async_metadata_time_spec(
    shd_async_metadata_handle h,
    time_t *full_secs_out,
    double *frac_secs_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::time_spec_t time_spec_cpp = h->async_metadata_cpp.time_spec;
        *full_secs_out = time_spec_cpp.get_full_secs();
        *frac_secs_out = time_spec_cpp.get_frac_secs();
    )
}

shd_error shd_async_metadata_event_code(
    shd_async_metadata_handle h,
    shd_async_metadata_event_code_t *event_code_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *event_code_out = shd_async_metadata_event_code_t(h->async_metadata_cpp.event_code);
    )
}

shd_error shd_async_metadata_user_payload(
    shd_async_metadata_handle h,
    uint32_t user_payload_out[4]
){
    SHD_SAFE_C_SAVE_ERROR(h,
        memcpy(user_payload_out, h->async_metadata_cpp.user_payload, 4*sizeof(uint32_t));
    )
}

shd_error shd_async_metadata_last_error(
    shd_async_metadata_handle h,
    char* error_out,
    size_t strbuffer_len
){
    SHD_SAFE_C(
        memset(error_out, '\0', strbuffer_len);
        strncpy(error_out, h->last_error.c_str(), strbuffer_len);
    )
}
