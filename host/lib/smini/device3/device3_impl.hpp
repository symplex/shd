//
// Copyright 2014-2015 Ettus Research LLC
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

// Declares the device3_impl class which is a layer between device3 and
// the different 3-rd gen device impls (e.g. x300_impl)

#ifndef INCLUDED_DEVICE3_IMPL_HPP
#define INCLUDED_DEVICE3_IMPL_HPP

#include <shd/transport/bounded_buffer.hpp>
#include <shd/transport/vrt_if_packet.hpp>
#include <shd/transport/chdr.hpp>
#include <shd/transport/zero_copy.hpp>
#include <shd/types/sid.hpp>
#include <shd/types/metadata.hpp>
#include <shd/types/endianness.hpp>
#include <shd/types/direction.hpp>
#include <shd/utils/tasks.hpp>
#include <shd/device3.hpp>
#include "xports.hpp"

namespace shd { namespace smini {

/***********************************************************************
 * Default settings (any device3 may override these)
 **********************************************************************/
static const size_t DEVICE3_RX_FC_REQUEST_FREQ         = 32;    //per flow-control window
static const size_t DEVICE3_TX_FC_RESPONSE_FREQ        = 8;
static const size_t DEVICE3_TX_FC_RESPONSE_CYCLES      = 0;     // Cycles: Off.

static const size_t DEVICE3_TX_MAX_HDR_LEN             = shd::transport::vrt::chdr::max_if_hdr_words64 * sizeof(uint64_t);    // Bytes
static const size_t DEVICE3_RX_MAX_HDR_LEN             = shd::transport::vrt::chdr::max_if_hdr_words64 * sizeof(uint64_t);    // Bytes

class device3_impl : public shd::device3, public boost::enable_shared_from_this<device3_impl>
{
public:
    /***********************************************************************
     * device3-specific Types
     **********************************************************************/
    typedef shd::transport::bounded_buffer<shd::async_metadata_t> async_md_type;

    //! The purpose of a transport
    enum xport_type_t {
        CTRL = 0,
        ASYNC_MSG,
        TX_DATA,
        RX_DATA
    };

    enum xport_t {AXI, ETH, PCIE};

    //! Stores all streaming-related options
    struct stream_options_t
    {
        //! Max size of the header in bytes for TX
        size_t tx_max_len_hdr;
        //! Max size of the header in bytes for RX
        size_t rx_max_len_hdr;
        //! How often we send ACKs to the upstream block per one full FC window
        size_t rx_fc_request_freq;
        //! How often the downstream block should send ACKs per one full FC window
        size_t tx_fc_response_freq;
        //! How often the downstream block should send ACKs in cycles
        size_t tx_fc_response_cycles;
        stream_options_t(void)
            : tx_max_len_hdr(DEVICE3_TX_MAX_HDR_LEN)
            , rx_max_len_hdr(DEVICE3_RX_MAX_HDR_LEN)
            , rx_fc_request_freq(DEVICE3_RX_FC_REQUEST_FREQ)
            , tx_fc_response_freq(DEVICE3_TX_FC_RESPONSE_FREQ)
            , tx_fc_response_cycles(DEVICE3_TX_FC_RESPONSE_CYCLES)
        {};
    };

    /***********************************************************************
     * I/O Interface
     **********************************************************************/
    shd::tx_streamer::sptr get_tx_stream(const shd::stream_args_t &);
    shd::rx_streamer::sptr get_rx_stream(const shd::stream_args_t &);
    bool recv_async_msg(shd::async_metadata_t &async_metadata, double timeout);

    /***********************************************************************
     * Other public APIs
     **********************************************************************/
    rfnoc::graph::sptr create_graph(const std::string &name="");

protected:
    /***********************************************************************
     * Structors
     **********************************************************************/
    device3_impl();
    virtual ~device3_impl() {};

    /***********************************************************************
     * Streaming-related
     **********************************************************************/
    // The 'rate' argument is so we can use these as subscribers to rate changes
public: // TODO make these protected again
    void update_rx_streamers(double rate=-1.0);
    void update_tx_streamers(double rate=-1.0);
protected:

    /***********************************************************************
     * Transport-related
     **********************************************************************/
    stream_options_t stream_options;

    /*! \brief Create a transport to a given endpoint.
     *
     * \param address The endpoint address of the block we're creating a transport to.
     *                The source address in this value is not considered, only the
     *                destination address.
     * \param xport_type Specify which kind of transport this is.
     * \param args Additional arguments for the transport generation. See \ref page_transport
     *             for valid arguments.
     */
    virtual shd::both_xports_t make_transport(
        const shd::sid_t &address,
        const xport_type_t xport_type,
        const shd::device_addr_t& args
    ) = 0;

    virtual shd::device_addr_t get_tx_hints(size_t) { return shd::device_addr_t(); };
    virtual shd::device_addr_t get_rx_hints(size_t) { return shd::device_addr_t(); };
    virtual shd::endianness_t get_transport_endianness(size_t mb_index) = 0;

    //! Is called after a streamer is generated
    virtual void post_streamer_hooks(shd::direction_t) {};

    /***********************************************************************
     * Channel-related
     **********************************************************************/
    /*! Merge a list of channels into the existing channel definition.
     *
     * Intelligently merge the channels described in \p chan_ids
     * into the current channel definition. If none of the channels in
     * \p chan_ids is in the current definition, they simply get appended.
     * Otherwise, they get overwritten in the order of \p chan_ids.
     *
     * \param chan_ids List of block IDs for the channels.
     * \param chan_args New channel args. Must have same length as chan_ids.
     *
     */
    void merge_channel_defs(
            const std::vector<rfnoc::block_id_t> &chan_ids,
            const std::vector<shd::device_addr_t> &chan_args,
            const shd::direction_t dir
    );

    /***********************************************************************
     * RFNoC-Specific
     **********************************************************************/
    void enumerate_rfnoc_blocks(
            size_t device_index,
            size_t n_blocks,
            size_t base_port,
            const shd::sid_t &base_sid,
            shd::device_addr_t transport_args,
            shd::endianness_t endianness
    );

    /***********************************************************************
     * Members
     **********************************************************************/
    //! A counter, designed to create unique SIDs
    size_t _sid_framer;

    // TODO: Maybe move these to private
    shd::dict<std::string, boost::weak_ptr<shd::rx_streamer> > _rx_streamers;
    shd::dict<std::string, boost::weak_ptr<shd::tx_streamer> > _tx_streamers;

private:
    /***********************************************************************
     * Private Members
     **********************************************************************/
    //! Buffer for async metadata
    boost::shared_ptr<async_md_type> _async_md;

    //! This mutex locks the get_xx_stream() functions.
    boost::mutex _transport_setup_mutex;
};

}} /* namespace shd::smini */

#endif /* INCLUDED_DEVICE3_IMPL_HPP */
// vim: sw=4 expandtab:
