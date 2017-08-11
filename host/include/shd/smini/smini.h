//
// Copyright 2015-2016 Ettus Research LLC
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

#ifndef INCLUDED_SHD_SMINI_H
#define INCLUDED_SHD_SMINI_H

#include <shd/config.h>
#include <shd/error.h>
#include <shd/types/metadata.h>
#include <shd/types/ranges.h>
#include <shd/types/sensors.h>
#include <shd/types/string_vector.h>
#include <shd/types/tune_request.h>
#include <shd/types/tune_result.h>
#include <shd/types/smini_info.h>
#include <shd/smini/mboard_eeprom.h>
#include <shd/smini/dboard_eeprom.h>
#include <shd/smini/subdev_spec.h>
/* version.hpp is safe to include in C: */
#include <shd/version.hpp> /* Provides SHD_VERSION */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

//! Register info
typedef struct {
    size_t bitwidth;
    bool readable;
    bool writable;
} shd_smini_register_info_t;

/*
 * Streamers
 */

//! A struct of parameters to construct a stream.
/*!
 * See shd::stream_args_t for more details.
 */
typedef struct {
    //! Format of host memory
    char* cpu_format;
    //! Over-the-wire format
    char* otw_format;
    //! Other stream args
    char* args;
    //! Array that lists channels
    size_t* channel_list;
    //! Number of channels
    int n_channels;
} shd_stream_args_t;

//! How streaming is issued to the device
/*!
 * See shd::stream_cmd_t for more details.
 */
typedef enum {
    //! Stream samples indefinitely
    SHD_STREAM_MODE_START_CONTINUOUS   = 97,
    //! End continuous streaming
    SHD_STREAM_MODE_STOP_CONTINUOUS    = 111,
    //! Stream some number of samples and finish
    SHD_STREAM_MODE_NUM_SAMPS_AND_DONE = 100,
    //! Stream some number of samples but expect more
    SHD_STREAM_MODE_NUM_SAMPS_AND_MORE = 109
} shd_stream_mode_t;

//! Define how device streams to host
/*!
 * See shd::stream_cmd_t for more details.
 */
typedef struct {
    //! How streaming is issued to the device
    shd_stream_mode_t stream_mode;
    //! Number of samples
    size_t num_samps;
    //! Stream now?
    bool stream_now;
    //! If not now, then full seconds into future to stream
    time_t time_spec_full_secs;
    //! If not now, then fractional seconds into future to stream
    double time_spec_frac_secs;
} shd_stream_cmd_t;

struct shd_rx_streamer;
struct shd_tx_streamer;

//! C-level interface for working with an RX streamer
/*!
 * See shd::rx_streamer for more details.
 */
typedef struct shd_rx_streamer* shd_rx_streamer_handle;

//! C-level interface for working with a TX streamer
/*!
 * See shd::tx_streamer for more details.
 */
typedef struct shd_tx_streamer* shd_tx_streamer_handle;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * RX Streamer
 */

//! Create an RX streamer handle.
/*!
 * NOTE: Using this streamer before passing it into shd_smini_get_rx_stream()
 * will result in undefined behavior.
 */
SHD_API shd_error shd_rx_streamer_make(
    shd_rx_streamer_handle *h
);

//! Free an RX streamer handle.
/*!
 * NOTE: Using a streamer after passing it into this function will result
 * in a segmentation fault.
 */
SHD_API shd_error shd_rx_streamer_free(
    shd_rx_streamer_handle *h
);

//! Get the number of channels associated with this streamer
SHD_API shd_error shd_rx_streamer_num_channels(
    shd_rx_streamer_handle h,
    size_t *num_channels_out
);

//! Get the max number of samples per buffer per packet
SHD_API shd_error shd_rx_streamer_max_num_samps(
    shd_rx_streamer_handle h,
    size_t *max_num_samps_out
);

//! Receive buffers containing samples into the given RX streamer
/*!
 * See shd::rx_streamer::recv() for more details.
 *
 * \param h RX streamer handle
 * \param buffs pointer to buffers in which to receive samples
 * \param samps_per_buff max number of samples per buffer
 * \param md handle to RX metadata in which to receive results
 * \param timeout timeout in seconds to wait for a packet
 * \param one_packet send a single packet
 * \param items_recvd pointer to output variable for number of samples received
 */
SHD_API shd_error shd_rx_streamer_recv(
    shd_rx_streamer_handle h,
    void** buffs,
    size_t samps_per_buff,
    shd_rx_metadata_handle *md,
    double timeout,
    bool one_packet,
    size_t *items_recvd
);

//! Issue the given stream command
/*!
 * See shd::rx_streamer::issue_stream_cmd() for more details.
 */
SHD_API shd_error shd_rx_streamer_issue_stream_cmd(
    shd_rx_streamer_handle h,
    const shd_stream_cmd_t *stream_cmd
);

//! Get the last error reported by the RX streamer
/*!
 * NOTE: This will overwrite the string currently in error_out before
 * using it to return its error.
 *
 * \param h RX streamer handle
 * \param error_out string buffer in which to place error
 * \param strbuffer_len buffer size
 */
SHD_API shd_error shd_rx_streamer_last_error(
    shd_rx_streamer_handle h,
    char* error_out,
    size_t strbuffer_len
);

/*
 * TX Streamer
 */

//! Create an TX streamer handle.
/*!
 * NOTE: Using this streamer before passing it into shd_smini_get_tx_stream()
 * will result in undefined behavior.
 */
SHD_API shd_error shd_tx_streamer_make(
    shd_tx_streamer_handle *h
);

//! Free an TX streamer handle.
/*!
 * NOTE: Using a streamer after passing it into this function will result
 * in a segmentation fault.
 */
SHD_API shd_error shd_tx_streamer_free(
    shd_tx_streamer_handle *h
);

//! Get the number of channels associated with this streamer
SHD_API shd_error shd_tx_streamer_num_channels(
    shd_tx_streamer_handle h,
    size_t *num_channels_out
);

//! Get the max number of samples per buffer per packet
SHD_API shd_error shd_tx_streamer_max_num_samps(
    shd_tx_streamer_handle h,
    size_t *max_num_samps_out
);

//! Send buffers containing samples described by the metadata
/*!
 * See shd::tx_streamer::send() for more details.
 *
 * \param h TX streamer handle
 * \param buffs pointer to buffers containing samples to send
 * \param samps_per_buff max number of samples per buffer
 * \param md handle to TX metadata
 * \param timeout timeout in seconds to wait for a packet
 * \param items_sent pointer to output variable for number of samples send
 */
SHD_API shd_error shd_tx_streamer_send(
    shd_tx_streamer_handle h,
    const void **buffs,
    size_t samps_per_buff,
    shd_tx_metadata_handle *md,
    double timeout,
    size_t *items_sent
);

//! Receive an asynchronous message from this streamer
/*!
 * See shd::tx_streamer::recv_async_msg() for more details.
 */
SHD_API shd_error shd_tx_streamer_recv_async_msg(
    shd_tx_streamer_handle h,
    shd_async_metadata_handle *md,
    double timeout,
    bool *valid
);

//! Get the last error reported by the TX streamer
/*!
 * NOTE: This will overwrite the string currently in error_out before
 * using it to return its error.
 *
 * \param h TX streamer handle
 * \param error_out string buffer in which to place error
 * \param strbuffer_len buffer size
 */
SHD_API shd_error shd_tx_streamer_last_error(
    shd_tx_streamer_handle h,
    char* error_out,
    size_t strbuffer_len
);

#ifdef __cplusplus
}
#endif

/****************************************************************************
 * Public Datatypes for SMINI / streamer handling.
 ***************************************************************************/
struct shd_smini;

//! C-level interface for working with a SMINI device
/*
 * See shd::smini::multi_smini for more details.
 *
 * NOTE: You must pass this handle into shd_smini_make before using it.
 */
typedef struct shd_smini* shd_smini_handle;

/****************************************************************************
 * SMINI Make / Free API calls
 ***************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

//! Find all connected SMINI devices.
/*!
 * See shd::device::find() for more details.
 */
SHD_API shd_error shd_smini_find(
    const char* args,
    shd_string_vector_handle *strings_out
);

//! Create a SMINI handle.
/*!
 * \param h the handle
 * \param args device args (e.g. "type=x300")
 */
SHD_API shd_error shd_smini_make(
    shd_smini_handle *h,
    const char *args
);

//! Safely destroy the SMINI object underlying the handle.
/*!
 * NOTE: Attempting to use a SMINI handle after passing it into this function
 * will result in a segmentation fault.
 */
SHD_API shd_error shd_smini_free(
    shd_smini_handle *h
);

//! Get the last error reported by the SMINI handle
SHD_API shd_error shd_smini_last_error(
    shd_smini_handle h,
    char* error_out,
    size_t strbuffer_len
);

//! Create RX streamer from a SMINI handle and given stream args
SHD_API shd_error shd_smini_get_rx_stream(
    shd_smini_handle h,
    shd_stream_args_t *stream_args,
    shd_rx_streamer_handle h_out
);

//! Create TX streamer from a SMINI handle and given stream args
SHD_API shd_error shd_smini_get_tx_stream(
    shd_smini_handle h,
    shd_stream_args_t *stream_args,
    shd_tx_streamer_handle h_out
);

/****************************************************************************
 * multi_smini API calls
 ***************************************************************************/

//! Get RX info from the SMINI device
/*!
 * NOTE: After calling this function, shd_smini_rx_info_free() must be called on info_out.
 */
SHD_API shd_error shd_smini_get_rx_info(
    shd_smini_handle h,
    size_t chan,
    shd_smini_rx_info_t *info_out
);

//! Get TX info from the SMINI device
/*!
 * NOTE: After calling this function, shd_smini_tx_info_free() must be called on info_out.
 */
SHD_API shd_error shd_smini_get_tx_info(
    shd_smini_handle h,
    size_t chan,
    shd_smini_tx_info_t *info_out
);

/****************************************************************************
 * Motherboard methods
 ***************************************************************************/

//! Set the master clock rate.
/*!
 * See shd::smini::multi_smini::set_master_clock_rate() for more details.
 */
SHD_API shd_error shd_smini_set_master_clock_rate(
    shd_smini_handle h,
    double rate,
    size_t mboard
);

//! Get the master clock rate.
/*!
 * See shd::smini::multi_smini::get_master_clock_rate() for more details.
 */
SHD_API shd_error shd_smini_get_master_clock_rate(
    shd_smini_handle h,
    size_t mboard,
    double *clock_rate_out
);

//! Get a pretty-print representation of the SMINI device.
/*!
 * See shd::smini::multi_smini::get_pp_string() for more details.
 */
SHD_API shd_error shd_smini_get_pp_string(
    shd_smini_handle h,
    char* pp_string_out,
    size_t strbuffer_len
);

//! Get the motherboard name for the given device
/*!
 * See shd::smini::multi_smini::get_mboard_name() for more details.
 */
SHD_API shd_error shd_smini_get_mboard_name(
    shd_smini_handle h,
    size_t mboard,
    char* mboard_name_out,
    size_t strbuffer_len
);

//! Get the SMINI device's current internal time
/*!
 * See shd::smini::multi_smini::get_time_now() for more details.
 */
SHD_API shd_error shd_smini_get_time_now(
    shd_smini_handle h,
    size_t mboard,
    time_t *full_secs_out,
    double *frac_secs_out
);

//! Get the time when this device's last PPS pulse occurred
/*!
 * See shd::smini::multi_smini::get_time_last_pps() for more details.
 */
SHD_API shd_error shd_smini_get_time_last_pps(
    shd_smini_handle h,
    size_t mboard,
    time_t *full_secs_out,
    double *frac_secs_out
);

//! Set the SMINI device's time
/*!
 * See shd::smini::multi_smini::set_time_now() for more details.
 */
SHD_API shd_error shd_smini_set_time_now(
    shd_smini_handle h,
    time_t full_secs,
    double frac_secs,
    size_t mboard
);

//! Set the SMINI device's time to the given value upon the next PPS detection
/*!
 * See shd::smini::multi_smini::set_time_next_pps() for more details.
 */
SHD_API shd_error shd_smini_set_time_next_pps(
    shd_smini_handle h,
    time_t full_secs,
    double frac_secs,
    size_t mboard
);

//! Synchronize the time across all motherboards
/*!
 * See shd::smini::multi_smini::set_time_unknown_pps() for more details.
 */
SHD_API shd_error shd_smini_set_time_unknown_pps(
    shd_smini_handle h,
    time_t full_secs,
    double frac_secs
);

//! Are all motherboard times synchronized?
SHD_API shd_error shd_smini_get_time_synchronized(
    shd_smini_handle h,
    bool *result_out
);

//! Set the time at which timed commands will take place
/*!
 * See shd::smini::multi_smini::set_command_time() for more details.
 */
SHD_API shd_error shd_smini_set_command_time(
    shd_smini_handle h,
    time_t full_secs,
    double frac_secs,
    size_t mboard
);

//! Clear the command time so that commands are sent ASAP
SHD_API shd_error shd_smini_clear_command_time(
    shd_smini_handle h,
    size_t mboard
);

//! Set the time source for the given device
/*!
 * See shd::smini::multi_smini::set_time_source() for more details.
 */
SHD_API shd_error shd_smini_set_time_source(
    shd_smini_handle h,
    const char* time_source,
    size_t mboard
);

//! Get the time source for the given device
/*!
 * See shd::smini::multi_smini::get_time_source() for more details.
 */
SHD_API shd_error shd_smini_get_time_source(
    shd_smini_handle h,
    size_t mboard,
    char* time_source_out,
    size_t strbuffer_len
);

//! Get a list of time sources for the given device
SHD_API shd_error shd_smini_get_time_sources(
    shd_smini_handle h,
    size_t mboard,
    shd_string_vector_handle *time_sources_out
);

//! Set the given device's clock source
/*!
 * See shd::smini::multi_smini::set_clock_source() for more details.
 */
SHD_API shd_error shd_smini_set_clock_source(
    shd_smini_handle h,
    const char* clock_source,
    size_t mboard
);

//! Get the given device's clock source
/*!
 * See shd::smini::multi_smini::get_clock_source() for more details.
 */
SHD_API shd_error shd_smini_get_clock_source(
    shd_smini_handle h,
    size_t mboard,
    char* clock_source_out,
    size_t strbuffer_len
);

//! Get a list of clock sources for the given device
SHD_API shd_error shd_smini_get_clock_sources(
    shd_smini_handle h,
    size_t mboard,
    shd_string_vector_handle *clock_sources_out
);

//! Enable or disable sending the clock source to an output connector
/*!
 * See shd::smini::set_clock_source_out() for more details.
 */
SHD_API shd_error shd_smini_set_clock_source_out(
    shd_smini_handle h,
    bool enb,
    size_t mboard
);

//! Enable or disable sending the time source to an output connector
/*!
 * See shd::smini::set_time_source_out() for more details.
 */
SHD_API shd_error shd_smini_set_time_source_out(
    shd_smini_handle h,
    bool enb,
    size_t mboard
);

//! Get the number of devices associated with the given SMINI handle
SHD_API shd_error shd_smini_get_num_mboards(
    shd_smini_handle h,
    size_t *num_mboards_out
);

//! Get the value associated with the given sensor name
SHD_API shd_error shd_smini_get_mboard_sensor(
    shd_smini_handle h,
    const char* name,
    size_t mboard,
    shd_sensor_value_handle *sensor_value_out
);

//! Get a list of motherboard sensors for the given device
SHD_API shd_error shd_smini_get_mboard_sensor_names(
    shd_smini_handle h,
    size_t mboard,
    shd_string_vector_handle *mboard_sensor_names_out
);

//! Perform a write on a user configuration register bus
/*!
 * See shd::smini::multi_smini::set_user_register() for more details.
 */
SHD_API shd_error shd_smini_set_user_register(
    shd_smini_handle h,
    uint8_t addr,
    uint32_t data,
    size_t mboard
);

/****************************************************************************
 * EEPROM access methods
 ***************************************************************************/

//! Get a handle for the given motherboard's EEPROM
SHD_API shd_error shd_smini_get_mboard_eeprom(
    shd_smini_handle h,
    shd_mboard_eeprom_handle mb_eeprom,
    size_t mboard
);

//! Set values in the given motherboard's EEPROM
SHD_API shd_error shd_smini_set_mboard_eeprom(
    shd_smini_handle h,
    shd_mboard_eeprom_handle mb_eeprom,
    size_t mboard
);

//! Get a handle for the given device's daughterboard EEPROM
SHD_API shd_error shd_smini_get_dboard_eeprom(
    shd_smini_handle h,
    shd_dboard_eeprom_handle db_eeprom,
    const char* unit,
    const char* slot,
    size_t mboard
);

//! Set values in the given daughterboard's EEPROM
SHD_API shd_error shd_smini_set_dboard_eeprom(
    shd_smini_handle h,
    shd_dboard_eeprom_handle db_eeprom,
    const char* unit,
    const char* slot,
    size_t mboard
);

/****************************************************************************
 * RX methods
 ***************************************************************************/

//! Map the given device's RX frontend to a channel
/*!
 * See shd::smini::multi_smini::set_rx_subdev_spec() for more details.
 */
SHD_API shd_error shd_smini_set_rx_subdev_spec(
    shd_smini_handle h,
    shd_subdev_spec_handle subdev_spec,
    size_t mboard
);

//! Get the RX frontend specification for the given device
SHD_API shd_error shd_smini_get_rx_subdev_spec(
    shd_smini_handle h,
    size_t mboard,
    shd_subdev_spec_handle subdev_spec_out
);

//! Get the number of RX channels for the given handle
SHD_API shd_error shd_smini_get_rx_num_channels(
    shd_smini_handle h,
    size_t *num_channels_out
);

//! Get the name for the RX frontend
SHD_API shd_error shd_smini_get_rx_subdev_name(
    shd_smini_handle h,
    size_t chan,
    char* rx_subdev_name_out,
    size_t strbuffer_len
);

//! Set the given RX channel's sample rate (in Sps)
SHD_API shd_error shd_smini_set_rx_rate(
    shd_smini_handle h,
    double rate,
    size_t chan
);

//! Get the given RX channel's sample rate (in Sps)
SHD_API shd_error shd_smini_get_rx_rate(
    shd_smini_handle h,
    size_t chan,
    double *rate_out
);

//! Get a range of possible RX rates for the given channel
SHD_API shd_error shd_smini_get_rx_rates(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle rates_out
);

//! Set the given channel's center RX frequency
SHD_API shd_error shd_smini_set_rx_freq(
    shd_smini_handle h,
    shd_tune_request_t *tune_request,
    size_t chan,
    shd_tune_result_t *tune_result
);

//! Get the given channel's center RX frequency
SHD_API shd_error shd_smini_get_rx_freq(
    shd_smini_handle h,
    size_t chan,
    double *freq_out
);

//! Get all possible center frequency ranges for the given channel
/*!
 * See shd::smini::multi_smini::get_rx_freq_range() for more details.
 */
SHD_API shd_error shd_smini_get_rx_freq_range(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle freq_range_out
);

//! Get all possible RF frequency ranges for the given channel's RX RF frontend
SHD_API shd_error shd_smini_get_fe_rx_freq_range(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle freq_range_out
);

//! A wildcard for all LO names
SHD_UNUSED(static const char* SHD_SMINI_ALL_LOS) = "all";

//! Get a list of possible LO stage names
/*
 * See shd::smini::multi_smini::get_rx_lo_names() for more details.
 */
SHD_API shd_error shd_smini_get_rx_lo_names(
    shd_smini_handle h,
    size_t chan,
    shd_string_vector_handle *rx_lo_names_out
);

//! Set the LO source for the SMINI device
/*
 * See shd::smini::multi_smini::set_rx_lo_source() for more details.
 */
SHD_API shd_error shd_smini_set_rx_lo_source(
    shd_smini_handle h,
    const char* src,
    const char* name,
    size_t chan
);

//! Get the currently set LO source
SHD_API shd_error shd_smini_get_rx_lo_source(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    char* rx_lo_source_out,
    size_t strbuffer_len
);

//! Get a list of possible LO sources
SHD_API shd_error shd_smini_get_rx_lo_sources(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    shd_string_vector_handle *rx_lo_sources_out
);

//! Set whether the LO used by the SMINI device is exported
/*
 * See shd::smini::multi_smini::set_rx_lo_enabled() for more details.
 */
SHD_API shd_error shd_smini_set_rx_lo_export_enabled(
    shd_smini_handle h,
    bool enabled,
    const char* name,
    size_t chan
);

//! Returns true if the currently selected LO is being exported.
SHD_API shd_error shd_smini_get_rx_lo_export_enabled(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    bool* result_out
);

//! Set the RX LO frequency.
SHD_API shd_error shd_smini_set_rx_lo_freq(
    shd_smini_handle h,
    double freq,
    const char* name,
    size_t chan,
    double* coerced_freq_out
);

//! Get the current RX LO frequency.
SHD_API shd_error shd_smini_get_rx_lo_freq(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    double* rx_lo_freq_out
);

//! Set the RX gain for the given channel and name
SHD_API shd_error shd_smini_set_rx_gain(
    shd_smini_handle h,
    double gain,
    size_t chan,
    const char *gain_name
);

//! Set the normalized RX gain [0.0, 1.0] for the given channel
/*!
 * See shd::smini::multi_smini::set_normalized_rx_gain() for more details.
 */
SHD_API shd_error shd_smini_set_normalized_rx_gain(
    shd_smini_handle h,
    double gain,
    size_t chan
);

//! Enable or disable the given channel's RX AGC module
/*!
 * See shd::smini::multi_smini::set_rx_agc() for more details.
 */
SHD_API shd_error shd_smini_set_rx_agc(
    shd_smini_handle h,
    bool enable,
    size_t chan
);

//! Get the given channel's RX gain
SHD_API shd_error shd_smini_get_rx_gain(
    shd_smini_handle h,
    size_t chan,
    const char *gain_name,
    double *gain_out
);

//! Get the given channel's normalized RX gain [0.0, 1.0]
/*!
 * See shd::smini::multi_smini::get_normalized_rx_gain() for more details.
 */
SHD_API shd_error shd_smini_get_normalized_rx_gain(
    shd_smini_handle h,
    size_t chan,
    double *gain_out
);

//! Get all possible gain ranges for the given channel and name
SHD_API shd_error shd_smini_get_rx_gain_range(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    shd_meta_range_handle gain_range_out
);

//! Get a list of RX gain names for the given channel
SHD_API shd_error shd_smini_get_rx_gain_names(
    shd_smini_handle h,
    size_t chan,
    shd_string_vector_handle *gain_names_out
);

//! Set the RX antenna for the given channel
SHD_API shd_error shd_smini_set_rx_antenna(
    shd_smini_handle h,
    const char* ant,
    size_t chan
);

//! Get the RX antenna for the given channel
SHD_API shd_error shd_smini_get_rx_antenna(
    shd_smini_handle h,
    size_t chan,
    char* ant_out,
    size_t strbuffer_len
);

//! Get a list of RX antennas associated with the given channels
SHD_API shd_error shd_smini_get_rx_antennas(
    shd_smini_handle h,
    size_t chan,
    shd_string_vector_handle *antennas_out
);

//! Get a list of RX sensors associated with the given channels
SHD_API shd_error shd_smini_get_rx_sensor_names(
    shd_smini_handle h,
    size_t chan,
    shd_string_vector_handle *sensor_names_out
);

//! Set the bandwidth for the given channel's RX frontend
SHD_API shd_error shd_smini_set_rx_bandwidth(
    shd_smini_handle h,
    double bandwidth,
    size_t chan
);

//! Get the bandwidth for the given channel's RX frontend
SHD_API shd_error shd_smini_get_rx_bandwidth(
    shd_smini_handle h,
    size_t chan,
    double *bandwidth_out
);

//! Get all possible bandwidth ranges for the given channel's RX frontend
SHD_API shd_error shd_smini_get_rx_bandwidth_range(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle bandwidth_range_out
);

//! Get the value for the given RX sensor
SHD_API shd_error shd_smini_get_rx_sensor(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    shd_sensor_value_handle *sensor_value_out
);

//! Enable or disable RX DC offset correction for the given channel
/*!
 * See shd::smini::multi_smini::set_rx_dc_offset() for more details.
 */
SHD_API shd_error shd_smini_set_rx_dc_offset_enabled(
    shd_smini_handle h,
    bool enb,
    size_t chan
);

//! Enable or disable RX IQ imbalance correction for the given channel
SHD_API shd_error shd_smini_set_rx_iq_balance_enabled(
    shd_smini_handle h,
    bool enb,
    size_t chan
);

/****************************************************************************
 * TX methods
 ***************************************************************************/

//! Map the given device's TX frontend to a channel
/*!
 * See shd::smini::multi_smini::set_tx_subdev_spec() for more details.
 */
SHD_API shd_error shd_smini_set_tx_subdev_spec(
    shd_smini_handle h,
    shd_subdev_spec_handle subdev_spec,
    size_t mboard
);

//! Get the TX frontend specification for the given device
SHD_API shd_error shd_smini_get_tx_subdev_spec(
    shd_smini_handle h,
    size_t mboard,
    shd_subdev_spec_handle subdev_spec_out
);

//! Get the number of TX channels for the given handle
SHD_API shd_error shd_smini_get_tx_num_channels(
    shd_smini_handle h,
    size_t *num_channels_out
);

//! Get the name for the RX frontend
SHD_API shd_error shd_smini_get_tx_subdev_name(
    shd_smini_handle h,
    size_t chan,
    char* tx_subdev_name_out,
    size_t strbuffer_len
);

//! Set the given RX channel's sample rate (in Sps)
SHD_API shd_error shd_smini_set_tx_rate(
    shd_smini_handle h,
    double rate,
    size_t chan
);

//! Get the given RX channel's sample rate (in Sps)
SHD_API shd_error shd_smini_get_tx_rate(
    shd_smini_handle h,
    size_t chan,
    double *rate_out
);

//! Get a range of possible RX rates for the given channel
SHD_API shd_error shd_smini_get_tx_rates(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle rates_out
);

//! Set the given channel's center TX frequency
SHD_API shd_error shd_smini_set_tx_freq(
    shd_smini_handle h,
    shd_tune_request_t *tune_request,
    size_t chan,
    shd_tune_result_t *tune_result
);

//! Get the given channel's center TX frequency
SHD_API shd_error shd_smini_get_tx_freq(
    shd_smini_handle h,
    size_t chan,
    double *freq_out
);

//! Get all possible center frequency ranges for the given channel
/*!
 * See shd::smini::multi_smini::get_rx_freq_range() for more details.
 */
SHD_API shd_error shd_smini_get_tx_freq_range(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle freq_range_out
);

//! Get all possible RF frequency ranges for the given channel's TX RF frontend
SHD_API shd_error shd_smini_get_fe_tx_freq_range(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle freq_range_out
);

//! Set the TX gain for the given channel and name
SHD_API shd_error shd_smini_set_tx_gain(
    shd_smini_handle h,
    double gain,
    size_t chan,
    const char *gain_name
);

//! Set the normalized TX gain [0.0, 1.0] for the given channel
/*!
 * See shd::smini::multi_smini::set_normalized_tx_gain() for more details.
 */
SHD_API shd_error shd_smini_set_normalized_tx_gain(
    shd_smini_handle h,
    double gain,
    size_t chan
);

//! Get all possible gain ranges for the given channel and name
SHD_API shd_error shd_smini_get_tx_gain_range(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    shd_meta_range_handle gain_range_out
);

//! Get the given channel's RX gain
SHD_API shd_error shd_smini_get_tx_gain(
    shd_smini_handle h,
    size_t chan,
    const char *gain_name,
    double *gain_out
);

//! Get the given channel's normalized TX gain [0.0, 1.0]
/*!
 * See shd::smini::multi_smini::get_normalized_tx_gain() for more details.
 */
SHD_API shd_error shd_smini_get_normalized_tx_gain(
    shd_smini_handle h,
    size_t chan,
    double *gain_out
);

//! Get a list of TX gain names for the given channel
SHD_API shd_error shd_smini_get_tx_gain_names(
    shd_smini_handle h,
    size_t chan,
    shd_string_vector_handle *gain_names_out
);

//! Set the TX antenna for the given channel
SHD_API shd_error shd_smini_set_tx_antenna(
    shd_smini_handle h,
    const char* ant,
    size_t chan
);

//! Get the TX antenna for the given channel
SHD_API shd_error shd_smini_get_tx_antenna(
    shd_smini_handle h,
    size_t chan,
    char* ant_out,
    size_t strbuffer_len
);

//! Get a list of tx antennas associated with the given channels
SHD_API shd_error shd_smini_get_tx_antennas(
    shd_smini_handle h,
    size_t chan,
    shd_string_vector_handle *antennas_out
);

//! Set the bandwidth for the given channel's TX frontend
SHD_API shd_error shd_smini_set_tx_bandwidth(
    shd_smini_handle h,
    double bandwidth,
    size_t chan
);

//! Get the bandwidth for the given channel's TX frontend
SHD_API shd_error shd_smini_get_tx_bandwidth(
    shd_smini_handle h,
    size_t chan,
    double *bandwidth_out
);

//! Get all possible bandwidth ranges for the given channel's TX frontend
SHD_API shd_error shd_smini_get_tx_bandwidth_range(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle bandwidth_range_out
);

//! Get the value for the given TX sensor
SHD_API shd_error shd_smini_get_tx_sensor(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    shd_sensor_value_handle *sensor_value_out
);

//! Get a list of TX sensors associated with the given channels
SHD_API shd_error shd_smini_get_tx_sensor_names(
    shd_smini_handle h,
    size_t chan,
    shd_string_vector_handle *sensor_names_out
);

//! Enable or disable TX DC offset correction for the given channel
/*!
 * See shd::smini::multi_smini::set_tx_dc_offset() for more details.
 */
SHD_API shd_error shd_smini_set_tx_dc_offset_enabled(
    shd_smini_handle h,
    bool enb,
    size_t chan
);

//! Enable or disable TX IQ imbalance correction for the given channel
SHD_API shd_error shd_smini_set_tx_iq_balance_enabled(
    shd_smini_handle h,
    bool enb,
    size_t chan
);

/****************************************************************************
 * GPIO methods
 ***************************************************************************/

//! Get a list of GPIO banks associated with the given channels
SHD_API shd_error shd_smini_get_gpio_banks(
    shd_smini_handle h,
    size_t mboard,
    shd_string_vector_handle *gpio_banks_out
);

//! Set a GPIO attribute for a given GPIO bank
/*!
 * See shd::smini::multi_smini::set_gpio_attr() for more details.
 */
SHD_API shd_error shd_smini_set_gpio_attr(
    shd_smini_handle h,
    const char* bank,
    const char* attr,
    uint32_t value,
    uint32_t mask,
    size_t mboard
);

//! Get a GPIO attribute on a particular GPIO bank
/*!
 * See shd::smini::multi_smini::get_gpio_attr() for more details.
 */
SHD_API shd_error shd_smini_get_gpio_attr(
    shd_smini_handle h,
    const char* bank,
    const char* attr,
    size_t mboard,
    uint32_t *attr_out
);

//! Enumerate the full paths of SMINI registers available for read/write
SHD_API shd_error shd_smini_enumerate_registers(
    shd_smini_handle h,
    size_t mboard,
    shd_string_vector_handle *registers_out
);

//! Get more information about a low-level device register
SHD_API shd_error shd_smini_get_register_info(
    shd_smini_handle h,
    const char* path,
    size_t mboard,
    shd_smini_register_info_t *register_info_out
);

//! Write a low-level register field for a device register in the SMINI hardware
SHD_API shd_error shd_smini_write_register(
    shd_smini_handle h,
    const char* path,
    uint32_t field,
    uint64_t value,
    size_t mboard
);

//! Read a low-level register field from a device register in the SMINI hardware
SHD_API shd_error shd_smini_read_register(
    shd_smini_handle h,
    const char* path,
    uint32_t field,
    size_t mboard,
    uint64_t *value_out
);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_SHD_SMINI_H */
