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


#ifndef INCLUDED_SHD_TRANSPORT_NIRIO_NISMINIRIO_SESSION_H
#define INCLUDED_SHD_TRANSPORT_NIRIO_NISMINIRIO_SESSION_H

#include <stdint.h>
#include <shd/transport/nirio/rpc/sminirio_rpc_client.hpp>
#include <shd/transport/nirio/niriok_proxy.h>
#include <shd/transport/nirio/nirio_resource_manager.h>
#include <shd/transport/nirio/nifpga_lvbitx.h>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <string>

namespace shd { namespace nisminirio {

class SHD_API nisminirio_session : private boost::noncopyable
{
public:
    typedef boost::shared_ptr<nisminirio_session> sptr;
    typedef shd::sminirio_rpc::sminirio_device_info device_info;
    typedef shd::sminirio_rpc::sminirio_device_info_vtr device_info_vtr;

    static nirio_status enumerate(
        const std::string& rpc_port_name,
        device_info_vtr& device_info_vtr);

    nisminirio_session(
        const std::string& resource_name,
        const std::string& port_name);
    virtual ~nisminirio_session();

    nirio_status open(
        nifpga_lvbitx::sptr lvbitx,
        bool force_download = false);

    void close(bool skip_reset = false);

    nirio_status reset();

    template<typename data_t>
    nirio_status create_tx_fifo(
        const char* fifo_name,
        boost::shared_ptr< nirio_fifo<data_t> >& fifo)
    {
        if (!_session_open) return NiRio_Status_ResourceNotInitialized;
        return _resource_manager.create_tx_fifo(fifo_name, fifo);
    }

    template<typename data_t>
    nirio_status create_tx_fifo(
        uint32_t fifo_instance,
        boost::shared_ptr< nirio_fifo<data_t> >& fifo)
    {
        if ((size_t)fifo_instance >= _lvbitx->get_output_fifo_count()) return NiRio_Status_InvalidParameter;
        return create_tx_fifo(_lvbitx->get_output_fifo_names()[fifo_instance], fifo);
    }

    template<typename data_t>
    nirio_status create_rx_fifo(
        const char* fifo_name,
        boost::shared_ptr< nirio_fifo<data_t> >& fifo)
    {
        if (!_session_open) return NiRio_Status_ResourceNotInitialized;
        return _resource_manager.create_rx_fifo(fifo_name, fifo);
    }

    template<typename data_t>
    nirio_status create_rx_fifo(
        uint32_t fifo_instance,
        boost::shared_ptr< nirio_fifo<data_t> >& fifo)
    {
        if ((size_t)fifo_instance >= _lvbitx->get_input_fifo_count()) return NiRio_Status_InvalidParameter;
        return create_rx_fifo(_lvbitx->get_input_fifo_names()[fifo_instance], fifo);
    }

    SHD_INLINE niriok_proxy::sptr get_kernel_proxy() {
        return _riok_proxy;
    }

    nirio_status download_bitstream_to_flash(const std::string& bitstream_path);

    //Static
    static niriok_proxy::sptr create_kernel_proxy(
        const std::string& resource_name,
        const std::string& rpc_port_name);

private:
    nirio_status _verify_signature();
    std::string _read_bitstream_checksum();
    nirio_status _write_bitstream_checksum(const std::string& checksum);
    nirio_status _ensure_fpga_ready();

    std::string                     _resource_name;
    nifpga_lvbitx::sptr             _lvbitx;
    std::string                     _interface_path;
    bool                            _session_open;
    niriok_proxy::sptr              _riok_proxy;
    nirio_resource_manager          _resource_manager;
    sminirio_rpc::sminirio_rpc_client _rpc_client;
    boost::recursive_mutex          _session_mutex;

    static const uint32_t FPGA_READY_TIMEOUT_IN_MS      = 1000;
    static const uint32_t SESSION_LOCK_TIMEOUT_IN_MS    = 3000;
    static const uint32_t SESSION_LOCK_RETRY_INT_IN_MS  = 500;
};

}}

#endif /* INCLUDED_SHD_TRANSPORT_NIRIO_NISMINIRIO_SESSION_H */
