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
#include <shd/image_loader.hpp>
#include <shd/utils/msg.hpp>
#include <shd/utils/paths.hpp>
#include <shd/utils/static.hpp>

#include "e300_impl.hpp"
#include "e300_fifo_config.hpp"
#include "e300_fifo_config.hpp"

#include "e300_common.hpp"

#include <boost/filesystem.hpp>
#include <fstream>
#include <string>

#ifdef E300_NATIVE
namespace shd { namespace smini { namespace e300 {

namespace common {

void load_fpga_image(const std::string &path)
{
    if (not boost::filesystem::exists("/dev/xdevcfg"))
        ::system("mknod /dev/xdevcfg c 259 0");

    SHD_MSG(status) << "Loading FPGA image: " << path << "..." << std::flush;

    std::ifstream fpga_file(path.c_str(), std::ios_base::binary);
    SHD_ASSERT_THROW(fpga_file.good());

    std::FILE *wfile;
    wfile = std::fopen("/dev/xdevcfg", "wb");
    SHD_ASSERT_THROW(!(wfile == NULL));

    char buff[16384]; // devcfg driver can't handle huge writes
    do {
        fpga_file.read(buff, sizeof(buff));
        std::fwrite(buff, 1, size_t(fpga_file.gcount()), wfile);
    } while (fpga_file);

    fpga_file.close();
    std::fclose(wfile);

    SHD_MSG(status) << " done" << std::endl;
}

static bool e300_image_loader(const image_loader::image_loader_args_t &image_loader_args) {
    // Make sure this is an E3x0 and we don't want to use anything connected
    shd::device_addrs_t devs = e300_find(image_loader_args.args);
    if(devs.size() == 0 or !image_loader_args.load_fpga) return false;

    std::string fpga_filename, idle_image; // idle_image never used, just needed for function
    if(image_loader_args.fpga_path == "") {
        get_e3x0_fpga_images(devs[0], fpga_filename, idle_image);
    }
    else {
        if(not boost::filesystem::exists(image_loader_args.fpga_path)) {
            throw shd::runtime_error(str(boost::format("The path \"%s\" does not exist.")
                                         % image_loader_args.fpga_path));
        }
        else fpga_filename = image_loader_args.fpga_path;
    }

    load_fpga_image(fpga_filename);
    return true;
}

SHD_STATIC_BLOCK(register_e300_image_loader) {
    std::string recovery_instructions = "The default FPGA image will be loaded the next "
                                        "time SHD uses this device.";

    image_loader::register_image_loader("e3x0", e300_image_loader, recovery_instructions);
}

}

}}}

#else
namespace shd { namespace smini { namespace e300 {

namespace common {

void load_fpga_image(const std::string&)
{
    throw shd::assertion_error("load_fpga_image() !E300_NATIVE");
}

}

}}}
#endif
