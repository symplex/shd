//
// Copyright 2011-2012,2014 Ettus Research LLC
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

#include <shd/types/tune_request.hpp>
#include <shd/utils/thread_priority.hpp>
#include <shd/utils/safe_main.hpp>
#include <shd/smini/multi_smini.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <fstream>
#include <complex>
#include <csignal>

namespace po = boost::program_options;

static bool stop_signal_called = false;
void sig_int_handler(int){stop_signal_called = true;}

template<typename samp_type> void send_from_file(
    shd::smini::multi_smini::sptr smini,
    const std::string &cpu_format,
    const std::string &wire_format,
    const std::string &file,
    size_t samps_per_buff
){

    //create a transmit streamer
    shd::stream_args_t stream_args(cpu_format, wire_format);
    shd::tx_streamer::sptr tx_stream = smini->get_tx_stream(stream_args);

    shd::tx_metadata_t md;
    md.start_of_burst = false;
    md.end_of_burst = false;
    std::vector<samp_type> buff(samps_per_buff);
    std::ifstream infile(file.c_str(), std::ifstream::binary);

    //loop until the entire file has been read

    while(not md.end_of_burst and not stop_signal_called){

        infile.read((char*)&buff.front(), buff.size()*sizeof(samp_type));
        size_t num_tx_samps = size_t(infile.gcount()/sizeof(samp_type));

        md.end_of_burst = infile.eof();

        tx_stream->send(&buff.front(), num_tx_samps, md);
    }

    infile.close();
}

int SHD_SAFE_MAIN(int argc, char *argv[]){
    shd::set_thread_priority_safe();

    //variables to be set by po
    std::string args, file, type, ant, subdev, ref, wirefmt;
    size_t spb;
    double rate, freq, gain, bw, delay, lo_off;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "multi shd device address args")
        ("file", po::value<std::string>(&file)->default_value("smini_samples.dat"), "name of the file to read binary samples from")
        ("type", po::value<std::string>(&type)->default_value("short"), "sample type: double, float, or short")
        ("spb", po::value<size_t>(&spb)->default_value(10000), "samples per buffer")
        ("rate", po::value<double>(&rate), "rate of outgoing samples")
        ("freq", po::value<double>(&freq), "RF center frequency in Hz")
        ("lo_off", po::value<double>(&lo_off), "Offset for frontend LO in Hz (optional)")
        ("gain", po::value<double>(&gain), "gain for the RF chain")
        ("ant", po::value<std::string>(&ant), "antenna selection")
        ("subdev", po::value<std::string>(&subdev), "subdevice specification")
        ("bw", po::value<double>(&bw), "analog frontend filter bandwidth in Hz")
        ("ref", po::value<std::string>(&ref)->default_value("internal"), "reference source (internal, external, mimo)")
        ("wirefmt", po::value<std::string>(&wirefmt)->default_value("sc16"), "wire format (sc8 or sc16)")
        ("delay", po::value<double>(&delay)->default_value(0.0), "specify a delay between repeated transmission of file")
        ("repeat", "repeatedly transmit file")
        ("int-n", "tune SMINI with integer-n tuning")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("SHD TX samples from file %s") % desc << std::endl;
        return ~0;
    }

    bool repeat = vm.count("repeat") > 0;

    //create a smini device
    std::cout << std::endl;
    std::cout << boost::format("Creating the smini device with: %s...") % args << std::endl;
    shd::smini::multi_smini::sptr smini = shd::smini::multi_smini::make(args);

    //Lock mboard clocks
    smini->set_clock_source(ref);

    //always select the subdevice first, the channel mapping affects the other settings
    if (vm.count("subdev")) smini->set_tx_subdev_spec(subdev);

    std::cout << boost::format("Using Device: %s") % smini->get_pp_string() << std::endl;

    //set the sample rate
    if (not vm.count("rate")){
        std::cerr << "Please specify the sample rate with --rate" << std::endl;
        return ~0;
    }
    std::cout << boost::format("Setting TX Rate: %f Msps...") % (rate/1e6) << std::endl;
    smini->set_tx_rate(rate);
    std::cout << boost::format("Actual TX Rate: %f Msps...") % (smini->get_tx_rate()/1e6) << std::endl << std::endl;

    //set the center frequency
    if (not vm.count("freq")){
        std::cerr << "Please specify the center frequency with --freq" << std::endl;
        return ~0;
    }
    std::cout << boost::format("Setting TX Freq: %f MHz...") % (freq/1e6) << std::endl;
    shd::tune_request_t tune_request;
    if(vm.count("lo_off")) tune_request = shd::tune_request_t(freq, lo_off);
    else tune_request = shd::tune_request_t(freq);
    if(vm.count("int-n")) tune_request.args = shd::device_addr_t("mode_n=integer");
    smini->set_tx_freq(tune_request);
    std::cout << boost::format("Actual TX Freq: %f MHz...") % (smini->get_tx_freq()/1e6) << std::endl << std::endl;

    //set the rf gain
    if (vm.count("gain")){
        std::cout << boost::format("Setting TX Gain: %f dB...") % gain << std::endl;
        smini->set_tx_gain(gain);
        std::cout << boost::format("Actual TX Gain: %f dB...") % smini->get_tx_gain() << std::endl << std::endl;
    }

    //set the analog frontend filter bandwidth
    if (vm.count("bw")){
        std::cout << boost::format("Setting TX Bandwidth: %f MHz...") % bw << std::endl;
        smini->set_tx_bandwidth(bw);
        std::cout << boost::format("Actual TX Bandwidth: %f MHz...") % smini->get_tx_bandwidth() << std::endl << std::endl;
    }

    //set the antenna
    if (vm.count("ant")) smini->set_tx_antenna(ant);

    boost::this_thread::sleep(boost::posix_time::seconds(1)); //allow for some setup time

    //Check Ref and LO Lock detect
    std::vector<std::string> sensor_names;
    sensor_names = smini->get_tx_sensor_names(0);
    if (std::find(sensor_names.begin(), sensor_names.end(), "lo_locked") != sensor_names.end()) {
        shd::sensor_value_t lo_locked = smini->get_tx_sensor("lo_locked",0);
        std::cout << boost::format("Checking TX: %s ...") % lo_locked.to_pp_string() << std::endl;
        SHD_ASSERT_THROW(lo_locked.to_bool());
    }
    sensor_names = smini->get_mboard_sensor_names(0);
    if ((ref == "mimo") and (std::find(sensor_names.begin(), sensor_names.end(), "mimo_locked") != sensor_names.end())) {
        shd::sensor_value_t mimo_locked = smini->get_mboard_sensor("mimo_locked",0);
        std::cout << boost::format("Checking TX: %s ...") % mimo_locked.to_pp_string() << std::endl;
        SHD_ASSERT_THROW(mimo_locked.to_bool());
    }
    if ((ref == "external") and (std::find(sensor_names.begin(), sensor_names.end(), "ref_locked") != sensor_names.end())) {
        shd::sensor_value_t ref_locked = smini->get_mboard_sensor("ref_locked",0);
        std::cout << boost::format("Checking TX: %s ...") % ref_locked.to_pp_string() << std::endl;
        SHD_ASSERT_THROW(ref_locked.to_bool());
    }

    //set sigint if user wants to receive
    if(repeat){
        std::signal(SIGINT, &sig_int_handler);
        std::cout << "Press Ctrl + C to stop streaming..." << std::endl;
    }

    //send from file
    do{
        if (type == "double") send_from_file<std::complex<double> >(smini, "fc64", wirefmt, file, spb);
        else if (type == "float") send_from_file<std::complex<float> >(smini, "fc32", wirefmt, file, spb);
        else if (type == "short") send_from_file<std::complex<short> >(smini, "sc16", wirefmt, file, spb);
        else throw std::runtime_error("Unknown type " + type);

        if(repeat and delay != 0.0) boost::this_thread::sleep(boost::posix_time::milliseconds(delay));
    } while(repeat and not stop_signal_called);

    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;

    return EXIT_SUCCESS;
}
