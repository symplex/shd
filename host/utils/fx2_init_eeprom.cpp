//
// Copyright 2010,2014,2016 Ettus Research LLC
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

#include <shd/utils/safe_main.hpp>
#include <shd/device.hpp>
#include <shd/property_tree.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <iostream>
//#include <cstdlib>
#ifdef SHD_PLATFORM_LINUX
#include <fstream>
#include <unistd.h> // syscall constants
#include <fcntl.h> // O_NONBLOCK
#include <sys/syscall.h>
#include <cerrno>
#include <cstring> // for std::strerror
#endif //SHD_PLATFORM_LINUX

const std::string FX2_VENDOR_ID("0x04b4");
const std::string FX2_PRODUCT_ID("0x8613");

namespace po = boost::program_options;

int SHD_SAFE_MAIN(int argc, char *argv[]){
    std::string type;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("image", po::value<std::string>(), "BIN image file")
        ("vid", po::value<std::string>(), "VID of device to program")
        ("pid", po::value<std::string>(), "PID of device to program")
        ("type", po::value<std::string>(), "device type (smini1 or b100)")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("SMINI EEPROM initialization %s") % desc << std::endl;
        return EXIT_FAILURE;
    }

#ifdef SHD_PLATFORM_LINUX
    //can't find an uninitialized smini with this mystery usbtest in the way...
    std::string module("usbtest");
    std::ifstream modules("/proc/modules");
    bool module_found = false;
    std::string module_line;
    while(std::getline(modules, module_line) && (!module_found)) {
        module_found = boost::starts_with(module_line, module);
    }
    if(module_found) {
        std::cout << boost::format("Found the '%s' module. Unloading it.\n" ) % module;
        int fail = syscall(__NR_delete_module, module.c_str(), O_NONBLOCK);
        if(fail)
            std::cerr << ( boost::format("Removing the '%s' module failed with error '%s'.\n") % module % std::strerror(errno) );
    }
#endif //SHD_PLATFORM_LINUX

    //load the options into the address
    shd::device_addr_t device_addr;
    device_addr["type"] = type;
    if(vm.count("vid") or vm.count("pid") or vm.count("type")) {
        if(not (vm.count("vid") and vm.count("pid") and vm.count("type"))) {
            std::cerr << "ERROR: Must specify vid, pid, and type if specifying any of the three args" << std::endl;
        } else {
            device_addr["vid"] = vm["vid"].as<std::string>();
            device_addr["pid"] = vm["pid"].as<std::string>();
            device_addr["type"] = vm["type"].as<std::string>();
        }
    } else {
        device_addr["vid"] = FX2_VENDOR_ID;
        device_addr["pid"] = FX2_PRODUCT_ID;
    }

    //find and create a control transport to do the writing.

    shd::device_addrs_t found_addrs = shd::device::find(device_addr, shd::device::SMINI);

    if (found_addrs.size() == 0){
        std::cerr << "No SMINI devices found" << std::endl;
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < found_addrs.size(); i++){
        std::cout << "Writing EEPROM data..." << std::endl;
        //shd::device_addrs_t devs = shd::device::find(found_addrs[i]);
        shd::device::sptr dev = shd::device::make(found_addrs[i], shd::device::SMINI);
        shd::property_tree::sptr tree = dev->get_tree();
        tree->access<std::string>("/mboards/0/load_eeprom").set(vm["image"].as<std::string>());
    }


    std::cout << "Power-cycle the smini for the changes to take effect." << std::endl;
    return EXIT_SUCCESS;
}
