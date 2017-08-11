//
// Copyright 2014 Ettus Research LLC
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

#ifndef INCLUDED_N230_IMPL_HPP
#define INCLUDED_N230_IMPL_HPP

#include <shd/property_tree.hpp>
#include <shd/device.hpp>
#include <shd/smini/subdev_spec.hpp>

#include "n230_device_args.hpp"
#include "n230_eeprom_manager.hpp"
#include "n230_resource_manager.hpp"
#include "n230_stream_manager.hpp"
#include "recv_packet_demuxer_3000.hpp"

namespace shd { namespace smini { namespace n230 {

class n230_impl : public shd::device
{
public: //Functions
    // ctor and dtor
    n230_impl(const shd::device_addr_t& device_addr);
    virtual ~n230_impl(void);

    //---------------------------------------------------------------------
    // shd::device interface
    //
    static sptr make(const shd::device_addr_t &hint, size_t which = 0);

    //! Make a new receive streamer from the streamer arguments
    virtual shd::rx_streamer::sptr get_rx_stream(const shd::stream_args_t &args);

    //! Make a new transmit streamer from the streamer arguments
    virtual shd::tx_streamer::sptr get_tx_stream(const shd::stream_args_t &args);

    //!Receive and asynchronous message from the device.
    virtual bool recv_async_msg(shd::async_metadata_t &async_metadata, double timeout = 0.1);

    //!Registration methods the discovery and factory system.
    //[static void register_device(const find_t &find, const make_t &make)]
    static shd::device_addrs_t n230_find(const shd::device_addr_t &hint);
    static shd::device::sptr n230_make(const shd::device_addr_t &device_addr);
    //
    //---------------------------------------------------------------------

    typedef shd::transport::bounded_buffer<shd::async_metadata_t> async_md_type;

private:    //Functions
    void _initialize_property_tree(const fs_path& mb_path);
    void _initialize_radio_properties(const fs_path& mb_path, size_t instance);

    void _update_rx_subdev_spec(const shd::smini::subdev_spec_t &);
    void _update_tx_subdev_spec(const shd::smini::subdev_spec_t &);
    void _check_time_source(std::string);
    void _check_clock_source(std::string);

private:    //Classes and Members
    n230_device_args_t                        _dev_args;
    boost::shared_ptr<n230_resource_manager>  _resource_mgr;
    boost::shared_ptr<n230_eeprom_manager>    _eeprom_mgr;
    boost::shared_ptr<n230_stream_manager>    _stream_mgr;
};

}}} //namespace

#endif /* INCLUDED_N230_IMPL_HPP */
