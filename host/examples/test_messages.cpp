//
// Copyright 2010-2013 Ettus Research LLC
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

#include <shd/config.hpp>
#include <shd/utils/thread_priority.hpp>
#include <shd/utils/safe_main.hpp>
#include <shd/utils/static.hpp>
#include <shd/types/stream_cmd.hpp>
#include <shd/smini/multi_smini.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <cstdlib>
#include <ctime>
#include <complex>
#include <iostream>

namespace po = boost::program_options;

/*!
 * Test the late command message:
 *    Issue a stream command with a time that is in the past.
 *    We expect to get an inline late command message.
 */
bool test_late_command_message(shd::smini::multi_smini::sptr smini, shd::rx_streamer::sptr rx_stream, shd::tx_streamer::sptr){
    std::cout << "Test late command message... " << std::flush;

    smini->set_time_now(shd::time_spec_t(200.0)); //set time

    shd::stream_cmd_t stream_cmd(shd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);
    stream_cmd.num_samps = rx_stream->get_max_num_samps();
    stream_cmd.stream_now = false;
    stream_cmd.time_spec = shd::time_spec_t(100.0); //time in the past
    rx_stream->issue_stream_cmd(stream_cmd);

    std::vector<std::complex<float> > buff(rx_stream->get_max_num_samps());
    shd::rx_metadata_t md;

    const size_t nsamps = rx_stream->recv(
        &buff.front(), buff.size(), md
    );

    switch(md.error_code){
    case shd::rx_metadata_t::ERROR_CODE_LATE_COMMAND:
        std::cout << boost::format(
            "success:\n"
            "    Got error code late command message.\n"
        ) << std::endl;
        return true;

    case shd::rx_metadata_t::ERROR_CODE_TIMEOUT:
        std::cout << boost::format(
            "failed:\n"
            "    Inline message recv timed out.\n"
        ) << std::endl;
        return false;

    default:
        std::cout << boost::format(
            "failed:\n"
            "    Got unexpected error code 0x%x (%s), nsamps %u.\n"
        ) % md.error_code % md.strerror() % nsamps << std::endl;
        return false;
    }
}

/*!
 * Test the broken chain message:
 *    Issue a stream command with num samps and more.
 *    We expect to get an inline broken chain message.
 */
bool test_broken_chain_message(SHD_UNUSED(shd::smini::multi_smini::sptr smini), shd::rx_streamer::sptr rx_stream, shd::tx_streamer::sptr){
    std::cout << "Test broken chain message... " << std::flush;

    shd::stream_cmd_t stream_cmd(shd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_MORE);
    stream_cmd.stream_now = true;
    stream_cmd.num_samps = rx_stream->get_max_num_samps();
    rx_stream->issue_stream_cmd(stream_cmd);

    std::vector<std::complex<float> > buff(rx_stream->get_max_num_samps());
    shd::rx_metadata_t md;

    rx_stream->recv( //once for the requested samples
        &buff.front(), buff.size(), md
    );

    rx_stream->recv( //again for the inline message
        &buff.front(), buff.size(), md
    );

    switch(md.error_code){
    case shd::rx_metadata_t::ERROR_CODE_BROKEN_CHAIN:
        std::cout << boost::format(
            "success:\n"
            "    Got error code broken chain message.\n"
        ) << std::endl;
        return true;

    case shd::rx_metadata_t::ERROR_CODE_TIMEOUT:
        std::cout << boost::format(
            "failed:\n"
            "    Inline message recv timed out.\n"
        ) << std::endl;
        return false;

    default:
        std::cout << boost::format(
            "failed:\n"
            "    Got unexpected error code 0x%x (%s).\n"
        ) % md.error_code % md.strerror() << std::endl;
        return false;
    }
}

/*!
 * Test the burst ack message:
 *    Send a burst of many samples that will fragment internally.
 *    We expect to get an burst ack async message.
 */
bool test_burst_ack_message(shd::smini::multi_smini::sptr, shd::rx_streamer::sptr, shd::tx_streamer::sptr tx_stream){
    std::cout << "Test burst ack message... " << std::flush;

    shd::tx_metadata_t md;
    md.start_of_burst = true;
    md.end_of_burst   = true;
    md.has_time_spec  = false;

    //3 times max-sps guarantees a SOB, no burst, and EOB packet
    std::vector<std::complex<float> > buff(tx_stream->get_max_num_samps()*3);
    tx_stream->send(&buff.front(), buff.size(), md);

    shd::async_metadata_t async_md;
    if (not tx_stream->recv_async_msg(async_md)){
        std::cout << boost::format(
            "failed:\n"
            "    Async message recv timed out.\n"
        ) << std::endl;
        return false;
    }

    switch(async_md.event_code){
    case shd::async_metadata_t::EVENT_CODE_BURST_ACK:
        std::cout << boost::format(
            "success:\n"
            "    Got event code burst ack message.\n"
        ) << std::endl;
        return true;

    default:
        std::cout << boost::format(
            "failed:\n"
            "    Got unexpected event code 0x%x.\n"
        ) % async_md.event_code << std::endl;
        return false;
    }
}

/*!
 * Test the underflow message:
 *    Send a start of burst packet with no following end of burst.
 *    We expect to get an underflow(within a burst) async message.
 */
bool test_underflow_message(shd::smini::multi_smini::sptr, shd::rx_streamer::sptr, shd::tx_streamer::sptr tx_stream){
    std::cout << "Test underflow message... " << std::flush;

    shd::tx_metadata_t md;
    md.start_of_burst = true;
    md.end_of_burst   = false;
    md.has_time_spec  = false;

    std::vector< std::complex<float> > buff(tx_stream->get_max_num_samps());
    tx_stream->send(&buff.front(), buff.size(), md);

    shd::async_metadata_t async_md;
    if (not tx_stream->recv_async_msg(async_md, 1)){
        std::cout << boost::format(
            "failed:\n"
            "    Async message recv timed out.\n"
        ) << std::endl;
        return false;
    }

    switch(async_md.event_code){
    case shd::async_metadata_t::EVENT_CODE_UNDERFLOW:
        std::cout << boost::format(
            "success:\n"
            "    Got event code underflow message.\n"
        ) << std::endl;
        return true;

    default:
        std::cout << boost::format(
            "failed:\n"
            "    Got unexpected event code 0x%x.\n"
        ) % async_md.event_code << std::endl;
        return false;
    }
}

/*!
 * Test the time error message:
 *    Send a burst packet that occurs at a time in the past.
 *    We expect to get a time error async message.
 */
bool test_time_error_message(shd::smini::multi_smini::sptr smini, shd::rx_streamer::sptr, shd::tx_streamer::sptr tx_stream){
    std::cout << "Test time error message... " << std::flush;

    shd::tx_metadata_t md;
    md.start_of_burst = true;
    md.end_of_burst   = true;
    md.has_time_spec  = true;
    md.time_spec      = shd::time_spec_t(100.0); //send at 100s

    smini->set_time_now(shd::time_spec_t(200.0)); //time at 200s

    std::vector< std::complex<float> > buff(tx_stream->get_max_num_samps());
    tx_stream->send(&buff.front(), buff.size(), md);

    shd::async_metadata_t async_md;
    if (not tx_stream->recv_async_msg(async_md)){
        std::cout << boost::format(
            "failed:\n"
            "    Async message recv timed out.\n"
        ) << std::endl;
        return false;
    }

    switch(async_md.event_code){
    case shd::async_metadata_t::EVENT_CODE_TIME_ERROR:
        std::cout << boost::format(
            "success:\n"
            "    Got event code time error message.\n"
        ) << std::endl;
        return true;

    default:
        std::cout << boost::format(
            "failed:\n"
            "    Got unexpected event code 0x%x.\n"
        ) % async_md.event_code << std::endl;
        return false;
    }
}

void flush_async(shd::tx_streamer::sptr tx_stream){
    shd::async_metadata_t async_md;
    while (tx_stream->recv_async_msg(async_md)){}
}

void flush_recv(shd::rx_streamer::sptr rx_stream){
    shd::stream_cmd_t stream_cmd(shd::stream_cmd_t::STREAM_MODE_NUM_SAMPS_AND_DONE);
    stream_cmd.num_samps = rx_stream->get_max_num_samps()*3;
    stream_cmd.stream_now = true;
    rx_stream->issue_stream_cmd(stream_cmd);

    std::vector<std::complex<float> > buff(stream_cmd.num_samps);
    shd::rx_metadata_t md;

    do{
        rx_stream->recv(&buff.front(), buff.size(), md);
    } while (md.error_code != shd::rx_metadata_t::ERROR_CODE_TIMEOUT);
}

int SHD_SAFE_MAIN(int argc, char *argv[]){
    shd::set_thread_priority_safe();

    //variables to be set by po
    std::string args;
    size_t ntests;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args",   po::value<std::string>(&args)->default_value(""), "multi shd device address args")
        ("ntests", po::value<size_t>(&ntests)->default_value(50),    "number of tests to run")
        ("test-chain", "Run broken chain tests")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("SHD Test Messages %s") % desc << std::endl;
        return ~0;
    }

    //create a smini device
    std::cout << std::endl;
    std::cout << boost::format("Creating the smini device with: %s...") % args << std::endl;
    shd::smini::multi_smini::sptr smini = shd::smini::multi_smini::make(args);
    std::cout << boost::format("Using Device: %s") % smini->get_pp_string() << std::endl;

    //create RX and TX streamers
    shd::stream_args_t stream_args("fc32"); //complex floats
    shd::rx_streamer::sptr rx_stream = smini->get_rx_stream(stream_args);
    shd::tx_streamer::sptr tx_stream = smini->get_tx_stream(stream_args);

    //------------------------------------------------------------------
    // begin messages test
    //------------------------------------------------------------------
    static shd::dict<std::string, boost::function<bool(shd::smini::multi_smini::sptr, shd::rx_streamer::sptr, shd::tx_streamer::sptr)> >
        tests = boost::assign::map_list_of
        ("Test Burst ACK ", &test_burst_ack_message)
        ("Test Underflow ", &test_underflow_message)
        ("Test Time Error", &test_time_error_message)
        ("Test Late Command", &test_late_command_message)
    ;

    if (vm.count("test-chain")) {
        tests["Test Broken Chain"] = &test_broken_chain_message;
    }

    //init result counts
    shd::dict<std::string, size_t> failures, successes;
    BOOST_FOREACH(const std::string &key, tests.keys()){
        failures[key] = 0;
        successes[key] = 0;
    }

    //run the tests, pick at random
    std::srand((unsigned int) time(NULL));
    for (size_t n = 0; n < ntests; n++){
        std::string key = tests.keys()[std::rand() % tests.size()];
        bool pass = tests[key](smini, rx_stream, tx_stream);

        flush_recv(rx_stream);
        flush_async(tx_stream);

        //store result
        if (pass) successes[key]++;
        else      failures[key]++;
    }

    //print the result summary
    bool any_failure = false;
    std::cout << std::endl << "Summary:" << std::endl << std::endl;
    BOOST_FOREACH(const std::string &key, tests.keys()){
        std::cout << boost::format(
            "%s   ->   %3u successes, %3u failures"
        ) % key % successes[key] % failures[key] << std::endl;
        any_failure = any_failure or bool(failures[key]);
    }

    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;

    return any_failure ? EXIT_FAILURE : EXIT_SUCCESS;
}
