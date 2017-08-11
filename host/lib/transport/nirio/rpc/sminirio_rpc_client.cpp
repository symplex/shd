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

#include <shd/transport/nirio/rpc/sminirio_rpc_client.hpp>
#include <shd/utils/platform.hpp>

namespace shd { namespace sminirio_rpc {

sminirio_rpc_client::sminirio_rpc_client(
    std::string server,
    std::string port
) : _rpc_client(server, port, shd::get_process_id(), shd::get_host_id()),
    _timeout(boost::posix_time::milliseconds(DEFAULT_TIMEOUT_IN_MS))
{
   _ctor_status = _rpc_client.status() ? NiRio_Status_RpcConnectionError : NiRio_Status_Success;
}

sminirio_rpc_client::~sminirio_rpc_client()
{
}

nirio_status sminirio_rpc_client::nisminirio_enumerate(NISMINIRIO_ENUMERATE_ARGS)
/*
#define NISMINIRIO_ENUMERATE_ARGS         \
    sminirio_device_info_vtr& device_info_vtr
*/
{
    sminirio_rpc::func_args_writer_t in_args;
    sminirio_rpc::func_args_reader_t out_args;
    nirio_status status = NiRio_Status_Success;
    uint32_t vtr_size = 0;

    status = _boost_error_to_nirio_status(
        _rpc_client.call(NISMINIRIO_ENUMERATE, in_args, out_args, _timeout));

    if (nirio_status_not_fatal(status)) {
        out_args >> status;
        out_args >> vtr_size;
    }
    if (nirio_status_not_fatal(status) && vtr_size > 0) {
        device_info_vtr.resize(vtr_size);
        for (size_t i = 0; i < (size_t)vtr_size; i++) {
            sminirio_device_info info;
            out_args >> info;
            device_info_vtr[i] = info;
        }
    }
    return status;
}

nirio_status sminirio_rpc_client::nisminirio_open_session(NISMINIRIO_OPEN_SESSION_ARGS)
/*
#define NISMINIRIO_OPEN_SESSION_ARGS     \
    const std::string& resource,        \
    const std::string& path,            \
    const std::string& signature,       \
    const uint16_t& download_fpga
*/
{
    sminirio_rpc::func_args_writer_t in_args;
    sminirio_rpc::func_args_reader_t out_args;
    nirio_status status = NiRio_Status_Success;

    in_args << resource;
    in_args << path;
    in_args << signature;
    in_args << download_fpga;

    //Open needs a longer timeout because the FPGA download can take upto 6 secs and the NiFpga libload can take 4.
    static const uint32_t OPEN_TIMEOUT = 15000;
    status = _boost_error_to_nirio_status(
        _rpc_client.call(NISMINIRIO_OPEN_SESSION, in_args, out_args, boost::posix_time::milliseconds(OPEN_TIMEOUT)));

    if (nirio_status_not_fatal(status)) {
        out_args >> status;
    }

    return status;
}

nirio_status sminirio_rpc_client::nisminirio_close_session(NISMINIRIO_CLOSE_SESSION_ARGS)
/*
#define NISMINIRIO_CLOSE_SESSION_ARGS    \
    const std::string& resource
*/
{
    sminirio_rpc::func_args_writer_t in_args;
    sminirio_rpc::func_args_reader_t out_args;
    nirio_status status = NiRio_Status_Success;

    in_args << resource;

    status = _boost_error_to_nirio_status(
        _rpc_client.call(NISMINIRIO_CLOSE_SESSION, in_args, out_args, _timeout));

    if (nirio_status_not_fatal(status)) {
        out_args >> status;
    }

    return status;
}

nirio_status sminirio_rpc_client::nisminirio_reset_device(NISMINIRIO_RESET_SESSION_ARGS)
/*
#define NISMINIRIO_RESET_SESSION_ARGS    \
    const std::string& resource
*/
{
    sminirio_rpc::func_args_writer_t in_args;
    sminirio_rpc::func_args_reader_t out_args;
    nirio_status status = NiRio_Status_Success;

    in_args << resource;

    status = _boost_error_to_nirio_status(
        _rpc_client.call(NISMINIRIO_RESET_SESSION, in_args, out_args, _timeout));

    if (nirio_status_not_fatal(status)) {
        out_args >> status;
    }

    return status;
}

nirio_status sminirio_rpc_client::nisminirio_get_interface_path(NISMINIRIO_GET_INTERFACE_PATH_ARGS)
/*
#define NISMINIRIO_GET_INTERFACE_PATH_ARGS   \
    const std::string& resource,            \
    std::string& interface_path
*/
{
    sminirio_rpc::func_args_writer_t in_args;
    sminirio_rpc::func_args_reader_t out_args;
    nirio_status status = NiRio_Status_Success;

    in_args << resource;

    status = _boost_error_to_nirio_status(
        _rpc_client.call(NISMINIRIO_GET_INTERFACE_PATH, in_args, out_args, _timeout));

    if (nirio_status_not_fatal(status)) {
        out_args >> status;
        out_args >> interface_path;
    }

    return status;
}

nirio_status sminirio_rpc_client::nisminirio_download_fpga_to_flash(NISMINIRIO_DOWNLOAD_FPGA_TO_FLASH_ARGS)
/*
#define NISMINIRIO_DOWNLOAD_FPGA_TO_FLASH_ARGS   \
    const uint32_t& interface_num,       \
    const std::string& bitstream_path
*/
{
    sminirio_rpc::func_args_writer_t in_args;
    sminirio_rpc::func_args_reader_t out_args;
    nirio_status status = NiRio_Status_Success;

    in_args << resource;
    in_args << bitstream_path;

    static const uint32_t DOWNLOAD_FPGA_TIMEOUT = 1200000;
    status = _boost_error_to_nirio_status(
        _rpc_client.call(NISMINIRIO_DOWNLOAD_FPGA_TO_FLASH, in_args, out_args,
            boost::posix_time::milliseconds(DOWNLOAD_FPGA_TIMEOUT)));

    if (nirio_status_not_fatal(status)) {
        out_args >> status;
    }

    return status;
}

nirio_status sminirio_rpc_client::nisminirio_download_bitstream_to_fpga(NISMINIRIO_DOWNLOAD_BITSTREAM_TO_FPGA_ARGS)
/*
#define NISMINIRIO_DOWNLOAD_BITSTREAM_TO_FPGA_ARGS    \
    const std::string& resource
*/
{
    sminirio_rpc::func_args_writer_t in_args;
    sminirio_rpc::func_args_reader_t out_args;
    nirio_status status = NiRio_Status_Success;

    in_args << resource;

    status = _boost_error_to_nirio_status(
        _rpc_client.call(NISMINIRIO_DOWNLOAD_BITSTREAM_TO_FPGA, in_args, out_args, _timeout));

    if (nirio_status_not_fatal(status)) {
        out_args >> status;
    }

    return status;
}

nirio_status sminirio_rpc_client::_boost_error_to_nirio_status(const boost::system::error_code& err) {
    if (err) {
        switch (err.value()) {
            case boost::asio::error::connection_aborted:
            case boost::asio::error::connection_refused:
            case boost::asio::error::eof:
                return NiRio_Status_RpcSessionError;
            case boost::asio::error::timed_out:
            case boost::asio::error::operation_aborted:
                return NiRio_Status_RpcOperationError;
            default:
                return NiRio_Status_SoftwareFault;
        }
    } else {
        return NiRio_Status_Success;
    }
}

}}
