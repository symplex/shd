//
// Copyright 2010-2011 Ettus Research LLC
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

#include <shd/utils/thread_priority.hpp>
#include <shd/utils/safe_main.hpp>
#include <shd/smini/multi_smini.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <complex>

namespace po = boost::program_options;

int SHD_SAFE_MAIN(int argc, char *argv[]){
    shd::set_thread_priority_safe();

    //variables to be set by po
    std::string args;
    std::string time_source;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "single shd device address args")
        ("source", po::value<std::string>(&time_source)->default_value(""), "the time source (gpsdo, external) or blank for default")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("SHD Test PPS Input %s") % desc << std::endl;
        std::cout
                << std::endl
                << "Tests if the PPS input signal is working. Will throw an error if not."
                << std::endl
                << std::endl;
        return ~0;
    }

    //create a smini device
    std::cout << std::endl;
    std::cout << boost::format("Creating the smini device with: %s...") % args << std::endl;
    shd::smini::multi_smini::sptr smini = shd::smini::multi_smini::make(args);
    std::cout << boost::format("Using Device: %s") % smini->get_pp_string() << std::endl;

    //sleep off if gpsdo detected and time next pps already set
    boost::this_thread::sleep(boost::posix_time::seconds(1));

    //set time source if specified
    if (not time_source.empty()) smini->set_time_source(time_source);

    //set the time at an unknown pps (will throw if no pps)
    std::cout << std::endl << "Attempt to detect the PPS and set the time..." << std::endl << std::endl;
    smini->set_time_unknown_pps(shd::time_spec_t(0.0));
    std::cout << std::endl << "Success!" << std::endl << std::endl;
    return EXIT_SUCCESS;
}
