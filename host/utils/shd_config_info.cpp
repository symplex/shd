//
// Copyright 2015-2016 National Instruments Corp.
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

#include <shd/build_info.hpp>
#include <shd/version.hpp>
#include <shd/utils/paths.hpp>
#include <shd/utils/safe_main.hpp>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int SHD_SAFE_MAIN(int argc, char* argv[]) {
    // Program Options
    po::options_description desc("Allowed Options");
    desc.add_options()
        ("build-date",         "Print build date")
        ("c-compiler",         "Print C compiler")
        ("cxx-compiler",       "Print C++ compiler")
        ("c-flags",            "Print C compiler flags")
        ("cxx-flags",          "Print C++ compiler flags")
        ("enabled-components", "Print built-time enabled components")
        ("install-prefix",     "Print install prefix")
        ("boost-version",      "Print Boost version")
        ("libusb-version",     "Print libusb version")
        ("pkg-path",           "Print pkg path")
        ("images-dir",         "Print images dir")
        ("abi-version",        "Print ABI version string")
        ("print-all",          "Print everything")
        ("version",            "Print this SHD build's version")
        ("help",               "Print help message")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // Print the help message
    if(vm.count("help") > 0) {
        std::cout << boost::format("SHD Config Info - %s") % desc << std::endl;
        return EXIT_FAILURE;
    }

    bool print_all = (vm.count("print-all") > 0);

    if(vm.count("version") > 0 or print_all) {
        std::cout << "SHD " << shd::get_version_string() << std::endl;
    }
    if(vm.count("build-date") > 0 or print_all) {
        std::cout << "Build date: " << shd::build_info::build_date() << std::endl;
    }
    if(vm.count("c-compiler") > 0 or print_all) {
        std::cout << "C compiler: " << shd::build_info::c_compiler() << std::endl;
    }
    if(vm.count("cxx-compiler") > 0 or print_all) {
        std::cout << "C++ compiler: " << shd::build_info::cxx_compiler() << std::endl;
    }
    if(vm.count("c-flags") > 0 or print_all) {
        std::cout << "C flags: " << shd::build_info::c_flags() << std::endl;
    }
    if(vm.count("cxx-flags") > 0 or print_all) {
        std::cout << "C++ flags: " << shd::build_info::cxx_flags() << std::endl;
    }
    if(vm.count("enabled-components") > 0 or print_all) {
        std::cout << "Enabled components: " << shd::build_info::enabled_components() << std::endl;
    }
    if(vm.count("install-prefix") > 0 or print_all) {
        std::cout << "Install prefix: " << shd::build_info::install_prefix() << std::endl;
    }
    if(vm.count("boost-version") > 0 or print_all) {
        std::cout << "Boost version: " << shd::build_info::boost_version() << std::endl;
    }
    if(vm.count("libusb-version") > 0 or print_all) {
        std::string _libusb_version = shd::build_info::libusb_version();
        std::cout << "Libusb version: " << (_libusb_version.empty() ? "N/A" : _libusb_version) << std::endl;
    }
    if(vm.count("pkg-path") > 0 or print_all) {
        std::cout << "Package path: " << shd::get_pkg_path() << std::endl;
    }
    if(vm.count("images-dir") > 0 or print_all) {
        std::cout << "Images directory: " << shd::get_images_dir("") << std::endl;
    }
    if(vm.count("abi-version") > 0 or print_all) {
        std::cout << "ABI version string: " << shd::get_abi_string() << std::endl;
    }

    return EXIT_SUCCESS;
}
