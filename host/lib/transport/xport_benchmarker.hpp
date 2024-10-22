//
// Copyright 2010-2013 Ettus Research LLC
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

#ifndef INCLUDED_LIBSHD_XPORT_BENCHMARKER_HPP
#define INCLUDED_LIBSHD_XPORT_BENCHMARKER_HPP

#include <shd/transport/zero_copy.hpp>
#include <shd/types/device_addr.hpp>
#include <shd/utils/msg.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <shd/transport/vrt_if_packet.hpp>

namespace shd { namespace transport {

//Test class to benchmark a low-level transport object with a VITA/C-VITA data stream
class xport_benchmarker : boost::noncopyable {
public:
    const device_addr_t& benchmark_throughput_chdr(
        zero_copy_if::sptr tx_transport,
        zero_copy_if::sptr rx_transport,
        uint32_t sid,
        bool big_endian,
        uint32_t duration_ms);

private:
    void _stream_tx(
        zero_copy_if* transport,
        vrt::if_packet_info_t* pkt_info,
        bool big_endian);

    void _stream_rx(
        zero_copy_if* transport,
        const vrt::if_packet_info_t* exp_pkt_info,
        bool big_endian);

    void _initialize_chdr(
        zero_copy_if::sptr tx_transport,
        zero_copy_if::sptr rx_transport,
        uint32_t sid,
        vrt::if_packet_info_t& pkt_info);

    void _reset_counters(void);

    boost::shared_ptr<boost::thread>    _tx_thread;
    boost::shared_ptr<boost::thread>    _rx_thread;

    uint64_t     _num_tx_packets;
    uint64_t     _num_rx_packets;
    uint64_t     _num_tx_timeouts;
    uint64_t     _num_rx_timeouts;
    uint64_t     _num_data_errors;

    double              _tx_timeout;
    double              _rx_timeout;

    device_addr_t       _results;
};


}} //namespace

#endif /* INCLUDED_LIBSHD_XPORT_BENCHMARKER_HPP */
