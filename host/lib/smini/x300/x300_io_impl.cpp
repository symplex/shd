//
// Copyright 2013-2014 Ettus Research LLC
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

#include "x300_regs.hpp"
#include "x300_impl.hpp"
#include "../../transport/super_recv_packet_handler.hpp"
#include "../../transport/super_send_packet_handler.hpp"
#include <shd/transport/nirio_zero_copy.hpp>
#include "async_packet_handler.hpp"
#include <shd/transport/bounded_buffer.hpp>
#include <boost/bind.hpp>
#include <shd/utils/tasks.hpp>
#include <shd/utils/log.hpp>
#include <shd/utils/msg.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

using namespace shd;
using namespace shd::smini;
using namespace shd::transport;

/***********************************************************************
 * Hooks for get_tx_stream() and get_rx_stream()
 **********************************************************************/
device_addr_t x300_impl::get_rx_hints(size_t mb_index)
{
    device_addr_t rx_hints = _mb[mb_index].recv_args;
    // (default to a large recv buff)
    if (not rx_hints.has_key("recv_buff_size"))
    {
        if (_mb[mb_index].xport_path != "nirio") {
            //For the ethernet transport, the buffer has to be set before creating
            //the transport because it is independent of the frame size and # frames
            //For nirio, the buffer size is not configurable by the user
            #if defined(SHD_PLATFORM_MACOS) || defined(SHD_PLATFORM_BSD)
                //limit buffer resize on macos or it will error
                rx_hints["recv_buff_size"] = boost::lexical_cast<std::string>(X300_RX_SW_BUFF_SIZE_ETH_MACOS);
            #elif defined(SHD_PLATFORM_LINUX) || defined(SHD_PLATFORM_WIN32)
                //set to half-a-second of buffering at max rate
                rx_hints["recv_buff_size"] = boost::lexical_cast<std::string>(X300_RX_SW_BUFF_SIZE_ETH);
            #endif
        }
    }
    return rx_hints;
}


device_addr_t x300_impl::get_tx_hints(size_t mb_index)
{
    device_addr_t tx_hints = _mb[mb_index].send_args;
    return tx_hints;
}

void x300_impl::post_streamer_hooks(direction_t dir)
{
    if (dir != TX_DIRECTION) {
        return;
    }

    // Loop through all tx streamers. Find all radios connected to one
    // streamer. Sync those.
    BOOST_FOREACH(const boost::weak_ptr<shd::tx_streamer> &streamer_w, _tx_streamers.vals()) {
        const boost::shared_ptr<sph::send_packet_streamer> streamer =
            boost::dynamic_pointer_cast<sph::send_packet_streamer>(streamer_w.lock());
        if (not streamer) {
            continue;
        }

        std::vector<rfnoc::x300_radio_ctrl_impl::sptr> radio_ctrl_blks =
            streamer->get_terminator()->find_downstream_node<rfnoc::x300_radio_ctrl_impl>();
        try {
            //SHD_MSG(status) << "[X300] syncing " << radio_ctrl_blks.size() << " radios " << std::endl;
            rfnoc::x300_radio_ctrl_impl::synchronize_dacs(radio_ctrl_blks);
        }
        catch(const shd::io_error &ex) {
            throw shd::io_error(str(boost::format("Failed to sync DACs! %s ") % ex.what()));
        }
    }
}

// vim: sw=4 expandtab:
