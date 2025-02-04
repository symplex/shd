//
// Copyright 2010-2011,2015-2016 Ettus Research LLC
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
#include <shd/version.hpp>
#include <shd/device.hpp>
#include <shd/device3.hpp>
#include <shd/types/ranges.hpp>
#include <shd/property_tree.hpp>
#include <boost/algorithm/string.hpp> //for split
#include <shd/smini/dboard_id.hpp>
#include <shd/smini/mboard_eeprom.hpp>
#include <shd/smini/dboard_eeprom.hpp>
#include <shd/types/sensors.hpp>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>

namespace po = boost::program_options;
using namespace shd;

static std::string make_border(const std::string &text){
    std::stringstream ss;
    ss << boost::format("  _____________________________________________________") << std::endl;
    ss << boost::format(" /") << std::endl;
    std::vector<std::string> lines; boost::split(lines, text, boost::is_any_of("\n"));
    while (lines.back().empty()) lines.pop_back(); //strip trailing newlines
    if (lines.size()) lines[0] = "    " + lines[0]; //indent the title line
    BOOST_FOREACH(const std::string &line, lines){
        ss << boost::format("|   %s") % line << std::endl;
    }
    //ss << boost::format(" \\_____________________________________________________") << std::endl;
    return ss.str();
}

static std::string get_dsp_pp_string(const std::string &type, property_tree::sptr tree, const fs_path &path){
    std::stringstream ss;
    ss << boost::format("%s DSP: %s") % type % path.leaf() << std::endl;
    ss << std::endl;
    meta_range_t freq_range = tree->access<meta_range_t>(path / "freq/range").get();
    ss << boost::format("Freq range: %.3f to %.3f MHz") % (freq_range.start()/1e6) % (freq_range.stop()/1e6) << std::endl;;
    return ss.str();
}

static std::string prop_names_to_pp_string(const std::vector<std::string> &prop_names){
    std::stringstream ss; size_t count = 0;
    BOOST_FOREACH(const std::string &prop_name, prop_names){
        ss << ((count++)? ", " : "") << prop_name;
    }
    return ss.str();
}

static std::string get_frontend_pp_string(const std::string &type, property_tree::sptr tree, const fs_path &path){
    std::stringstream ss;
    ss << boost::format("%s Frontend: %s") % type % path.leaf() << std::endl;
    //ss << std::endl;

    ss << boost::format("Name: %s") % (tree->access<std::string>(path / "name").get()) << std::endl;
    ss << boost::format("Antennas: %s") % prop_names_to_pp_string(tree->access<std::vector<std::string> >(path / "antenna/options").get()) << std::endl;
    ss << boost::format("Sensors: %s") % prop_names_to_pp_string(tree->list(path / "sensors")) << std::endl;

    meta_range_t freq_range = tree->access<meta_range_t>(path / "freq/range").get();
    ss << boost::format("Freq range: %.3f to %.3f MHz") % (freq_range.start()/1e6) % (freq_range.stop()/1e6) << std::endl;

    std::vector<std::string> gain_names = tree->list(path / "gains");
    if (gain_names.size() == 0) ss << "Gain Elements: None" << std::endl;
    BOOST_FOREACH(const std::string &name, gain_names){
        meta_range_t gain_range = tree->access<meta_range_t>(path / "gains" / name / "range").get();
        ss << boost::format("Gain range %s: %.1f to %.1f step %.1f dB") % name % gain_range.start() % gain_range.stop() % gain_range.step() << std::endl;
    }
    if (tree->exists(path / "bandwidth" / "range"))
    {
        meta_range_t bw_range = tree->access<meta_range_t>(path / "bandwidth" / "range").get();
        ss << boost::format("Bandwidth range: %.1f to %.1f step %.1f Hz") % bw_range.start() % bw_range.stop() % bw_range.step() << std::endl;
    }

    ss << boost::format("Connection Type: %s") % (tree->access<std::string>(path / "connection").get()) << std::endl;
    ss << boost::format("Uses LO offset: %s") % ((tree->access<bool>(path / "use_lo_offset").get())? "Yes" : "No") << std::endl;

    return ss.str();
}

static std::string get_codec_pp_string(const std::string &type, property_tree::sptr tree, const fs_path &path){
    std::stringstream ss;
    ss << boost::format("%s Codec: %s") % type % path.leaf() << std::endl;
    //ss << std::endl;

    ss << boost::format("Name: %s") % (tree->access<std::string>(path / "name").get()) << std::endl;
    std::vector<std::string> gain_names = tree->list(path / "gains");
    if (gain_names.size() == 0) ss << "Gain Elements: None" << std::endl;
    BOOST_FOREACH(const std::string &name, gain_names){
        meta_range_t gain_range = tree->access<meta_range_t>(path / "gains" / name / "range").get();
        ss << boost::format("Gain range %s: %.1f to %.1f step %.1f dB") % name % gain_range.start() % gain_range.stop() % gain_range.step() << std::endl;
    }
    return ss.str();
}

static std::string get_dboard_pp_string(const std::string &type, property_tree::sptr tree, const fs_path &path){
    std::stringstream ss;
    ss << boost::format("%s Dboard: %s") % type % path.leaf() << std::endl;
    //ss << std::endl;
    const std::string prefix = (type == "RX")? "rx" : "tx";
    if (tree->exists(path / (prefix + "_eeprom")))
    {
        smini::dboard_eeprom_t db_eeprom = tree->access<smini::dboard_eeprom_t>(path / (prefix + "_eeprom")).get();
        if (db_eeprom.id != smini::dboard_id_t::none()) ss << boost::format("ID: %s") % db_eeprom.id.to_pp_string() << std::endl;
        if (not db_eeprom.serial.empty()) ss << boost::format("Serial: %s") % db_eeprom.serial << std::endl;
        if (type == "TX"){
            smini::dboard_eeprom_t gdb_eeprom = tree->access<smini::dboard_eeprom_t>(path / "gdb_eeprom").get();
            if (gdb_eeprom.id != smini::dboard_id_t::none()) ss << boost::format("ID: %s") % gdb_eeprom.id.to_pp_string() << std::endl;
            if (not gdb_eeprom.serial.empty()) ss << boost::format("Serial: %s") % gdb_eeprom.serial << std::endl;
        }
    }
    BOOST_FOREACH(const std::string &name, tree->list(path / (prefix + "_frontends"))){
        ss << make_border(get_frontend_pp_string(type, tree, path / (prefix + "_frontends") / name));
    }
    ss << make_border(get_codec_pp_string(type, tree, path.branch_path().branch_path() / (prefix + "_codecs") / path.leaf()));
    return ss.str();
}


static std::string get_rfnoc_pp_string(property_tree::sptr tree, const fs_path &path){
    std::stringstream ss;
    ss << "RFNoC blocks on this device:" << std::endl << std::endl;
    BOOST_FOREACH(const std::string &name, tree->list(path)){
        ss << "* " << name << std::endl;
    }
    return ss.str();
}

static std::string get_mboard_pp_string(property_tree::sptr tree, const fs_path &path){
    std::stringstream ss;
    ss << boost::format("Mboard: %s") % (tree->access<std::string>(path / "name").get()) << std::endl;
    //ss << std::endl;
    smini::mboard_eeprom_t mb_eeprom = tree->access<smini::mboard_eeprom_t>(path / "eeprom").get();
    BOOST_FOREACH(const std::string &key, mb_eeprom.keys()){
        if (not mb_eeprom[key].empty()) ss << boost::format("%s: %s") % key % mb_eeprom[key] << std::endl;
    }
    if (tree->exists(path / "fw_version")){
        ss << "FW Version: " << tree->access<std::string>(path / "fw_version").get() << std::endl;
    }
    if (tree->exists(path / "fpga_version")){
        ss << "FPGA Version: " << tree->access<std::string>(path / "fpga_version").get() << std::endl;
    }
    if (tree->exists(path / "xbar")){
        ss << "RFNoC capable: Yes" << std::endl;
    }
    ss << std::endl;
    try {
        if (tree->exists(path / "time_source" / "options")){
            const std::vector< std::string > time_sources  = tree->access<std::vector<std::string> >(path / "time_source" / "options").get();
            ss << "Time sources:  " << prop_names_to_pp_string(time_sources)  << std::endl;
        }
        if (tree->exists(path / "clock_source" / "options")){
            const std::vector< std::string > clock_sources = tree->access<std::vector<std::string> >(path / "clock_source" / "options").get();
            ss << "Clock sources: " << prop_names_to_pp_string(clock_sources) << std::endl;
        }
        ss << "Sensors: " << prop_names_to_pp_string(tree->list(path / "sensors")) << std::endl;
        if (tree->exists(path / "rx_dsps")){
            BOOST_FOREACH(const std::string &name, tree->list(path / "rx_dsps")){
                ss << make_border(get_dsp_pp_string("RX", tree, path / "rx_dsps" / name));
            }
        }
        BOOST_FOREACH(const std::string &name, tree->list(path / "dboards")){
            ss << make_border(get_dboard_pp_string("RX", tree, path / "dboards" / name));
        }
        if (tree->exists(path / "tx_dsps")){
            BOOST_FOREACH(const std::string &name, tree->list(path / "tx_dsps")){
                ss << make_border(get_dsp_pp_string("TX", tree, path / "tx_dsps" / name));
            }
        }
        BOOST_FOREACH(const std::string &name, tree->list(path / "dboards")){
            ss << make_border(get_dboard_pp_string("TX", tree, path / "dboards" / name));
        }
            ss << make_border(get_rfnoc_pp_string(tree, path / "xbar"));
    }
    catch (const shd::lookup_error&) {
        /* nop */
    }
    return ss.str();
}


static std::string get_device_pp_string(property_tree::sptr tree){
    std::stringstream ss;
    ss << boost::format("Device: %s") % (tree->access<std::string>("/name").get()) << std::endl;
    //ss << std::endl;
    BOOST_FOREACH(const std::string &name, tree->list("/mboards")){
        ss << make_border(get_mboard_pp_string(tree, "/mboards/" + name));
    }
    return ss.str();
}

void print_tree(const shd::fs_path &path, shd::property_tree::sptr tree){
    std::cout << path << std::endl;
    BOOST_FOREACH(const std::string &name, tree->list(path)){
        print_tree(path / name, tree);
    }
}

int SHD_SAFE_MAIN(int argc, char *argv[]){
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("version", "print the version string and exit")
        ("args", po::value<std::string>()->default_value(""), "device address args")
        ("tree", "specify to print a complete property tree")
        ("string", po::value<std::string>(), "query a string value from the property tree")
        ("double", po::value<std::string>(), "query a double precision floating point value from the property tree")
        ("int", po::value<std::string>(), "query a integer value from the property tree")
        ("sensor", po::value<std::string>(), "query a sensor value from the property tree")
        ("range", po::value<std::string>(), "query a range (gain, bandwidth, frequency, ...)  from the property tree")
        ("vector", "when querying a string, interpret that as std::vector")
        ("init-only", "skip all queries, only initialize device")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("SHD SMINI Probe %s") % desc << std::endl;
        return EXIT_FAILURE;
    }

    if (vm.count("version")){
        std::cout << shd::get_version_string() << std::endl;
        return EXIT_SUCCESS;
    }

    device::sptr dev = device::make(vm["args"].as<std::string>());
    property_tree::sptr tree = dev->get_tree();

    if (vm.count("string")){
        if (vm.count("vector")) {
            std::vector<std::string> str_vector = tree->access< std::vector<std::string> >(vm["string"].as<std::string>()).get();
            std::cout << "(";
            BOOST_FOREACH(const std::string &str, str_vector) {
                std::cout << str << ",";
            }
            std::cout << ")" << std::endl;
        } else {
            std::cout << tree->access<std::string>(vm["string"].as<std::string>()).get() << std::endl;
        }
        return EXIT_SUCCESS;
    }

    if (vm.count("double")){
        std::cout << tree->access<double>(vm["double"].as<std::string>()).get() << std::endl;
        return EXIT_SUCCESS;
    }

    if (vm.count("int")){
        std::cout << tree->access<int>(vm["int"].as<std::string>()).get() << std::endl;
        return EXIT_SUCCESS;
    }

    if (vm.count("sensor")){
        std::cout << tree->access<shd::sensor_value_t>(vm["sensor"].as<std::string>()).get().value << std::endl;
        return EXIT_SUCCESS;
    }

    if (vm.count("range")){
        meta_range_t range = tree->access<meta_range_t>(vm["range"].as<std::string>()).get();
        std::cout << boost::format("%.1f:%.1f:%.1f") % range.start() % range.step() % range.stop() << std::endl;
        return EXIT_SUCCESS;
    }

    if (vm.count("tree") != 0) print_tree("/", tree);
    else if (not vm.count("init-only")) std::cout << make_border(get_device_pp_string(tree)) << std::endl;

    return EXIT_SUCCESS;
}
