//
// Copyright 2014 Ettus Research LLC
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

#include <iostream>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/thread.hpp>

#include <shd/device.hpp>
#include <shd/exception.hpp>
#include <shd/smini_clock/multi_smini_clock.hpp>
#include <shd/types/time_spec.hpp>
#include <shd/smini/multi_smini.hpp>
#include <shd/utils/safe_main.hpp>
#include <shd/utils/thread_priority.hpp>

namespace po = boost::program_options;

using namespace shd::smini_clock;
using namespace shd::smini;

void get_smini_time(multi_smini::sptr smini, size_t mboard, std::vector<time_t> *times){
    (*times)[mboard] = smini->get_time_now(mboard).get_full_secs();
}

int SHD_SAFE_MAIN(int argc, char *argv[]){
    shd::set_thread_priority_safe();

    //Variables to be set by command line options
    std::string clock_args, smini_args;
    uint32_t max_interval, num_tests;

    //Set up program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "Display this help message")
        ("clock-args", po::value<std::string>(&clock_args), "Clock device arguments")
        ("smini-args", po::value<std::string>(&smini_args), "SMINI device arguments")
        ("max-interval", po::value<uint32_t>(&max_interval)->default_value(10000), "Maximum interval between comparisons (in ms)")
        ("num-tests", po::value<uint32_t>(&num_tests)->default_value(10), "Number of times to compare device times")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //Print the help message
    if (vm.count("help")){
        std::cout << std::endl << "Test Clock Synchronization" << std::endl << std::endl;

        std::cout << "This example shows how to use a clock device to" << std::endl
                  << "synchronize the time on multiple SMINI devices." << std::endl << std::endl;

        std::cout << desc << std::endl;
        return EXIT_SUCCESS;
    }

    //Create a Multi-SMINI-Clock device (currently OctoClock only)
    std::cout << boost::format("\nCreating the Clock device with: %s") % clock_args << std::endl;
    multi_smini_clock::sptr clock = multi_smini_clock::make(clock_args);

    //Make sure Clock configuration is correct
    if(clock->get_sensor("gps_detected").value == "false"){
        throw shd::runtime_error("No GPSDO detected on Clock.");
    }
    if(clock->get_sensor("using_ref").value != "internal"){
        throw shd::runtime_error("Clock must be using an internal reference.");
    }

    //Create a Multi-SMINI device
    std::cout << boost::format("\nCreating the SMINI device with: %s") % smini_args << std::endl;
    multi_smini::sptr smini = multi_smini::make(smini_args);

    //Store SMINI device serials for useful output
    std::vector<std::string> serials;
    for(size_t ch = 0; ch < smini->get_num_mboards(); ch++){
        serials.push_back(smini->get_smini_tx_info(ch)["mboard_serial"]);
    }

    std::cout << std::endl << "Checking SMINI devices for lock." << std::endl;
    bool all_locked = true;
    for(size_t ch = 0; ch < smini->get_num_mboards(); ch++){
        std::string ref_locked = smini->get_mboard_sensor("ref_locked",ch).value;
        std::cout << boost::format(" * %s: %s") % serials[ch] % ref_locked << std::endl;

        if(ref_locked != "true") all_locked = false;
    }
    if(not all_locked) std::cout << std::endl << "WARNING: One or more devices not locked." << std::endl;

    //Get GPS time to initially set SMINI devices
    std::cout << std::endl << "Querying Clock for time and setting SMINI times..." << std::endl << std::endl;
    time_t clock_time = clock->get_time();
    smini->set_time_next_pps(shd::time_spec_t(double(clock_time+1)));
    srand((unsigned int)time(NULL));

    std::cout << boost::format("Running %d comparisons at random intervals.") % num_tests << std::endl;
    uint32_t num_matches = 0;
    for(size_t i = 0; i < num_tests; i++){
        //Wait random time before querying
        uint16_t wait_time = rand() % max_interval;
        boost::this_thread::sleep(boost::posix_time::milliseconds(wait_time));

        //Get all times before output
        std::vector<time_t> smini_times(smini->get_num_mboards());
        boost::thread_group thread_group;
        clock_time = clock->get_time();
        for(size_t j = 0; j < smini->get_num_mboards(); j++){
            thread_group.create_thread(boost::bind(&get_smini_time, smini, j, &smini_times));
        }
        //Wait for threads to complete
        thread_group.join_all();

        std::cout << boost::format("Comparison #%d") % (i+1) << std::endl;
        bool all_match = true;
        std::cout << boost::format(" * Clock time: %d") % clock_time << std::endl;
        for(size_t j = 0; j < smini->get_num_mboards(); j++){
            std::cout << boost::format(" * %s time: %d") % serials[j] % smini_times[j] << std::endl;
            if(smini_times[j] != clock_time) all_match = false;
        }
        if(all_match) num_matches++;
    }

    std::cout << std::endl << boost::format("Number of matches: %d/%d") % num_matches % num_tests << std::endl;

    return EXIT_SUCCESS;
}
