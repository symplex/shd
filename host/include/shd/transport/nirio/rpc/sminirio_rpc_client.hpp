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

#ifndef INCLUDED_SMINIRIO_RPC_CLIENT_HPP
#define INCLUDED_SMINIRIO_RPC_CLIENT_HPP

#include <shd/transport/nirio/rpc/rpc_common.hpp>
#include <shd/transport/nirio/rpc/rpc_client.hpp>
#include <shd/transport/nirio/rpc/sminirio_rpc_common.hpp>
#include <shd/transport/nirio/status.h>

namespace shd { namespace sminirio_rpc {

class SHD_API sminirio_rpc_client {
public:
    sminirio_rpc_client(
        std::string server,
        std::string port);
    ~sminirio_rpc_client();

    inline void set_rpc_timeout(boost::posix_time::milliseconds timeout_in_ms) {
        _timeout = timeout_in_ms;
    }

    inline nirio_status get_ctor_status() {
        return _ctor_status;
    }

    nirio_status nisminirio_enumerate(
        NISMINIRIO_ENUMERATE_ARGS);
    nirio_status nisminirio_open_session(
        NISMINIRIO_OPEN_SESSION_ARGS);
    nirio_status nisminirio_close_session(
        NISMINIRIO_CLOSE_SESSION_ARGS);
    nirio_status nisminirio_reset_device(
        NISMINIRIO_RESET_SESSION_ARGS);
    nirio_status nisminirio_download_bitstream_to_fpga(
        NISMINIRIO_DOWNLOAD_BITSTREAM_TO_FPGA_ARGS);
    nirio_status nisminirio_get_interface_path(
         NISMINIRIO_GET_INTERFACE_PATH_ARGS);
    nirio_status nisminirio_download_fpga_to_flash(
        NISMINIRIO_DOWNLOAD_FPGA_TO_FLASH_ARGS);

    static const int64_t DEFAULT_TIMEOUT_IN_MS = 5000;

private:
    static nirio_status _boost_error_to_nirio_status(const boost::system::error_code& err);

    rpc_client                      _rpc_client;
    boost::posix_time::milliseconds _timeout;
    nirio_status                    _ctor_status;
};

}}

#endif /* INCLUDED_SMINIRIO_RPC_CLIENT_HPP */
