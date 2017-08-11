/*
 * Copyright 2015-2016 Ettus Research LLC
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

/* C-Interface for multi_smini */

#include <shd/utils/static.hpp>
#include <shd/smini/multi_smini.hpp>

#include <shd/error.h>
#include <shd/smini/smini.h>

#include <boost/foreach.hpp>
#include <boost/thread/mutex.hpp>

#include <string.h>
#include <map>

/****************************************************************************
 * Helpers
 ***************************************************************************/
shd::stream_args_t stream_args_c_to_cpp(const shd_stream_args_t *stream_args_c)
{
    std::string otw_format(stream_args_c->otw_format);
    std::string cpu_format(stream_args_c->cpu_format);
    std::string args(stream_args_c->args);
    std::vector<size_t> channels(stream_args_c->channel_list, stream_args_c->channel_list + stream_args_c->n_channels);

    shd::stream_args_t stream_args_cpp(cpu_format, otw_format);
    stream_args_cpp.args = args;
    stream_args_cpp.channels = channels;

    return stream_args_cpp;
}

shd::stream_cmd_t stream_cmd_c_to_cpp(const shd_stream_cmd_t *stream_cmd_c)
{
    shd::stream_cmd_t stream_cmd_cpp(shd::stream_cmd_t::stream_mode_t(stream_cmd_c->stream_mode));
    stream_cmd_cpp.num_samps   = stream_cmd_c->num_samps;
    stream_cmd_cpp.stream_now  = stream_cmd_c->stream_now;
    stream_cmd_cpp.time_spec   = shd::time_spec_t(stream_cmd_c->time_spec_full_secs, stream_cmd_c->time_spec_frac_secs);
    return stream_cmd_cpp;
}

/****************************************************************************
 * Registry / Pointer Management
 ***************************************************************************/
/* Public structs */
struct shd_smini {
    size_t smini_index;
    std::string last_error;
};

struct shd_tx_streamer {
    size_t smini_index;
    size_t streamer_index;
    std::string last_error;
};

struct shd_rx_streamer {
    size_t smini_index;
    size_t streamer_index;
    std::string last_error;
};

/* Not public: We use this for our internal registry */
struct smini_ptr {
    shd::smini::multi_smini::sptr ptr;
    std::vector< shd::rx_streamer::sptr > rx_streamers;
    std::vector< shd::tx_streamer::sptr > tx_streamers;
    static size_t smini_counter;
};
size_t smini_ptr::smini_counter = 0;
typedef struct smini_ptr smini_ptr;
/* Prefer map, because the list can be discontiguous */
typedef std::map<size_t, smini_ptr> smini_ptrs;

SHD_SINGLETON_FCN(smini_ptrs, get_smini_ptrs);
/* Shortcut for accessing the underlying SMINI sptr from a shd_smini_handle* */
#define SMINI(h_ptr) (get_smini_ptrs()[h_ptr->smini_index].ptr)
#define RX_STREAMER(h_ptr) (get_smini_ptrs()[h_ptr->smini_index].rx_streamers[h_ptr->streamer_index])
#define TX_STREAMER(h_ptr) (get_smini_ptrs()[h_ptr->smini_index].tx_streamers[h_ptr->streamer_index])

/****************************************************************************
 * RX Streamer
 ***************************************************************************/
static boost::mutex _rx_streamer_make_mutex;
shd_error shd_rx_streamer_make(shd_rx_streamer_handle* h){
    SHD_SAFE_C(
        boost::mutex::scoped_lock(_rx_streamer_make_mutex);
        (*h) = new shd_rx_streamer;
    )
}

static boost::mutex _rx_streamer_free_mutex;
shd_error shd_rx_streamer_free(shd_rx_streamer_handle* h){
    SHD_SAFE_C(
        boost::mutex::scoped_lock lock(_rx_streamer_free_mutex);
        delete (*h);
        (*h) = NULL;
    )
}

shd_error shd_rx_streamer_num_channels(shd_rx_streamer_handle h,
                                       size_t *num_channels_out){
    SHD_SAFE_C_SAVE_ERROR(h,
        *num_channels_out = RX_STREAMER(h)->get_num_channels();
    )
}

shd_error shd_rx_streamer_max_num_samps(shd_rx_streamer_handle h,
                                        size_t *max_num_samps_out){
    SHD_SAFE_C_SAVE_ERROR(h,
        *max_num_samps_out = RX_STREAMER(h)->get_max_num_samps();
    )
}

shd_error shd_rx_streamer_recv(
    shd_rx_streamer_handle h,
    void **buffs,
    size_t samps_per_buff,
    shd_rx_metadata_handle *md,
    double timeout,
    bool one_packet,
    size_t *items_recvd
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::rx_streamer::buffs_type buffs_cpp(buffs, RX_STREAMER(h)->get_num_channels());
        *items_recvd = RX_STREAMER(h)->recv(buffs_cpp, samps_per_buff, (*md)->rx_metadata_cpp, timeout, one_packet);
    )
}

shd_error shd_rx_streamer_issue_stream_cmd(
    shd_rx_streamer_handle h,
    const shd_stream_cmd_t *stream_cmd
){
    SHD_SAFE_C_SAVE_ERROR(h,
        RX_STREAMER(h)->issue_stream_cmd(stream_cmd_c_to_cpp(stream_cmd));
    )
}

shd_error shd_rx_streamer_last_error(
    shd_rx_streamer_handle h,
    char* error_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        memset(error_out, '\0', strbuffer_len);
        strncpy(error_out, h->last_error.c_str(), strbuffer_len);
    )
}

/****************************************************************************
 * TX Streamer
 ***************************************************************************/
static boost::mutex _tx_streamer_make_mutex;
shd_error shd_tx_streamer_make(
    shd_tx_streamer_handle* h
){
    SHD_SAFE_C(
        boost::mutex::scoped_lock lock(_tx_streamer_make_mutex);
        (*h) = new shd_tx_streamer;
    )
}

static boost::mutex _tx_streamer_free_mutex;
shd_error shd_tx_streamer_free(
    shd_tx_streamer_handle* h
){
    SHD_SAFE_C(
        boost::mutex::scoped_lock lock(_tx_streamer_free_mutex);
        delete *h;
        *h = NULL;
    )
}

shd_error shd_tx_streamer_num_channels(
    shd_tx_streamer_handle h,
    size_t *num_channels_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *num_channels_out = TX_STREAMER(h)->get_num_channels();
    )
}

shd_error shd_tx_streamer_max_num_samps(
    shd_tx_streamer_handle h,
    size_t *max_num_samps_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *max_num_samps_out = TX_STREAMER(h)->get_max_num_samps();
    )
}

shd_error shd_tx_streamer_send(
    shd_tx_streamer_handle h,
    const void **buffs,
    size_t samps_per_buff,
    shd_tx_metadata_handle *md,
    double timeout,
    size_t *items_sent
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::tx_streamer::buffs_type buffs_cpp(buffs, TX_STREAMER(h)->get_num_channels());
        *items_sent = TX_STREAMER(h)->send(
            buffs_cpp,
            samps_per_buff,
            (*md)->tx_metadata_cpp,
            timeout
        );
    )
}

shd_error shd_tx_streamer_recv_async_msg(
    shd_tx_streamer_handle h,
    shd_async_metadata_handle *md,
    const double timeout,
    bool *valid
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *valid = TX_STREAMER(h)->recv_async_msg((*md)->async_metadata_cpp, timeout);
    )
}

shd_error shd_tx_streamer_last_error(
    shd_tx_streamer_handle h,
    char* error_out,
    size_t strbuffer_len
){
    SHD_SAFE_C(
        memset(error_out, '\0', strbuffer_len);
        strncpy(error_out, h->last_error.c_str(), strbuffer_len);
    )
}

/****************************************************************************
 * Generate / Destroy API calls
 ***************************************************************************/
static boost::mutex _smini_find_mutex;
shd_error shd_smini_find(
    const char* args,
    shd_string_vector_handle *strings_out
){
    SHD_SAFE_C(
        boost::mutex::scoped_lock _lock(_smini_find_mutex);

        shd::device_addrs_t devs = shd::device::find(std::string(args), shd::device::SMINI);
        (*strings_out)->string_vector_cpp.clear();
        BOOST_FOREACH(const shd::device_addr_t &dev, devs){
            (*strings_out)->string_vector_cpp.push_back(dev.to_string());
        }
    )
}

static boost::mutex _smini_make_mutex;
shd_error shd_smini_make(
    shd_smini_handle *h,
    const char *args
){
    SHD_SAFE_C(
        boost::mutex::scoped_lock lock(_smini_make_mutex);

        size_t smini_count = smini_ptr::smini_counter;
        smini_ptr::smini_counter++;

        // Initialize SMINI
        shd::device_addr_t device_addr(args);
        smini_ptr P;
        P.ptr = shd::smini::multi_smini::make(device_addr);

        // Dump into registry
        get_smini_ptrs()[smini_count] = P;

        // Update handle
        (*h) = new shd_smini;
        (*h)->smini_index = smini_count;
    )
}

static boost::mutex _smini_free_mutex;
shd_error shd_smini_free(
    shd_smini_handle *h
){
    SHD_SAFE_C(
        boost::mutex::scoped_lock lock(_smini_free_mutex);

        if(!get_smini_ptrs().count((*h)->smini_index)){
            return SHD_ERROR_INVALID_DEVICE;
        }

        get_smini_ptrs().erase((*h)->smini_index);
        delete *h;
        *h = NULL;
    )
}

shd_error shd_smini_last_error(
    shd_smini_handle h,
    char* error_out,
    size_t strbuffer_len
){
    SHD_SAFE_C(
        memset(error_out, '\0', strbuffer_len);
        strncpy(error_out, h->last_error.c_str(), strbuffer_len);
    )
}

static boost::mutex _smini_get_rx_stream_mutex;
shd_error shd_smini_get_rx_stream(
    shd_smini_handle h_u,
    shd_stream_args_t *stream_args,
    shd_rx_streamer_handle h_s
){
    SHD_SAFE_C(
        boost::mutex::scoped_lock lock(_smini_get_rx_stream_mutex);

        if(!get_smini_ptrs().count(h_u->smini_index)){
            return SHD_ERROR_INVALID_DEVICE;
        }

        smini_ptr &smini = get_smini_ptrs()[h_u->smini_index];
        smini.rx_streamers.push_back(
            smini.ptr->get_rx_stream(stream_args_c_to_cpp(stream_args))
        );
        h_s->smini_index     = h_u->smini_index;
        h_s->streamer_index = smini.rx_streamers.size() - 1;
    )
}

static boost::mutex _smini_get_tx_stream_mutex;
shd_error shd_smini_get_tx_stream(
    shd_smini_handle h_u,
    shd_stream_args_t *stream_args,
    shd_tx_streamer_handle h_s
){
    SHD_SAFE_C(
        boost::mutex::scoped_lock lock(_smini_get_tx_stream_mutex);

        if(!get_smini_ptrs().count(h_u->smini_index)){
            return SHD_ERROR_INVALID_DEVICE;
        }

        smini_ptr &smini = get_smini_ptrs()[h_u->smini_index];
        smini.tx_streamers.push_back(
            smini.ptr->get_tx_stream(stream_args_c_to_cpp(stream_args))
        );
        h_s->smini_index     = h_u->smini_index;
        h_s->streamer_index = smini.tx_streamers.size() - 1;
    )
}

/****************************************************************************
 * multi_smini API calls
 ***************************************************************************/

#define COPY_INFO_FIELD(out, dict, field) \
    out->field = strdup(dict.get(BOOST_STRINGIZE(field)).c_str())

shd_error shd_smini_get_rx_info(
    shd_smini_handle h,
    size_t chan,
    shd_smini_rx_info_t *info_out
) {
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::dict<std::string, std::string> rx_info = SMINI(h)->get_smini_rx_info(chan);

        COPY_INFO_FIELD(info_out, rx_info, mboard_id);
        COPY_INFO_FIELD(info_out, rx_info, mboard_name);
        COPY_INFO_FIELD(info_out, rx_info, mboard_serial);
        COPY_INFO_FIELD(info_out, rx_info, rx_id);
        COPY_INFO_FIELD(info_out, rx_info, rx_subdev_name);
        COPY_INFO_FIELD(info_out, rx_info, rx_subdev_spec);
        COPY_INFO_FIELD(info_out, rx_info, rx_serial);
        COPY_INFO_FIELD(info_out, rx_info, rx_antenna);
    )
}

shd_error shd_smini_get_tx_info(
    shd_smini_handle h,
    size_t chan,
    shd_smini_tx_info_t *info_out
) {
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::dict<std::string, std::string> tx_info = SMINI(h)->get_smini_tx_info(chan);

        COPY_INFO_FIELD(info_out, tx_info, mboard_id);
        COPY_INFO_FIELD(info_out, tx_info, mboard_name);
        COPY_INFO_FIELD(info_out, tx_info, mboard_serial);
        COPY_INFO_FIELD(info_out, tx_info, tx_id);
        COPY_INFO_FIELD(info_out, tx_info, tx_subdev_name);
        COPY_INFO_FIELD(info_out, tx_info, tx_subdev_spec);
        COPY_INFO_FIELD(info_out, tx_info, tx_serial);
        COPY_INFO_FIELD(info_out, tx_info, tx_antenna);
    )
}

/****************************************************************************
 * Motherboard methods
 ***************************************************************************/
shd_error shd_smini_set_master_clock_rate(
    shd_smini_handle h,
    double rate,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_master_clock_rate(rate, mboard);
    )
}

shd_error shd_smini_get_master_clock_rate(
    shd_smini_handle h,
    size_t mboard,
    double *clock_rate_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *clock_rate_out = SMINI(h)->get_master_clock_rate(mboard);
    )
}

shd_error shd_smini_get_pp_string(
    shd_smini_handle h,
    char* pp_string_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        strncpy(pp_string_out, SMINI(h)->get_pp_string().c_str(), strbuffer_len);
    )
}

shd_error shd_smini_get_mboard_name(
    shd_smini_handle h,
    size_t mboard,
    char* mboard_name_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        strncpy(mboard_name_out, SMINI(h)->get_mboard_name(mboard).c_str(), strbuffer_len);
    )
}

shd_error shd_smini_get_time_now(
    shd_smini_handle h,
    size_t mboard,
    time_t *full_secs_out,
    double *frac_secs_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::time_spec_t time_spec_cpp = SMINI(h)->get_time_now(mboard);
        *full_secs_out = time_spec_cpp.get_full_secs();
        *frac_secs_out = time_spec_cpp.get_frac_secs();
    )
}

shd_error shd_smini_get_time_last_pps(
    shd_smini_handle h,
    size_t mboard,
    time_t *full_secs_out,
    double *frac_secs_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::time_spec_t time_spec_cpp = SMINI(h)->get_time_last_pps(mboard);
        *full_secs_out = time_spec_cpp.get_full_secs();
        *frac_secs_out = time_spec_cpp.get_frac_secs();
    )
}

shd_error shd_smini_set_time_now(
    shd_smini_handle h,
    time_t full_secs,
    double frac_secs,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::time_spec_t time_spec_cpp(full_secs, frac_secs);
        SMINI(h)->set_time_now(time_spec_cpp, mboard);
    )
}

shd_error shd_smini_set_time_next_pps(
    shd_smini_handle h,
    time_t full_secs,
    double frac_secs,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::time_spec_t time_spec_cpp(full_secs, frac_secs);
        SMINI(h)->set_time_next_pps(time_spec_cpp, mboard);
    )
}

shd_error shd_smini_set_time_unknown_pps(
    shd_smini_handle h,
    time_t full_secs,
    double frac_secs
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::time_spec_t time_spec_cpp(full_secs, frac_secs);
        SMINI(h)->set_time_unknown_pps(time_spec_cpp);
    )
}

shd_error shd_smini_get_time_synchronized(
    shd_smini_handle h,
    bool *result_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *result_out = SMINI(h)->get_time_synchronized();
        return SHD_ERROR_NONE;
    )
}

shd_error shd_smini_set_command_time(
    shd_smini_handle h,
    time_t full_secs,
    double frac_secs,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::time_spec_t time_spec_cpp(full_secs, frac_secs);
        SMINI(h)->set_command_time(time_spec_cpp, mboard);
    )
}

shd_error shd_smini_clear_command_time(
    shd_smini_handle h,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->clear_command_time(mboard);
    )
}

shd_error shd_smini_set_time_source(
    shd_smini_handle h,
    const char* time_source,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_time_source(std::string(time_source), mboard);
    )
}

shd_error shd_smini_get_time_source(
    shd_smini_handle h,
    size_t mboard,
    char* time_source_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        strncpy(time_source_out, SMINI(h)->get_time_source(mboard).c_str(), strbuffer_len);
    )
}

shd_error shd_smini_get_time_sources(
    shd_smini_handle h,
    size_t mboard,
    shd_string_vector_handle *time_sources_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        (*time_sources_out)->string_vector_cpp = SMINI(h)->get_time_sources(mboard);
    )
}

shd_error shd_smini_set_clock_source(
    shd_smini_handle h,
    const char* clock_source,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_clock_source(std::string(clock_source), mboard);
    )
}

shd_error shd_smini_get_clock_source(
    shd_smini_handle h,
    size_t mboard,
    char* clock_source_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        strncpy(clock_source_out, SMINI(h)->get_clock_source(mboard).c_str(), strbuffer_len);
    )
}

shd_error shd_smini_get_clock_sources(
    shd_smini_handle h,
    size_t mboard,
    shd_string_vector_handle *clock_sources_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        (*clock_sources_out)->string_vector_cpp = SMINI(h)->get_clock_sources(mboard);
    )
}

shd_error shd_smini_set_clock_source_out(
    shd_smini_handle h,
    bool enb,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_clock_source_out(enb, mboard);
    )
}

shd_error shd_smini_set_time_source_out(
    shd_smini_handle h,
    bool enb,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_time_source_out(enb, mboard);
    )
}

shd_error shd_smini_get_num_mboards(
    shd_smini_handle h,
    size_t *num_mboards_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *num_mboards_out = SMINI(h)->get_num_mboards();
    )
}

shd_error shd_smini_get_mboard_sensor(
    shd_smini_handle h,
    const char* name,
    size_t mboard,
    shd_sensor_value_handle *sensor_value_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        delete (*sensor_value_out)->sensor_value_cpp;
        (*sensor_value_out)->sensor_value_cpp = new shd::sensor_value_t(SMINI(h)->get_mboard_sensor(name, mboard));
    )
}

shd_error shd_smini_get_mboard_sensor_names(
    shd_smini_handle h,
    size_t mboard,
    shd_string_vector_handle *mboard_sensor_names_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        (*mboard_sensor_names_out)->string_vector_cpp = SMINI(h)->get_mboard_sensor_names(mboard);
    )
}

shd_error shd_smini_set_user_register(
    shd_smini_handle h,
    uint8_t addr,
    uint32_t data,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_user_register(addr, data, mboard);
    )
}

/****************************************************************************
 * EEPROM access methods
 ***************************************************************************/

shd_error shd_smini_get_mboard_eeprom(
    shd_smini_handle h,
    shd_mboard_eeprom_handle mb_eeprom,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::fs_path eeprom_path = str(boost::format("/mboards/%d/eeprom")
                                       % mboard);

        shd::property_tree::sptr ptree = SMINI(h)->get_device()->get_tree();
        mb_eeprom->mboard_eeprom_cpp = ptree->access<shd::smini::mboard_eeprom_t>(eeprom_path).get();
    )
}

shd_error shd_smini_set_mboard_eeprom(
    shd_smini_handle h,
    shd_mboard_eeprom_handle mb_eeprom,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::fs_path eeprom_path = str(boost::format("/mboards/%d/eeprom")
                                       % mboard);

        shd::property_tree::sptr ptree = SMINI(h)->get_device()->get_tree();
        ptree->access<shd::smini::mboard_eeprom_t>(eeprom_path).set(mb_eeprom->mboard_eeprom_cpp);
    )
}

shd_error shd_smini_get_dboard_eeprom(
    shd_smini_handle h,
    shd_dboard_eeprom_handle db_eeprom,
    const char* unit,
    const char* slot,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::fs_path eeprom_path = str(boost::format("/mboards/%d/dboards/%s/%s_eeprom")
                                       % mboard % slot % unit);

        shd::property_tree::sptr ptree = SMINI(h)->get_device()->get_tree();
        db_eeprom->dboard_eeprom_cpp = ptree->access<shd::smini::dboard_eeprom_t>(eeprom_path).get();
    )
}

shd_error shd_smini_set_dboard_eeprom(
    shd_smini_handle h,
    shd_dboard_eeprom_handle db_eeprom,
    const char* unit,
    const char* slot,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::fs_path eeprom_path = str(boost::format("/mboards/%d/dboards/%s/%s_eeprom")
                                       % mboard % slot % unit);

        shd::property_tree::sptr ptree = SMINI(h)->get_device()->get_tree();
        ptree->access<shd::smini::dboard_eeprom_t>(eeprom_path).set(db_eeprom->dboard_eeprom_cpp);
    )
}

/****************************************************************************
 * RX methods
 ***************************************************************************/

shd_error shd_smini_set_rx_subdev_spec(
    shd_smini_handle h,
    shd_subdev_spec_handle subdev_spec,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_rx_subdev_spec(subdev_spec->subdev_spec_cpp, mboard);
    )
}

shd_error shd_smini_get_rx_subdev_spec(
    shd_smini_handle h,
    size_t mboard,
    shd_subdev_spec_handle subdev_spec_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        subdev_spec_out->subdev_spec_cpp = SMINI(h)->get_rx_subdev_spec(mboard);
    )
}

shd_error shd_smini_get_rx_num_channels(
    shd_smini_handle h,
    size_t *num_channels_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *num_channels_out = SMINI(h)->get_rx_num_channels();
    )
}

shd_error shd_smini_get_rx_subdev_name(
    shd_smini_handle h,
    size_t chan,
    char* rx_subdev_name_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string rx_subdev_name = SMINI(h)->get_rx_subdev_name(chan);
        strncpy(rx_subdev_name_out, rx_subdev_name.c_str(), strbuffer_len);
    )
}

shd_error shd_smini_set_rx_rate(
    shd_smini_handle h,
    double rate,
    size_t chan
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_rx_rate(rate, chan);
    )
}

shd_error shd_smini_get_rx_rate(
    shd_smini_handle h,
    size_t chan,
    double *rate_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *rate_out = SMINI(h)->get_rx_rate(chan);
    )
}

shd_error shd_smini_get_rx_rates(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle rates_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        rates_out->meta_range_cpp = SMINI(h)->get_rx_rates(chan);
    )
}

shd_error shd_smini_set_rx_freq(
    shd_smini_handle h,
    shd_tune_request_t *tune_request,
    size_t chan,
    shd_tune_result_t *tune_result
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::tune_request_t tune_request_cpp = shd_tune_request_c_to_cpp(tune_request);
        shd::tune_result_t tune_result_cpp = SMINI(h)->set_rx_freq(tune_request_cpp, chan);
        shd_tune_result_cpp_to_c(tune_result_cpp, tune_result);
    )
}

shd_error shd_smini_get_rx_freq(
    shd_smini_handle h,
    size_t chan,
    double *freq_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *freq_out = SMINI(h)->get_rx_freq(chan);
    )
}

shd_error shd_smini_get_rx_freq_range(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle freq_range_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        freq_range_out->meta_range_cpp = SMINI(h)->get_rx_freq_range(chan);
    )
}

shd_error shd_smini_get_fe_rx_freq_range(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle freq_range_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        freq_range_out->meta_range_cpp = SMINI(h)->get_fe_rx_freq_range(chan);
    )
}

SHD_API shd_error shd_smini_get_rx_lo_names(
    shd_smini_handle h,
    size_t chan,
    shd_string_vector_handle *rx_lo_names_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        (*rx_lo_names_out)->string_vector_cpp = SMINI(h)->get_rx_lo_names(chan);
    )
}

SHD_API shd_error shd_smini_set_rx_lo_source(
    shd_smini_handle h,
    const char* src,
    const char* name,
    size_t chan
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_rx_lo_source(src, name, chan);
    )
}

SHD_API shd_error shd_smini_get_rx_lo_source(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    char* rx_lo_source_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        strncpy(rx_lo_source_out, SMINI(h)->get_rx_lo_source(name, chan).c_str(), strbuffer_len);
    )
}

SHD_API shd_error shd_smini_get_rx_lo_sources(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    shd_string_vector_handle *rx_lo_sources_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        (*rx_lo_sources_out)->string_vector_cpp = SMINI(h)->get_rx_lo_sources(name, chan);
    )
}

SHD_API shd_error shd_smini_set_rx_lo_export_enabled(
    shd_smini_handle h,
    bool enabled,
    const char* name,
    size_t chan
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_rx_lo_export_enabled(enabled, name, chan);
    )
}

SHD_API shd_error shd_smini_get_rx_lo_export_enabled(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    bool* result_out
) {
    SHD_SAFE_C_SAVE_ERROR(h,
        *result_out = SMINI(h)->get_rx_lo_export_enabled(name, chan);
    )
}

SHD_API shd_error shd_smini_set_rx_lo_freq(
    shd_smini_handle h,
    double freq,
    const char* name,
    size_t chan,
    double* coerced_freq_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *coerced_freq_out = SMINI(h)->set_rx_lo_freq(freq, name, chan);
    )
}

SHD_API shd_error shd_smini_get_rx_lo_freq(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    double* rx_lo_freq_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *rx_lo_freq_out = SMINI(h)->get_rx_lo_freq(name, chan);
    )
}

shd_error shd_smini_set_rx_gain(
    shd_smini_handle h,
    double gain,
    size_t chan,
    const char *gain_name
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string name(gain_name);
        if(name.empty()){
            SMINI(h)->set_rx_gain(gain, chan);
        }
        else{
            SMINI(h)->set_rx_gain(gain, name, chan);
        }
    )
}

shd_error shd_smini_set_normalized_rx_gain(
    shd_smini_handle h,
    double gain,
    size_t chan
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_normalized_rx_gain(gain, chan);
    )
}

shd_error shd_smini_set_rx_agc(
    shd_smini_handle h,
    bool enable,
    size_t chan
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_rx_agc(enable, chan);
    )
}

shd_error shd_smini_get_rx_gain(
    shd_smini_handle h,
    size_t chan,
    const char *gain_name,
    double *gain_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string name(gain_name);
        if(name.empty()){
            *gain_out = SMINI(h)->get_rx_gain(chan);
        }
        else{
            *gain_out = SMINI(h)->get_rx_gain(name, chan);
        }
    )
}

shd_error shd_smini_get_normalized_rx_gain(
    shd_smini_handle h,
    size_t chan,
    double *gain_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *gain_out = SMINI(h)->get_normalized_rx_gain(chan);
    )
}

shd_error shd_smini_get_rx_gain_range(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    shd_meta_range_handle gain_range_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        gain_range_out->meta_range_cpp = SMINI(h)->get_rx_gain_range(name, chan);
    )
}

shd_error shd_smini_get_rx_gain_names(
    shd_smini_handle h,
    size_t chan,
    shd_string_vector_handle *gain_names_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        (*gain_names_out)->string_vector_cpp = SMINI(h)->get_rx_gain_names(chan);
    )
}

shd_error shd_smini_set_rx_antenna(
    shd_smini_handle h,
    const char* ant,
    size_t chan
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_rx_antenna(std::string(ant), chan);
    )
}

shd_error shd_smini_get_rx_antenna(
    shd_smini_handle h,
    size_t chan,
    char* ant_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string rx_antenna = SMINI(h)->get_rx_antenna(chan);
        strncpy(ant_out, rx_antenna.c_str(), strbuffer_len);
    )
}

shd_error shd_smini_get_rx_antennas(
    shd_smini_handle h,
    size_t chan,
    shd_string_vector_handle *antennas_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        (*antennas_out)->string_vector_cpp = SMINI(h)->get_rx_antennas(chan);
    )
}

shd_error shd_smini_set_rx_bandwidth(
    shd_smini_handle h,
    double bandwidth,
    size_t chan
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_rx_bandwidth(bandwidth, chan);
    )
}

shd_error shd_smini_get_rx_bandwidth(
    shd_smini_handle h,
    size_t chan,
    double *bandwidth_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *bandwidth_out = SMINI(h)->get_rx_bandwidth(chan);
    )
}

shd_error shd_smini_get_rx_bandwidth_range(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle bandwidth_range_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        bandwidth_range_out->meta_range_cpp = SMINI(h)->get_rx_bandwidth_range(chan);
    )
}

shd_error shd_smini_get_rx_sensor(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    shd_sensor_value_handle *sensor_value_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        delete (*sensor_value_out)->sensor_value_cpp;
        (*sensor_value_out)->sensor_value_cpp = new shd::sensor_value_t(SMINI(h)->get_rx_sensor(name, chan));
    )
}

shd_error shd_smini_get_rx_sensor_names(
    shd_smini_handle h,
    size_t chan,
    shd_string_vector_handle *sensor_names_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        (*sensor_names_out)->string_vector_cpp = SMINI(h)->get_rx_sensor_names(chan);
    )
}

shd_error shd_smini_set_rx_dc_offset_enabled(
    shd_smini_handle h,
    bool enb,
    size_t chan
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_rx_dc_offset(enb, chan);
    )
}

shd_error shd_smini_set_rx_iq_balance_enabled(
    shd_smini_handle h,
    bool enb,
    size_t chan
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_rx_iq_balance(enb, chan);
    )
}

/****************************************************************************
 * TX methods
 ***************************************************************************/

shd_error shd_smini_set_tx_subdev_spec(
    shd_smini_handle h,
    shd_subdev_spec_handle subdev_spec,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_tx_subdev_spec(subdev_spec->subdev_spec_cpp, mboard);
    )
}

shd_error shd_smini_get_tx_subdev_spec(
    shd_smini_handle h,
    size_t mboard,
    shd_subdev_spec_handle subdev_spec_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        subdev_spec_out->subdev_spec_cpp = SMINI(h)->get_tx_subdev_spec(mboard);
    )
}


shd_error shd_smini_get_tx_num_channels(
    shd_smini_handle h,
    size_t *num_channels_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *num_channels_out = SMINI(h)->get_tx_num_channels();
    )
}

shd_error shd_smini_get_tx_subdev_name(
    shd_smini_handle h,
    size_t chan,
    char* tx_subdev_name_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string tx_subdev_name = SMINI(h)->get_tx_subdev_name(chan);
        strncpy(tx_subdev_name_out, tx_subdev_name.c_str(), strbuffer_len);
    )
}

shd_error shd_smini_set_tx_rate(
    shd_smini_handle h,
    double rate,
    size_t chan
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_tx_rate(rate, chan);
    )
}

shd_error shd_smini_get_tx_rate(
    shd_smini_handle h,
    size_t chan,
    double *rate_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *rate_out = SMINI(h)->get_tx_rate(chan);
    )
}

shd_error shd_smini_get_tx_rates(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle rates_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        rates_out->meta_range_cpp = SMINI(h)->get_tx_rates(chan);
    )
}

shd_error shd_smini_set_tx_freq(
    shd_smini_handle h,
    shd_tune_request_t *tune_request,
    size_t chan,
    shd_tune_result_t *tune_result
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::tune_request_t tune_request_cpp = shd_tune_request_c_to_cpp(tune_request);
        shd::tune_result_t tune_result_cpp = SMINI(h)->set_tx_freq(tune_request_cpp, chan);
        shd_tune_result_cpp_to_c(tune_result_cpp, tune_result);
    )
}

shd_error shd_smini_get_tx_freq(
    shd_smini_handle h,
    size_t chan,
    double *freq_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *freq_out = SMINI(h)->get_tx_freq(chan);
    )
}

shd_error shd_smini_get_tx_freq_range(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle freq_range_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        freq_range_out->meta_range_cpp = SMINI(h)->get_tx_freq_range(chan);
    )
}

shd_error shd_smini_get_fe_tx_freq_range(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle freq_range_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        freq_range_out->meta_range_cpp = SMINI(h)->get_fe_tx_freq_range(chan);
    )
}

shd_error shd_smini_set_tx_gain(
    shd_smini_handle h,
    double gain,
    size_t chan,
    const char *gain_name
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string name(gain_name);
        if(name.empty()){
            SMINI(h)->set_tx_gain(gain, chan);
        }
        else{
            SMINI(h)->set_tx_gain(gain, name, chan);
        }
    )
}

shd_error shd_smini_set_normalized_tx_gain(
    shd_smini_handle h,
    double gain,
    size_t chan
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_normalized_tx_gain(gain, chan);
    )
}

shd_error shd_smini_get_tx_gain(
    shd_smini_handle h,
    size_t chan,
    const char *gain_name,
    double *gain_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string name(gain_name);
        if(name.empty()){
            *gain_out = SMINI(h)->get_tx_gain(chan);
        }
        else{
            *gain_out = SMINI(h)->get_tx_gain(name, chan);
        }
    )
}

shd_error shd_smini_get_normalized_tx_gain(
    shd_smini_handle h,
    size_t chan,
    double *gain_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *gain_out = SMINI(h)->get_normalized_tx_gain(chan);
    )
}

shd_error shd_smini_get_tx_gain_range(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    shd_meta_range_handle gain_range_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        gain_range_out->meta_range_cpp = SMINI(h)->get_tx_gain_range(name, chan);
    )
}

shd_error shd_smini_get_tx_gain_names(
    shd_smini_handle h,
    size_t chan,
    shd_string_vector_handle *gain_names_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        (*gain_names_out)->string_vector_cpp = SMINI(h)->get_tx_gain_names(chan);
    )
}

shd_error shd_smini_set_tx_antenna(
    shd_smini_handle h,
    const char* ant,
    size_t chan
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_tx_antenna(std::string(ant), chan);
    )
}

shd_error shd_smini_get_tx_antenna(
    shd_smini_handle h,
    size_t chan,
    char* ant_out,
    size_t strbuffer_len
){
    SHD_SAFE_C_SAVE_ERROR(h,
        std::string tx_antenna = SMINI(h)->get_tx_antenna(chan);
        strncpy(ant_out, tx_antenna.c_str(), strbuffer_len);
    )
}

shd_error shd_smini_get_tx_antennas(
    shd_smini_handle h,
    size_t chan,
    shd_string_vector_handle *antennas_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        (*antennas_out)->string_vector_cpp = SMINI(h)->get_tx_antennas(chan);
    )
}

shd_error shd_smini_set_tx_bandwidth(
    shd_smini_handle h,
    double bandwidth,
    size_t chan
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_tx_bandwidth(bandwidth, chan);
    )
}

shd_error shd_smini_get_tx_bandwidth(
    shd_smini_handle h,
    size_t chan,
    double *bandwidth_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *bandwidth_out = SMINI(h)->get_tx_bandwidth(chan);
    )
}

shd_error shd_smini_get_tx_bandwidth_range(
    shd_smini_handle h,
    size_t chan,
    shd_meta_range_handle bandwidth_range_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        bandwidth_range_out->meta_range_cpp = SMINI(h)->get_tx_bandwidth_range(chan);
    )
}

shd_error shd_smini_get_tx_sensor(
    shd_smini_handle h,
    const char* name,
    size_t chan,
    shd_sensor_value_handle *sensor_value_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        delete (*sensor_value_out)->sensor_value_cpp;
        (*sensor_value_out)->sensor_value_cpp = new shd::sensor_value_t(SMINI(h)->get_tx_sensor(name, chan));
    )
}

shd_error shd_smini_get_tx_sensor_names(
    shd_smini_handle h,
    size_t chan,
    shd_string_vector_handle *sensor_names_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        (*sensor_names_out)->string_vector_cpp = SMINI(h)->get_tx_sensor_names(chan);
    )
}

shd_error shd_smini_set_tx_dc_offset_enabled(
    shd_smini_handle h,
    bool enb,
    size_t chan
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_tx_dc_offset(enb, chan);
    )
}

shd_error shd_smini_set_tx_iq_balance_enabled(
    shd_smini_handle h,
    bool enb,
    size_t chan
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_tx_iq_balance(enb, chan);
    )
}

/****************************************************************************
 * GPIO methods
 ***************************************************************************/

shd_error shd_smini_get_gpio_banks(
    shd_smini_handle h,
    size_t chan,
    shd_string_vector_handle *gpio_banks_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        (*gpio_banks_out)->string_vector_cpp = SMINI(h)->get_gpio_banks(chan);
    )
}

shd_error shd_smini_set_gpio_attr(
    shd_smini_handle h,
    const char* bank,
    const char* attr,
    uint32_t value,
    uint32_t mask,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->set_gpio_attr(std::string(bank), std::string(attr),
                               value, mask, mboard);
    )
}

shd_error shd_smini_get_gpio_attr(
    shd_smini_handle h,
    const char* bank,
    const char* attr,
    size_t mboard,
    uint32_t *attr_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *attr_out = SMINI(h)->get_gpio_attr(std::string(bank), std::string(attr), mboard);
    )
}

shd_error shd_smini_enumerate_registers(
    shd_smini_handle h,
    size_t mboard,
    shd_string_vector_handle *registers_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        (*registers_out)->string_vector_cpp = SMINI(h)->enumerate_registers(mboard);
    )
}

shd_error shd_smini_get_register_info(
    shd_smini_handle h,
    const char* path,
    size_t mboard,
    shd_smini_register_info_t *register_info_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        shd::smini::multi_smini::register_info_t register_info_cpp = SMINI(h)->get_register_info(path, mboard);
        register_info_out->bitwidth = register_info_cpp.bitwidth;
        register_info_out->readable = register_info_cpp.readable;
        register_info_out->writable = register_info_cpp.writable;
    )
}

shd_error shd_smini_write_register(
    shd_smini_handle h,
    const char* path,
    uint32_t field,
    uint64_t value,
    size_t mboard
){
    SHD_SAFE_C_SAVE_ERROR(h,
        SMINI(h)->write_register(path, field, value, mboard);
    )
}

shd_error shd_smini_read_register(
    shd_smini_handle h,
    const char* path,
    uint32_t field,
    size_t mboard,
    uint64_t *value_out
){
    SHD_SAFE_C_SAVE_ERROR(h,
        *value_out = SMINI(h)->read_register(path, field, mboard);
    )
}
