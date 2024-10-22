//
// Copyright 2010-2012,2014-2015 Ettus Research LLC
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

#include "e100_ctrl.hpp"
#include "clock_ctrl.hpp"
#include "codec_ctrl.hpp"
#include "i2c_core_200.hpp"
#include "rx_frontend_core_200.hpp"
#include "tx_frontend_core_200.hpp"
#include "rx_dsp_core_200.hpp"
#include "tx_dsp_core_200.hpp"
#include "time64_core_200.hpp"
#include "fifo_ctrl_excelsior.hpp"
#include "user_settings_core_200.hpp"
#include "recv_packet_demuxer.hpp"
#include <shd/device.hpp>
#include <shd/property_tree.hpp>
#include <shd/types/device_addr.hpp>
#include <shd/smini/subdev_spec.hpp>
#include <shd/smini/dboard_eeprom.hpp>
#include <shd/smini/mboard_eeprom.hpp>
#include <shd/smini/gps_ctrl.hpp>
#include <shd/types/sensors.hpp>
#include <shd/types/stream_cmd.hpp>
#include <shd/smini/dboard_manager.hpp>
#include <shd/transport/zero_copy.hpp>
#include <boost/weak_ptr.hpp>

#ifndef INCLUDED_E100_IMPL_HPP
#define INCLUDED_E100_IMPL_HPP

shd::transport::zero_copy_if::sptr e100_make_mmap_zero_copy(e100_ctrl::sptr iface);

// = gpmc_clock_rate/clk_div/cycles_per_transaction*bytes_per_transaction
static const double          E100_RX_LINK_RATE_BPS = 166e6/3/2*2;
static const double          E100_TX_LINK_RATE_BPS = 166e6/3/1*2;
static const std::string     E100_I2C_DEV_NODE = "/dev/i2c-3";
static const std::string     E100_UART_DEV_NODE = "/dev/ttyO0";
static const uint16_t E100_FPGA_COMPAT_NUM = 11;
static const uint32_t E100_RX_SID_BASE = 30;
static const uint32_t E100_TX_ASYNC_SID = 10;
static const uint32_t E100_CTRL_MSG_SID = 20;
static const double          E100_DEFAULT_CLOCK_RATE = 64e6;
static const std::string     E100_EEPROM_MAP_KEY = "E100";

//! load an fpga image from a bin file into the smini-e fpga
extern void e100_load_fpga(const std::string &bin_file);

//! Make an e100 dboard interface
shd::smini::dboard_iface::sptr make_e100_dboard_iface(
    shd::timed_wb_iface::sptr wb_iface,
    shd::i2c_iface::sptr i2c_iface,
    shd::spi_iface::sptr spi_iface,
    e100_clock_ctrl::sptr clock,
    e100_codec_ctrl::sptr codec
);

shd::device_addrs_t e100_find(const shd::device_addr_t &hint);
std::string get_default_e1x0_fpga_image(const shd::device_addr_t &device_addr);

/*!
 * SMINI-E100 implementation guts:
 * The implementation details are encapsulated here.
 * Handles properties on the mboard, dboard, dsps...
 */
class e100_impl : public shd::device{
public:
    //structors
    e100_impl(const shd::device_addr_t &);
    ~e100_impl(void);

    //the io interface
    shd::rx_streamer::sptr get_rx_stream(const shd::stream_args_t &args);
    shd::tx_streamer::sptr get_tx_stream(const shd::stream_args_t &args);
    bool recv_async_msg(shd::async_metadata_t &, double);

private:
    //controllers
    fifo_ctrl_excelsior::sptr _fifo_ctrl;
    i2c_core_200::sptr _fpga_i2c_ctrl;
    rx_frontend_core_200::sptr _rx_fe;
    tx_frontend_core_200::sptr _tx_fe;
    std::vector<rx_dsp_core_200::sptr> _rx_dsps;
    tx_dsp_core_200::sptr _tx_dsp;
    time64_core_200::sptr _time64;
    user_settings_core_200::sptr _user;
    e100_clock_ctrl::sptr _clock_ctrl;
    e100_codec_ctrl::sptr _codec_ctrl;
    e100_ctrl::sptr _fpga_ctrl;
    shd::i2c_iface::sptr _dev_i2c_iface;
    shd::spi_iface::sptr _aux_spi_iface;
    shd::gps_ctrl::sptr _gps;

    //transports
    shd::transport::zero_copy_if::sptr _data_transport;
    shd::smini::recv_packet_demuxer::sptr _recv_demuxer;

    //dboard stuff
    shd::smini::dboard_manager::sptr _dboard_manager;
    bool _ignore_cal_file;

    std::vector<boost::weak_ptr<shd::rx_streamer> > _rx_streamers;
    std::vector<boost::weak_ptr<shd::tx_streamer> > _tx_streamers;

    double update_rx_codec_gain(const double); //sets A and B at once
    void set_mb_eeprom(const shd::smini::mboard_eeprom_t &);
    void set_db_eeprom(const std::string &, const shd::smini::dboard_eeprom_t &);
    void update_tick_rate(const double rate);
    void update_rx_samp_rate(const size_t, const double rate);
    void update_tx_samp_rate(const size_t, const double rate);
    void update_rates(void);
    void update_rx_subdev_spec(const shd::smini::subdev_spec_t &);
    void update_tx_subdev_spec(const shd::smini::subdev_spec_t &);
    void update_clock_source(const std::string &);
    shd::sensor_value_t get_ref_locked(void);
    void check_fpga_compat(void);
    void set_rx_fe_corrections(const double);
    void set_tx_fe_corrections(const double);

};

#endif /* INCLUDED_E100_IMPL_HPP */
