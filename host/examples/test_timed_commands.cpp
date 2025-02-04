//
// Copyright 2012 Ettus Research LLC
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
#include <iostream>
#include <complex>

namespace po = boost::program_options;

int SHD_SAFE_MAIN(int argc, char *argv[]){
    shd::set_thread_priority_safe();

    //variables to be set by po
    std::string args;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "single shd device address args")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("SHD Test Timed Commands %s") % desc << std::endl;
        return ~0;
    }

    //create a smini device
    std::cout << std::endl;
    std::cout << boost::format("Creating the smini device with: %s...") % args << std::endl;
    shd::smini::multi_smini::sptr smini = shd::smini::multi_smini::make(args);
    std::cout << boost::format("Using Device: %s") % smini->get_pp_string() << std::endl;

    //check if timed commands are supported
    std::cout << std::endl;
    std::cout << "Testing support for timed commands on this hardware... " << std::flush;
    try{
        smini->set_command_time(shd::time_spec_t(0.0));
        smini->clear_command_time();
    }
    catch (const std::exception &e){
        std::cout << "fail" << std::endl;
        std::cerr << "Got exception: " << e.what() << std::endl;
        std::cerr << "Timed commands are not supported on this hardware." << std::endl;
        return ~0;
    }
    std::cout << "pass" << std::endl;

    //readback time really fast, time diff is small
    std::cout << std::endl;
    std::cout << "Perform fast readback of registers:" << std::endl;
    shd::time_spec_t total_time;
    for (size_t i = 0; i < 100; i++){
        const shd::time_spec_t t0 = smini->get_time_now();
        const shd::time_spec_t t1 = smini->get_time_now();
        total_time += (t1-t0);
    }
    std::cout << boost::format(
        " Difference between paired reads: %f us"
    ) % (total_time.get_real_secs()/100*1e6) << std::endl;

    //test timed control command
    //issues get_time_now() command twice a fixed time apart
    //outputs difference for each response time vs. the expected time
    //and difference between actual and expected time deltas
    std::cout << std::endl;
    std::cout << "Testing control timed command:" << std::endl;
    const shd::time_spec_t span = shd::time_spec_t(0.1);
    const shd::time_spec_t now = smini->get_time_now();
    const shd::time_spec_t cmd_time1 = now + shd::time_spec_t(0.1);
    const shd::time_spec_t cmd_time2 = cmd_time1 + span;
    smini->set_command_time(cmd_time1);
    shd::time_spec_t response_time1 = smini->get_time_now();
    smini->set_command_time(cmd_time2);
    shd::time_spec_t response_time2 = smini->get_time_now();
    smini->clear_command_time();
    std::cout << boost::format(
        " Span      : %f us\n"
        " Now       : %f us\n"
        " Response 1: %f us\n"
        " Response 2: %f us"
    ) % (span.get_real_secs()*1e6) % (now.get_real_secs()*1e6) % (response_time1.get_real_secs()*1e6) % (response_time2.get_real_secs()*1e6) << std::endl;
    std::cout << boost::format(
        " Difference of response time 1: %f us"
    ) % ((response_time1 - cmd_time1).get_real_secs()*1e6) << std::endl;
    std::cout << boost::format(
        " Difference of response time 2: %f us"
    ) % ((response_time2 - cmd_time2).get_real_secs()*1e6) << std::endl;
    std::cout << boost::format(
        " Difference between actual and expected time delta: %f us"
    ) % ((response_time2 - response_time1 - span).get_real_secs()*1e6) << std::endl;

    //use a timed command to start a stream at a specific time
    //this is not the right way start streaming at time x,
    //but it should approximate it within control RTT/2
    //setup streaming
    std::cout << std::endl;
    std::cout << "About to start streaming using timed command:" << std::endl;

    //create a receive streamer
    shd::stream_args_t stream_args("fc32"); //complex floats
    shd::rx_streamer::sptr rx_stream = smini->get_rx_stream(stream_args);

    shd::stream_cmd_t stream_cmd(shd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);
    stream_cmd.num_samps = 100;
    stream_cmd.stream_now = false;
    const shd::time_spec_t stream_time = smini->get_time_now() + shd::time_spec_t(0.1);
    stream_cmd.time_spec = stream_time;
    rx_stream->issue_stream_cmd(stream_cmd);

    //meta-data will be filled in by recv()
    shd::rx_metadata_t md;

    //allocate buffer to receive with samples
    std::vector<std::complex<float> > buff(stream_cmd.num_samps);

    const size_t num_rx_samps = rx_stream->recv(&buff.front(), buff.size(), md, 1.0);
    if (md.error_code != shd::rx_metadata_t::ERROR_CODE_NONE){
        throw std::runtime_error(str(boost::format(
            "Receiver error %s"
        ) % md.strerror()));
    }
    std::cout << boost::format(
        " Received packet: %u samples, %u full secs, %f frac secs"
    ) % num_rx_samps % md.time_spec.get_full_secs() % md.time_spec.get_frac_secs() << std::endl;
    std::cout << boost::format(
        " Stream time was: %u full secs, %f frac secs"
    ) % stream_time.get_full_secs() % stream_time.get_frac_secs() << std::endl;
    std::cout << boost::format(
        " Difference between stream time and first packet: %f us"
    ) % ((md.time_spec-stream_time).get_real_secs()*1e6) << std::endl;

    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;

    return EXIT_SUCCESS;
}
