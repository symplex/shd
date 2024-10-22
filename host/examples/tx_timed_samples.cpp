//
// Copyright 2010-2011,2014 Ettus Research LLC
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
#include <boost/thread/thread.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <complex>

namespace po = boost::program_options;

int SHD_SAFE_MAIN(int argc, char *argv[]){
    shd::set_thread_priority_safe();

    //variables to be set by po
    std::string args;
    std::string wire;
    double seconds_in_future;
    size_t total_num_samps;
    double rate;
    float ampl;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "single shd device address args")
        ("wire", po::value<std::string>(&wire)->default_value(""), "the over the wire type, sc16, sc8, etc")
        ("secs", po::value<double>(&seconds_in_future)->default_value(1.5), "number of seconds in the future to transmit")
        ("nsamps", po::value<size_t>(&total_num_samps)->default_value(10000), "total number of samples to transmit")
        ("rate", po::value<double>(&rate)->default_value(100e6/16), "rate of outgoing samples")
        ("ampl", po::value<float>(&ampl)->default_value(float(0.3)), "amplitude of each sample")
        ("dilv", "specify to disable inner-loop verbose")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("SHD TX Timed Samples %s") % desc << std::endl;
        return ~0;
    }

    bool verbose = vm.count("dilv") == 0;

    //create a smini device
    std::cout << std::endl;
    std::cout << boost::format("Creating the smini device with: %s...") % args << std::endl;
    shd::smini::multi_smini::sptr smini = shd::smini::multi_smini::make(args);
    std::cout << boost::format("Using Device: %s") % smini->get_pp_string() << std::endl;

    //set the tx sample rate
    std::cout << boost::format("Setting TX Rate: %f Msps...") % (rate/1e6) << std::endl;
    smini->set_tx_rate(rate);
    std::cout << boost::format("Actual TX Rate: %f Msps...") % (smini->get_tx_rate()/1e6) << std::endl << std::endl;

    std::cout << boost::format("Setting device timestamp to 0...") << std::endl;
    smini->set_time_now(shd::time_spec_t(0.0));

    //create a transmit streamer
    shd::stream_args_t stream_args("fc32", wire); //complex floats
    shd::tx_streamer::sptr tx_stream = smini->get_tx_stream(stream_args);

    //allocate buffer with data to send
    std::vector<std::complex<float> > buff(tx_stream->get_max_num_samps(), std::complex<float>(ampl, ampl));

    //setup metadata for the first packet
    shd::tx_metadata_t md;
    md.start_of_burst = false;
    md.end_of_burst = false;
    md.has_time_spec = true;
    md.time_spec = shd::time_spec_t(seconds_in_future);

    //the first call to send() will block this many seconds before sending:
    const double timeout = seconds_in_future + 0.1; //timeout (delay before transmit + padding)

    size_t num_acc_samps = 0; //number of accumulated samples
    while(num_acc_samps < total_num_samps){
        size_t samps_to_send = std::min(total_num_samps - num_acc_samps, buff.size());

        //send a single packet
        size_t num_tx_samps = tx_stream->send(
            &buff.front(), samps_to_send, md, timeout
        );

        //do not use time spec for subsequent packets
        md.has_time_spec = false;

        if (num_tx_samps < samps_to_send) std::cerr << "Send timeout..." << std::endl;
        if(verbose) std::cout << boost::format("Sent packet: %u samples") % num_tx_samps << std::endl;

        num_acc_samps += num_tx_samps;
    }

    //send a mini EOB packet
    md.end_of_burst   = true;
    tx_stream->send("", 0, md);

    std::cout << std::endl << "Waiting for async burst ACK... " << std::flush;
    shd::async_metadata_t async_md;
    bool got_async_burst_ack = false;
    //loop through all messages for the ACK packet (may have underflow messages in queue)
    while (not got_async_burst_ack and tx_stream->recv_async_msg(async_md, timeout)){
        got_async_burst_ack = (async_md.event_code == shd::async_metadata_t::EVENT_CODE_BURST_ACK);
    }
    std::cout << (got_async_burst_ack? "success" : "fail") << std::endl;

    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;

    return EXIT_SUCCESS;
}
