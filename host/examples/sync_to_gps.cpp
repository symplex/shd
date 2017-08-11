//
// Copyright 2016 Ettus Research
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
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <iostream>

namespace po = boost::program_options;

void print_notes(void)
{
    // Helpful notes
    std::cout << boost::format("**************************************Helpful Notes on Clock/PPS Selection**************************************\n");
    std::cout << boost::format("As you can see, the default 10 MHz Reference and 1 PPS signals are now from the GPSDO.\n");
    std::cout << boost::format("If you would like to use the internal reference(TCXO) in other applications, you must configure that explicitly.\n");
    std::cout << boost::format("You can no longer select the external SMAs for 10 MHz or 1 PPS signaling.\n");
    std::cout << boost::format("****************************************************************************************************************\n");
}

int SHD_SAFE_MAIN(int argc, char *argv[])
{
    shd::set_thread_priority_safe();

    std::string args;

    //Set up program options
    po::options_description desc("Allowed options");
    desc.add_options()
    ("help", "help message")
    ("args", po::value<std::string>(&args)->default_value(""), "SMINI device arguments")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //Print the help message
    if (vm.count("help"))
    {
        std::cout << boost::format("Synchronize SMINI to GPS %s") % desc << std::endl;
        return EXIT_FAILURE;
    }

    //Create a SMINI device
    std::cout << boost::format("\nCreating the SMINI device with: %s...\n") % args;
    shd::smini::multi_smini::sptr smini = shd::smini::multi_smini::make(args);
    std::cout << boost::format("Using Device: %s\n") % smini->get_pp_string();

    try
    {
        size_t num_mboards = smini->get_num_mboards();
        size_t num_gps_locked = 0;
        for (size_t mboard = 0; mboard < num_mboards; mboard++)
        {
            std::cout << "Synchronizing mboard " << mboard << ": " << smini->get_mboard_name(mboard) << std::endl;

            //Set references to GPSDO
            smini->set_clock_source("gpsdo", mboard);
            smini->set_time_source("gpsdo", mboard);

            std::cout << std::endl;
            print_notes();
            std::cout << std::endl;

            //Check for 10 MHz lock
            std::vector<std::string> sensor_names = smini->get_mboard_sensor_names(mboard);
            if(std::find(sensor_names.begin(), sensor_names.end(), "ref_locked") != sensor_names.end())
            {
                std::cout << "Waiting for reference lock..." << std::flush;
                bool ref_locked = false;
                for (int i = 0; i < 30 and not ref_locked; i++)
                {
                    ref_locked = smini->get_mboard_sensor("ref_locked", mboard).to_bool();
                    if (not ref_locked)
                    {
                        std::cout << "." << std::flush;
                        boost::this_thread::sleep(boost::posix_time::seconds(1));
                    }
                }
                if(ref_locked)
                {
                    std::cout << "LOCKED" << std::endl;
                } else {
                    std::cout << "FAILED" << std::endl;
                    std::cout << "Failed to lock to GPSDO 10 MHz Reference. Exiting." << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            else
            {
                std::cout << boost::format("ref_locked sensor not present on this board.\n");
            }

            //Wait for GPS lock
            bool gps_locked = smini->get_mboard_sensor("gps_locked", mboard).to_bool();
            if(gps_locked)
            {
                num_gps_locked++;
                std::cout << boost::format("GPS Locked\n");
            }
            else
            {
                std::cerr << "WARNING:  GPS not locked - time will not be accurate until locked" << std::endl;
            }

            //Set to GPS time
            shd::time_spec_t gps_time = shd::time_spec_t(time_t(smini->get_mboard_sensor("gps_time", mboard).to_int()));
            smini->set_time_next_pps(gps_time+1.0, mboard);

            //Wait for it to apply
            //The wait is 2 seconds because N-Series has a known issue where
            //the time at the last PPS does not properly update at the PPS edge
            //when the time is actually set.
            boost::this_thread::sleep(boost::posix_time::seconds(2));

            //Check times
            gps_time = shd::time_spec_t(time_t(smini->get_mboard_sensor("gps_time", mboard).to_int()));
            shd::time_spec_t time_last_pps = smini->get_time_last_pps(mboard);
            std::cout << "SMINI time: " << (boost::format("%0.9f") % time_last_pps.get_real_secs()) << std::endl;
            std::cout << "GPSDO time: " << (boost::format("%0.9f") % gps_time.get_real_secs()) << std::endl;
            if (gps_time.get_real_secs() == time_last_pps.get_real_secs())
                std::cout << std::endl << "SUCCESS: SMINI time synchronized to GPS time" << std::endl << std::endl;
            else
                std::cerr << std::endl << "ERROR: Failed to synchronize SMINI time to GPS time" << std::endl << std::endl;
        }

        if (num_gps_locked == num_mboards and num_mboards > 1)
        {
            //Check to see if all SMINI times are aligned
            //First, wait for PPS.
            shd::time_spec_t time_last_pps = smini->get_time_last_pps();
            while (time_last_pps == smini->get_time_last_pps())
            {
                boost::this_thread::sleep(boost::posix_time::milliseconds(1));
            }

            //Sleep a little to make sure all devices have seen a PPS edge
            boost::this_thread::sleep(boost::posix_time::milliseconds(200));

            //Compare times across all mboards
            bool all_matched = true;
            shd::time_spec_t mboard0_time = smini->get_time_last_pps(0);
            for (size_t mboard = 1; mboard < num_mboards; mboard++)
            {
                shd::time_spec_t mboard_time = smini->get_time_last_pps(mboard);
                if (mboard_time != mboard0_time)
                {
                    all_matched = false;
                    std::cerr << (boost::format("ERROR: Times are not aligned: SMINI 0=%0.9f, SMINI %d=%0.9f")
                                  % mboard0_time.get_real_secs()
                                  % mboard
                                  % mboard_time.get_real_secs()) << std::endl;
                }
            }
            if (all_matched)
            {
                std::cout << "SUCCESS: SMINI times aligned" << std::endl << std::endl;
            } else {
                std::cout << "ERROR: SMINI times are not aligned" << std::endl << std::endl;
            }
        }
    }
    catch (std::exception& e)
    {
        std::cout << boost::format("\nError: %s") % e.what();
        std::cout << boost::format("This could mean that you have not installed the GPSDO correctly.\n\n");
        std::cout << boost::format("Visit one of these pages if the problem persists:\n");
        std::cout << boost::format(" * N2X0/E1X0: http://files.ettus.com/manual/page_gpsdo.html");
        std::cout << boost::format(" * X3X0: http://files.ettus.com/manual/page_gpsdo_x3x0.html\n\n");
        std::cout << boost::format(" * E3X0: http://files.ettus.com/manual/page_smini_e3x0.html#e3x0_hw_gps\n\n");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
