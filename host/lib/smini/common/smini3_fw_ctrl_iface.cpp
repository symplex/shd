//
// Copyright 2013 Ettus Research LLC
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

#include "smini3_fw_ctrl_iface.hpp"

#include <shd/utils/byteswap.hpp>
#include <shd/utils/msg.hpp>
#include <shd/exception.hpp>
#include <boost/format.hpp>
#include <boost/asio.hpp> //used for htonl and ntohl
#include <boost/foreach.hpp>
#include "fw_comm_protocol.h"

namespace shd { namespace smini { namespace smini3 {

//----------------------------------------------------------
// Factory method
//----------------------------------------------------------
shd::wb_iface::sptr smini3_fw_ctrl_iface::make(
    shd::transport::udp_simple::sptr udp_xport,
    const uint16_t product_id,
    const bool verbose)
{
    return wb_iface::sptr(new smini3_fw_ctrl_iface(udp_xport, product_id, verbose));
}

//----------------------------------------------------------
// udp_fw_ctrl_iface
//----------------------------------------------------------

smini3_fw_ctrl_iface::smini3_fw_ctrl_iface(
    shd::transport::udp_simple::sptr udp_xport,
    const uint16_t product_id,
    const bool verbose) :
    _product_id(product_id), _verbose(verbose), _udp_xport(udp_xport),
    _seq_num(0)
{
    flush();
    peek32(0);
}

smini3_fw_ctrl_iface::~smini3_fw_ctrl_iface()
{
    flush();
}

void smini3_fw_ctrl_iface::flush()
{
    boost::mutex::scoped_lock lock(_mutex);
    _flush();
}

void smini3_fw_ctrl_iface::poke32(const wb_addr_type addr, const uint32_t data)
{
    boost::mutex::scoped_lock lock(_mutex);

    for (size_t i = 1; i <= NUM_RETRIES; i++) {
        try {
            _poke32(addr, data);
            return;
        } catch(const std::exception &ex) {
            const std::string error_msg = str(boost::format(
                "udp fw poke32 failure #%u\n%s") % i % ex.what());
            if (_verbose) SHD_MSG(warning) << error_msg << std::endl;
            if (i == NUM_RETRIES) throw shd::io_error(error_msg);
        }
    }
}

uint32_t smini3_fw_ctrl_iface::peek32(const wb_addr_type addr)
{
    boost::mutex::scoped_lock lock(_mutex);

    for (size_t i = 1; i <= NUM_RETRIES; i++) {
        try {
            return _peek32(addr);
        } catch(const std::exception &ex) {
            const std::string error_msg = str(boost::format(
                "udp fw peek32 failure #%u\n%s") % i % ex.what());
            if (_verbose) SHD_MSG(warning) << error_msg << std::endl;
            if (i == NUM_RETRIES) throw shd::io_error(error_msg);
        }
    }
    return 0;
}

void smini3_fw_ctrl_iface::_poke32(const wb_addr_type addr, const uint32_t data)
{
    //Load request struct
    fw_comm_pkt_t request;
    request.id = shd::htonx<uint32_t>(FW_COMM_GENERATE_ID(_product_id));
    request.flags = shd::htonx<uint32_t>(FW_COMM_FLAGS_ACK | FW_COMM_CMD_POKE32);
    request.sequence = shd::htonx<uint32_t>(_seq_num++);
    request.addr = shd::htonx(addr);
    request.data_words = 1;
    request.data[0] = shd::htonx(data);

    //Send request
    _flush();
    _udp_xport->send(boost::asio::buffer(&request, sizeof(request)));

    //Recv reply
    fw_comm_pkt_t reply;
    const size_t nbytes = _udp_xport->recv(boost::asio::buffer(&reply, sizeof(reply)), 1.0);
    if (nbytes == 0) throw shd::io_error("udp fw poke32 - reply timed out");

    //Sanity checks
    const size_t flags = shd::ntohx<uint32_t>(reply.flags);
    SHD_ASSERT_THROW(nbytes == sizeof(reply));
    SHD_ASSERT_THROW(not (flags & FW_COMM_FLAGS_ERROR_MASK));
    SHD_ASSERT_THROW(flags & FW_COMM_CMD_POKE32);
    SHD_ASSERT_THROW(flags & FW_COMM_FLAGS_ACK);
    SHD_ASSERT_THROW(reply.sequence == request.sequence);
    SHD_ASSERT_THROW(reply.addr == request.addr);
    SHD_ASSERT_THROW(reply.data[0] == request.data[0]);
}

uint32_t smini3_fw_ctrl_iface::_peek32(const wb_addr_type addr)
{
    //Load request struct
    fw_comm_pkt_t request;
    request.id = shd::htonx<uint32_t>(FW_COMM_GENERATE_ID(_product_id));
    request.flags = shd::htonx<uint32_t>(FW_COMM_FLAGS_ACK | FW_COMM_CMD_PEEK32);
    request.sequence = shd::htonx<uint32_t>(_seq_num++);
    request.addr = shd::htonx(addr);
    request.data_words = 1;
    request.data[0] = 0;

    //Send request
    _flush();
    _udp_xport->send(boost::asio::buffer(&request, sizeof(request)));

    //Recv reply
    fw_comm_pkt_t reply;
    const size_t nbytes = _udp_xport->recv(boost::asio::buffer(&reply, sizeof(reply)), 1.0);
    if (nbytes == 0) throw shd::io_error("udp fw peek32 - reply timed out");

    //Sanity checks
    const size_t flags = shd::ntohx<uint32_t>(reply.flags);
    SHD_ASSERT_THROW(nbytes == sizeof(reply));
    SHD_ASSERT_THROW(not (flags & FW_COMM_FLAGS_ERROR_MASK));
    SHD_ASSERT_THROW(flags & FW_COMM_CMD_PEEK32);
    SHD_ASSERT_THROW(flags & FW_COMM_FLAGS_ACK);
    SHD_ASSERT_THROW(reply.sequence == request.sequence);
    SHD_ASSERT_THROW(reply.addr == request.addr);

    //return result!
    return shd::ntohx<uint32_t>(reply.data[0]);
}

void smini3_fw_ctrl_iface::_flush(void)
{
    char buff[FW_COMM_PROTOCOL_MTU] = {};
    while (_udp_xport->recv(boost::asio::buffer(buff), 0.0)) {
        /*NOP*/
    }
}

std::vector<std::string> smini3_fw_ctrl_iface::discover_devices(
    const std::string& addr_hint, const std::string& port,
    uint16_t product_id)
{
    std::vector<std::string> addrs;

    //Create a UDP transport to communicate:
    //Some devices will cause a throw when opened for a broadcast address.
    //We print and recover so the caller can loop through all bcast addrs.
    shd::transport::udp_simple::sptr udp_bcast_xport;
    try {
        udp_bcast_xport = shd::transport::udp_simple::make_broadcast(addr_hint, port);
    } catch(const std::exception &e) {
        SHD_MSG(error) << boost::format("Cannot open UDP transport on %s for discovery\n%s")
        % addr_hint % e.what() << std::endl;
        return addrs;
    }

    //Send dummy request
    fw_comm_pkt_t request;
    request.id = shd::htonx<uint32_t>(FW_COMM_GENERATE_ID(product_id));
    request.flags = shd::htonx<uint32_t>(FW_COMM_FLAGS_ACK|FW_COMM_CMD_ECHO);
    request.sequence = shd::htonx<uint32_t>(std::rand());
    udp_bcast_xport->send(boost::asio::buffer(&request, sizeof(request)));

    //loop for replies until timeout
    while (true) {
        char buff[FW_COMM_PROTOCOL_MTU] = {};
        const size_t nbytes = udp_bcast_xport->recv(boost::asio::buffer(buff), 0.050);
        if (nbytes != sizeof(fw_comm_pkt_t)) break; //No more responses or responses are invalid

        const fw_comm_pkt_t *reply = (const fw_comm_pkt_t *)buff;
        if (request.id       == reply->id &&
            request.flags    == reply->flags &&
            request.sequence == reply->sequence)
        {
            addrs.push_back(udp_bcast_xport->get_recv_addr());
        }
    }

    return addrs;
}

uint32_t smini3_fw_ctrl_iface::get_iface_id(
    const std::string& addr, const std::string& port,
    uint16_t product_id)
{
    shd::transport::udp_simple::sptr udp_xport =
        shd::transport::udp_simple::make_connected(addr, port);

    //Send dummy request
    fw_comm_pkt_t request;
    request.id = shd::htonx<uint32_t>(FW_COMM_GENERATE_ID(product_id));
    request.flags = shd::htonx<uint32_t>(FW_COMM_FLAGS_ACK|FW_COMM_CMD_ECHO);
    request.sequence = shd::htonx<uint32_t>(std::rand());
    udp_xport->send(boost::asio::buffer(&request, sizeof(request)));

    //loop for replies until timeout
    char buff[FW_COMM_PROTOCOL_MTU] = {};
    const size_t nbytes = udp_xport->recv(boost::asio::buffer(buff), 1.0);

    const fw_comm_pkt_t *reply = (const fw_comm_pkt_t *)buff;
    if (nbytes            >  0 &&
        request.id        == reply->id &&
        request.flags     == reply->flags &&
        request.sequence  == reply->sequence)
    {
        return shd::ntohx<uint32_t>(reply->data[0]);
    } else {
        throw shd::io_error("udp get_iface_id - bad response");
    }
}

}}} //namespace
