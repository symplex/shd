//
// Copyright 2010-2012,2014 Ettus Research LLC
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

#ifndef INCLUDED_SMINI2_IMPL_HPP
#define INCLUDED_SMINI2_IMPL_HPP

#include "gpio_core_200.hpp"
#include "smini2_iface.hpp"
#include "smini2_fifo_ctrl.hpp"
#include "clock_ctrl.hpp"
#include "codec_ctrl.hpp"
#include "rx_frontend_core_200.hpp"
#include "tx_frontend_core_200.hpp"
#include "rx_dsp_core_200.hpp"
#include "tx_dsp_core_200.hpp"
#include "time64_core_200.hpp"
#include "user_settings_core_200.hpp"
#include <shd/property_tree.hpp>
#include <shd/smini/gps_ctrl.hpp>
#include <shd/device.hpp>
#include <shd/utils/pimpl.hpp>
#include <shd/types/dict.hpp>
#include <shd/types/stream_cmd.hpp>
#include <shd/types/clock_config.hpp>
#include <shd/smini/dboard_eeprom.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <shd/transport/vrt_if_packet.hpp>
#include <shd/transport/udp_simple.hpp>
#include <shd/transport/udp_zero_copy.hpp>
#include <shd/types/device_addr.hpp>
#include <shd/smini/dboard_manager.hpp>
#include <shd/smini/subdev_spec.hpp>
#include <boost/weak_ptr.hpp>

static const double SMINI2_LINK_RATE_BPS = 1000e6/8;
static const double mimo_clock_delay_smini2_rev4 = 4.18e-9;
static const double mimo_clock_delay_smini_n2xx = 4.10e-9;
static const size_t mimo_clock_sync_delay_cycles = 138;
static const size_t SMINI2_SRAM_BYTES = size_t(1 << 20);
static const uint32_t SMINI2_TX_ASYNC_SID = 2;
static const uint32_t SMINI2_RX_SID_BASE = 3;
static const std::string SMINI2_EEPROM_MAP_KEY = "N100";

shd::device_addrs_t smini2_find(const shd::device_addr_t &hint_);

//! Make a smini2 dboard interface.
shd::smini::dboard_iface::sptr make_smini2_dboard_iface(
    shd::timed_wb_iface::sptr wb_iface,
    shd::i2c_iface::sptr i2c_iface,
    shd::spi_iface::sptr spi_iface,
    smini2_clock_ctrl::sptr clk_ctrl
);

/*!
 * SMINI2 implementation guts:
 * The implementation details are encapsulated here.
 * Handles device properties and streaming...
 */
class smini2_impl : public shd::device{
public:
    smini2_impl(const shd::device_addr_t &);
    ~smini2_impl(void);

    //the io interface
    shd::rx_streamer::sptr get_rx_stream(const shd::stream_args_t &args);
    shd::tx_streamer::sptr get_tx_stream(const shd::stream_args_t &args);
    bool recv_async_msg(shd::async_metadata_t &, double);

private:
    struct mb_container_type{
        smini2_iface::sptr iface;
        smini2_fifo_ctrl::sptr fifo_ctrl;
        shd::spi_iface::sptr spiface;
        shd::timed_wb_iface::sptr wbiface;
        smini2_clock_ctrl::sptr clock;
        smini2_codec_ctrl::sptr codec;
        shd::gps_ctrl::sptr gps;
        rx_frontend_core_200::sptr rx_fe;
        tx_frontend_core_200::sptr tx_fe;
        std::vector<rx_dsp_core_200::sptr> rx_dsps;
        std::vector<boost::weak_ptr<shd::rx_streamer> > rx_streamers;
        std::vector<boost::weak_ptr<shd::tx_streamer> > tx_streamers;
        tx_dsp_core_200::sptr tx_dsp;
        time64_core_200::sptr time64;
        user_settings_core_200::sptr user;
        std::vector<shd::transport::zero_copy_if::sptr> rx_dsp_xports;
        shd::transport::zero_copy_if::sptr tx_dsp_xport;
        shd::transport::zero_copy_if::sptr fifo_ctrl_xport;
        shd::smini::dboard_manager::sptr dboard_manager;
        size_t rx_chan_occ, tx_chan_occ;
        mb_container_type(void): rx_chan_occ(0), tx_chan_occ(0){}
    };
    shd::dict<std::string, mb_container_type> _mbc;

    void set_mb_eeprom(const std::string &, const shd::smini::mboard_eeprom_t &);
    void set_db_eeprom(const std::string &, const std::string &, const shd::smini::dboard_eeprom_t &);

    shd::sensor_value_t get_mimo_locked(const std::string &);
    shd::sensor_value_t get_ref_locked(const std::string &);

    void set_rx_fe_corrections(const std::string &mb, const double);
    void set_tx_fe_corrections(const std::string &mb, const double);
    bool _ignore_cal_file;

    //io impl methods and members
    shd::device_addr_t device_addr;
    SHD_PIMPL_DECL(io_impl) _io_impl;
    void io_init(void);
    void update_tick_rate(const double rate);
    void update_rx_samp_rate(const std::string &, const size_t, const double rate);
    void update_tx_samp_rate(const std::string &, const size_t, const double rate);
    void update_rates(void);
    //update spec methods are coercers until we only accept db_name == A
    void update_rx_subdev_spec(const std::string &, const shd::smini::subdev_spec_t &);
    void update_tx_subdev_spec(const std::string &, const shd::smini::subdev_spec_t &);
    double set_tx_dsp_freq(const std::string &, const double);
    shd::meta_range_t get_tx_dsp_freq_range(const std::string &);
    void update_clock_source(const std::string &, const std::string &);
    void program_stream_dest(shd::transport::zero_copy_if::sptr &, const shd::stream_args_t &);
};

#endif /* INCLUDED_SMINI2_IMPL_HPP */
