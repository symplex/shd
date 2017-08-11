//
// Copyright 2012,2014-2016 Ettus Research LLC
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

#include <shd/utils/paths.hpp>
#include <shd/utils/thread_priority.hpp>
#include <shd/utils/safe_main.hpp>
#include <shd/smini/multi_smini.hpp>
#include <shd/smini_clock/multi_smini_clock.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <complex>
#include <boost/thread.hpp>
#include <string>
#include <cmath>
#include <ctime>
#include <cstdlib>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

const size_t WARMUP_TIMEOUT_MS = 30000;

void print_notes(void) {
  // Helpful notes
  std::cout << boost::format("**************************************Helpful Notes on Clock/PPS Selection**************************************\n");
  std::cout << boost::format("As you can see, the default 10 MHz Reference and 1 PPS signals are now from the GPSDO.\n");
  std::cout << boost::format("If you would like to use the internal reference(TCXO) in other applications, you must configure that explicitly.\n");
  std::cout << boost::format("****************************************************************************************************************\n");
}

int query_clock_sensors(const std::string &args) {
  std::cout << boost::format("\nCreating the clock device with: %s...\n") % args;
  shd::smini_clock::multi_smini_clock::sptr clock = shd::smini_clock::multi_smini_clock::make(args);

  //Verify GPS sensors are present
  std::vector<std::string> sensor_names = clock->get_sensor_names(0);
  if(std::find(sensor_names.begin(), sensor_names.end(), "gps_locked") == sensor_names.end()) {
    std::cout << boost::format("\ngps_locked sensor not found.  This could mean that this unit does not have a GPSDO.\n\n");
    return EXIT_FAILURE;
  }

  // Print NMEA strings
  try {
      shd::sensor_value_t gga_string = clock->get_sensor("gps_gpgga");
      shd::sensor_value_t rmc_string = clock->get_sensor("gps_gprmc");
      shd::sensor_value_t servo_string = clock->get_sensor("gps_servo");
      std::cout << boost::format("\nPrinting available NMEA strings:\n");
      std::cout << boost::format("%s\n%s\n") % gga_string.to_pp_string() % rmc_string.to_pp_string();
      std::cout << boost::format("\nPrinting GPS servo status:\n");
      std::cout << boost::format("%s\n\n") % servo_string.to_pp_string();
  } catch (shd::lookup_error &e) {
      std::cout << "NMEA strings not implemented for this device." << std::endl;
  }
  std::cout << boost::format("GPS Epoch time: %.5f seconds\n") % clock->get_sensor("gps_time").to_real();
  std::cout << boost::format("PC Clock time:  %.5f seconds\n") % time(NULL);

  //finished
  std::cout << boost::format("\nDone!\n\n");

  return EXIT_SUCCESS;
}

int SHD_SAFE_MAIN(int argc, char *argv[]){
  shd::set_thread_priority_safe();

  std::string args;

  //Set up program options
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "help message")
    ("args", po::value<std::string>(&args)->default_value(""), "Device address arguments specifying a single SMINI")
    ("clock", "query a clock device's sensors")
    ;
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  //Print the help message
  if (vm.count("help")) {
      std::cout << boost::format("Query GPSDO Sensors, try to lock the reference oscillator to the GPS disciplined clock, and set the device time to GPS time")
          << std::endl
          << std::endl
          << desc;
      return EXIT_FAILURE;
  }

  //If specified, query a clock device instead
  if(vm.count("clock")) {
      return query_clock_sensors(args);
  }

  //Create a SMINI device
  std::cout << boost::format("\nCreating the SMINI device with: %s...\n") % args;
  shd::smini::multi_smini::sptr smini = shd::smini::multi_smini::make(args);
  std::cout << boost::format("Using Device: %s\n") % smini->get_pp_string();

  //Verify GPS sensors are present (i.e. EEPROM has been burnt)
  std::vector<std::string> sensor_names = smini->get_mboard_sensor_names(0);

  if (std::find(sensor_names.begin(), sensor_names.end(), "gps_locked") == sensor_names.end()) {
      std::cout << boost::format("\ngps_locked sensor not found.  This could mean that you have not installed the GPSDO correctly.\n\n");
      std::cout << boost::format("Visit one of these pages if the problem persists:\n");
      std::cout << boost::format(" * N2X0/E1X0: http://files.ettus.com/manual/page_gpsdo.html\n");
      std::cout << boost::format(" * X3X0: http://files.ettus.com/manual/page_gpsdo_x3x0.html\n\n");
      return EXIT_FAILURE;
  }

  std::cout << "\nSetting the reference clock source to \"gpsdo\"...\n";
  try {
      smini->set_clock_source("gpsdo");
  } catch (shd::value_error &e) {
      std::cout << "could not set the clock source to \"gpsdo\"; error was:" <<std::endl;
      std::cout << e.what() << std::endl;
      std::cout << "trying \"external\"..." <<std::endl;
      try{
          smini->set_clock_source("external");
      } catch (shd::value_error&) {
          std::cout << "\"external\" failed, too." << std::endl;
      }
  }
  std::cout<< std::endl << "Clock source is now " << smini->get_clock_source(0) << std::endl;

  //Check for 10 MHz lock
  if(std::find(sensor_names.begin(), sensor_names.end(), "ref_locked") != sensor_names.end()) {
      shd::sensor_value_t ref_locked = smini->get_mboard_sensor("ref_locked",0);
      for (size_t i = 0; not ref_locked.to_bool() and i < 100; i++) {
          boost::this_thread::sleep(boost::posix_time::milliseconds(100));
          ref_locked = smini->get_mboard_sensor("ref_locked",0);
      }
      if(not ref_locked.to_bool()) {
          std::cout << boost::format("SMINI NOT Locked to GPSDO 10 MHz Reference.\n");
          std::cout << boost::format("Double check installation instructions (N2X0/E1X0 only): https://www.ettus.com/content/files/gpsdo-kit_4.pdf\n\n");
          return EXIT_FAILURE;
      } else {
          std::cout << boost::format("SMINI Locked to GPSDO 10 MHz Reference.\n");
      }
  } else {
      std::cout << boost::format("ref_locked sensor not present on this board.\n");
  }

  // Explicitly set time source to gpsdo
  try {
      smini->set_time_source("gpsdo");
  } catch (shd::value_error &e) {
      std::cout << "could not set the time source to \"gpsdo\"; error was:" <<std::endl;
      std::cout << e.what() << std::endl;
      std::cout << "trying \"external\"..." <<std::endl;
      try {
          smini->set_time_source("external");
      } catch (shd::value_error&) {
          std::cout << "\"external\" failed, too." << std::endl;
      }
  }
  std::cout << std::endl << "Time source is now " << smini->get_time_source(0) << std::endl;

  print_notes();

  // The TCXO has a long warm up time, so wait up to 30 seconds for sensor data to show up
  std::cout << "Waiting for the GPSDO to warm up..." << std::flush;
  boost::system_time exit_time = boost::get_system_time() +
              boost::posix_time::milliseconds(WARMUP_TIMEOUT_MS);
  while (boost::get_system_time() < exit_time) {
      try {
          smini->get_mboard_sensor("gps_locked",0);
          break;
      } catch (std::exception &) {}
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
      std::cout << "." << std::flush;
  }
  std::cout << std::endl;
  try {
      smini->get_mboard_sensor("gps_locked",0);
  } catch (std::exception &) {
      std::cout << "No response from GPSDO in 30 seconds" << std::endl;
      return EXIT_FAILURE;
  }
  std::cout << "The GPSDO is warmed up and talking." << std::endl;

  //Check for GPS lock
  shd::sensor_value_t gps_locked = smini->get_mboard_sensor("gps_locked",0);;
  if(not gps_locked.to_bool()) {
      std::cout << boost::format("\nGPS does not have lock. Wait a few minutes and try again.\n");
      std::cout << boost::format("NMEA strings and device time may not be accurate until lock is achieved.\n\n");
  } else {
      std::cout << boost::format("GPS Locked");
  }

  //Check PPS and compare SHD device time to GPS time
  shd::sensor_value_t gps_time = smini->get_mboard_sensor("gps_time");
  shd::time_spec_t last_pps_time = smini->get_time_last_pps();

  //we only care about the full seconds
  signed gps_seconds = gps_time.to_int();
  long long pps_seconds = last_pps_time.to_ticks(1.0);

  if(pps_seconds != gps_seconds) {
      std::cout << "\nTrying to align the device time to GPS time..."
                << std::endl;
      //set the device time to the GPS time
      //getting the GPS time returns just after the PPS edge, so just add a
      //second and set the device time at the next PPS edge
      smini->set_time_next_pps(shd::time_spec_t(gps_time.to_int() + 1.0));
      //allow some time to make sure the PPS has come…
      boost::this_thread::sleep(boost::posix_time::milliseconds(1100));
      //…then ask
      gps_seconds = smini->get_mboard_sensor("gps_time").to_int();
      pps_seconds = smini->get_time_last_pps().to_ticks(1.0);
  }

  if (pps_seconds == gps_seconds) {
      std::cout << boost::format("GPS and SHD Device time are aligned.\n");
  } else {
      std::cout << boost::format("Could not align SHD Device time to GPS time. Giving up.\n");
  }
  std::cout << boost::format("last_pps: %ld vs gps: %ld.")
            % pps_seconds % gps_seconds
            << std::endl;

  //print NMEA strings
  try {
      shd::sensor_value_t gga_string = smini->get_mboard_sensor("gps_gpgga");
      shd::sensor_value_t rmc_string = smini->get_mboard_sensor("gps_gprmc");
      std::cout << boost::format("Printing available NMEA strings:\n");
      std::cout << boost::format("%s\n%s\n") % gga_string.to_pp_string() % rmc_string.to_pp_string();
  } catch (shd::lookup_error&) {
      std::cout << "NMEA strings not implemented for this device." << std::endl;
  }
  std::cout << boost::format("GPS Epoch time at last PPS: %.5f seconds\n") % smini->get_mboard_sensor("gps_time").to_real();
  std::cout << boost::format("SHD Device time last PPS:   %.5f seconds\n") % (smini->get_time_last_pps().get_real_secs());
  std::cout << boost::format("SHD Device time right now:  %.5f seconds\n") % (smini->get_time_now().get_real_secs());
  std::cout << boost::format("PC Clock time:              %.5f seconds\n") % time(NULL);

  //finished
  std::cout << boost::format("\nDone!\n\n");

  return EXIT_SUCCESS;
}
