//
// Copyright 2014,2016 Ettus Research LLC
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

#ifndef INCLUDED_OCTOCLOCK_IMPL_HPP
#define INCLUDED_OCTOCLOCK_IMPL_HPP

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <shd/device.hpp>
#include <shd/stream.hpp>
#include <shd/smini/gps_ctrl.hpp>
#include <shd/smini_clock/octoclock_eeprom.hpp>
#include <shd/types/device_addr.hpp>
#include <shd/types/dict.hpp>
#include <shd/types/sensors.hpp>

#include "common.h"

shd::device_addrs_t octoclock_find(const shd::device_addr_t &hint);

/*!
 * OctoClock implementation guts
 */
class octoclock_impl : public shd::device{
public:
    octoclock_impl(const shd::device_addr_t &);
    ~octoclock_impl(void) {};

    shd::rx_streamer::sptr get_rx_stream(const shd::stream_args_t &args);

    shd::tx_streamer::sptr get_tx_stream(const shd::stream_args_t &args);

    bool recv_async_msg(shd::async_metadata_t&, double);

private:
    struct oc_container_type{
        shd::smini_clock::octoclock_eeprom_t eeprom;
        octoclock_state_t state;
        shd::transport::udp_simple::sptr ctrl_xport;
        shd::transport::udp_simple::sptr gpsdo_xport;
        shd::gps_ctrl::sptr gps;
    };
    shd::dict<std::string, oc_container_type> _oc_dict;
    uint32_t _sequence;
	uint32_t _proto_ver;

    void _set_eeprom(const std::string &oc, const shd::smini_clock::octoclock_eeprom_t &oc_eeprom);

    uint32_t _get_fw_version(const std::string &oc);

    void _get_state(const std::string &oc);

    shd::sensor_value_t _ext_ref_detected(const std::string &oc);

    shd::sensor_value_t _gps_detected(const std::string &oc);

    shd::sensor_value_t _which_ref(const std::string &oc);

    shd::sensor_value_t _switch_pos(const std::string &oc);

    uint32_t _get_time(const std::string &oc);

    std::string _get_images_help_message(const std::string &addr);

    boost::mutex _device_mutex;
};

#endif /* INCLUDED_OCTOCLOCK_IMPL_HPP */
