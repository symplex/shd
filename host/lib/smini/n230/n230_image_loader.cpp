//
// Copyright 2016 Ettus Research LLC
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

#include <fstream>
#include <algorithm>
#include <shd/image_loader.hpp>
#include <shd/exception.hpp>
#include <shd/utils/static.hpp>
#include <shd/utils/byteswap.hpp>
#include <shd/utils/paths.hpp>
#include <shd/transport/udp_simple.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include "n230_fw_host_iface.h"
#include "n230_impl.hpp"

using namespace shd;
using namespace shd::smini;
using namespace shd::transport;

struct xil_bitfile_hdr_t {
    xil_bitfile_hdr_t():
        valid(false), userid(0), product(""),
        fpga(""), timestamp(""), filesize(0)
    {}

    bool            valid;
    uint32_t userid;
    std::string     product;
    std::string     fpga;
    std::string     timestamp;
    uint32_t filesize;
};

static inline uint16_t _to_uint16(uint8_t* buf) {
    return (static_cast<uint16_t>(buf[0]) << 8) |
           (static_cast<uint16_t>(buf[1]) << 0);
}

static inline uint32_t _to_uint32(uint8_t* buf) {
    return (static_cast<uint32_t>(buf[0]) << 24) |
           (static_cast<uint32_t>(buf[1]) << 16) |
           (static_cast<uint32_t>(buf[2]) << 8)  |
           (static_cast<uint32_t>(buf[3]) << 0);
}

static void _parse_bitfile_header(const std::string& filepath, xil_bitfile_hdr_t& hdr) {
    // Read header into memory
    std::ifstream img_file(filepath.c_str(), std::ios::binary);
    static const size_t MAX_HDR_SIZE = 1024;
    boost::scoped_array<char> hdr_buf(new char[MAX_HDR_SIZE]);
    img_file.seekg(0, std::ios::beg);
    img_file.read(hdr_buf.get(), MAX_HDR_SIZE);
    img_file.close();

    //Parse header
    size_t ptr = 0;
    uint8_t* buf = reinterpret_cast<uint8_t*>(hdr_buf.get());  //Shortcut

    uint8_t signature[10] = {0x00, 0x09, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0};
    if (memcmp(buf, signature, 10) == 0) {  //Validate signature
        ptr += _to_uint16(buf + ptr) + 2;
        ptr += _to_uint16(buf + ptr) + 1;

        std::string fields[4];
        for (size_t i = 0; i < 4; i++) {
            size_t key = buf[ptr++] - 'a';
            uint16_t len = _to_uint16(buf + ptr); ptr += 2;
            fields[key] = std::string(reinterpret_cast<char*>(buf + ptr), size_t(len)); ptr += len;
        }

        hdr.filesize = _to_uint32(buf + ++ptr); ptr += 4;
        hdr.fpga = fields[1];
        hdr.timestamp = fields[2] + std::string(" ") + fields[3];

        std::vector<std::string> tokens;
        boost::split(tokens, fields[0], boost::is_any_of(";"));
        if (tokens.size() == 3) {
            hdr.product = tokens[0];
            std::vector<std::string> uidtokens;
            boost::split(uidtokens, tokens[1], boost::is_any_of("="));
            if (uidtokens.size() == 2 and uidtokens[0] == "UserID") {
                std::stringstream stream;
                stream << uidtokens[1];
                stream >> std::hex >> hdr.userid;
                hdr.valid = true;
            }
        }
    }
}

static size_t _send_and_recv(
    udp_simple::sptr xport,
    n230_flash_prog_t& out, n230_flash_prog_t& in)
{
    static uint32_t seqno = 0;
    out.seq = htonx<uint32_t>(++seqno);
    xport->send(boost::asio::buffer(&out, sizeof(n230_flash_prog_t)));
    size_t len = xport->recv(boost::asio::buffer(&in, udp_simple::mtu), 0.5);
    if (len != sizeof(n230_flash_prog_t) or ntohx<uint32_t>(in.seq) != seqno) {
        throw shd::io_error("Error communicating with the device.");
    }
    return len;
}


static bool n230_image_loader(const image_loader::image_loader_args_t &loader_args){
    // Run discovery routine and ensure that exactly one N230 is specified
    device_addrs_t devs = smini::n230::n230_impl::n230_find(loader_args.args);
    if (devs.size() == 0 or !loader_args.load_fpga) return false;
    if (devs.size() > 1) {
        throw shd::runtime_error("Multiple devices match the specified args. To avoid accidentally updating the "
                                 "wrong device, please narrow the search by specifying a unique \"addr\" argument.");
    }
    device_addr_t dev = devs[0];

    // Sanity check the specified bitfile
    std::string fpga_img_path = loader_args.fpga_path;
    bool fpga_path_specified = !loader_args.fpga_path.empty();
    if (not fpga_path_specified) {
        fpga_img_path = (
            fs::path(shd::get_pkg_path()) / "share" / "shd" / "images" / "smini_n230_fpga.bit"
        ).string();
    }

    if (not boost::filesystem::exists(fpga_img_path)) {
        if (fpga_path_specified) {
            throw shd::runtime_error(str(boost::format("The file \"%s\" does not exist.") % fpga_img_path));
        } else {
            throw shd::runtime_error(str(boost::format(
                "Could not find the default FPGA image: %s.\n"
                "Either specify the --fpga-path argument or download the latest prebuilt images:\n"
                "%s\n")
            % fpga_img_path % print_utility_error("shd_images_downloader.py")));
        }
    }
    xil_bitfile_hdr_t hdr;
    _parse_bitfile_header(fpga_img_path, hdr);

    // Create a UDP communication link
    udp_simple::sptr udp_xport =
        udp_simple::make_connected(dev["addr"],BOOST_STRINGIZE(N230_FW_COMMS_FLASH_PROG_PORT));

    if (hdr.valid and hdr.product == "n230") {
        if (hdr.userid != 0x5AFE0000) {
            std::cout << boost::format("Unit: SMINI N230 (%s, %s)\n-- FPGA Image: %s\n")
                         % dev["addr"] % dev["serial"] % fpga_img_path;

            // Write image
            std::ifstream image(fpga_img_path.c_str(), std::ios::binary);
            size_t image_size = boost::filesystem::file_size(fpga_img_path);

            static const size_t SECTOR_SIZE = 65536;
            static const size_t IMAGE_BASE  = 0x400000;

            n230_flash_prog_t out, in;
            size_t bytes_written = 0;
            while (bytes_written < image_size) {
                size_t payload_size = std::min<size_t>(image_size - bytes_written, N230_FLASH_COMM_MAX_PAYLOAD_SIZE);
                if (bytes_written % SECTOR_SIZE == 0) {
                    out.flags = htonx<uint32_t>(N230_FLASH_COMM_FLAGS_ACK|N230_FLASH_COMM_CMD_ERASE_FPGA);
                    out.offset = htonx<uint32_t>(bytes_written + IMAGE_BASE);
                    out.size = htonx<uint32_t>(payload_size);
                    _send_and_recv(udp_xport, out, in);
                }
                out.flags = htonx<uint32_t>(N230_FLASH_COMM_FLAGS_ACK|N230_FLASH_COMM_CMD_WRITE_FPGA);
                out.offset = htonx<uint32_t>(bytes_written + IMAGE_BASE);
                out.size = htonx<uint32_t>(payload_size);
                image.read((char*)out.data, payload_size);
                _send_and_recv(udp_xport, out, in);
                bytes_written += ntohx<uint32_t>(in.size);
                std::cout << boost::format("\r-- Loading FPGA image: %d%%")
                             % (int(double(bytes_written) / double(image_size) * 100.0))
                         << std::flush;
            }
            std::cout << std::endl << "FPGA image loaded successfully." << std::endl;
            std::cout << std::endl << "Power-cycle the device to run the image." << std::endl;
            return true;
        } else {
            throw shd::runtime_error("This utility cannot burn a failsafe image!");
        }
    } else {
        throw shd::runtime_error(str(boost::format("The file at path \"%s\" is not a valid SMINI N230 FPGA image.")
                                     % fpga_img_path));
    }
}

SHD_STATIC_BLOCK(register_n230_image_loader){
    std::string recovery_instructions = "Aborting. Your SMINI N230 device will likely boot in safe mode.\n"
                                        "Please re-run this command with the additional \"safe_mode\" device argument\n"
                                        "to recover your device.";

    image_loader::register_image_loader("n230", n230_image_loader, recovery_instructions);
}
