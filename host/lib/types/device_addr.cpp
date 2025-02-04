//
// Copyright 2011,2016 Ettus Research LLC
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

#include <shd/types/device_addr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <stdexcept>
#include <sstream>

using namespace shd;

static const char* arg_delim = ",";
static const char* pair_delim = "=";

static std::string trim(const std::string &in){
    return boost::algorithm::trim_copy(in);
}

#define tokenizer(inp, sep) \
    boost::tokenizer<boost::char_separator<char> > \
    (inp, boost::char_separator<char>(sep))

device_addr_t::device_addr_t(const std::string &args){
    BOOST_FOREACH(const std::string &pair, tokenizer(args, arg_delim)){
        if (trim(pair) == "") continue;
        std::vector<std::string> toks;
        BOOST_FOREACH(const std::string &tok, tokenizer(pair, pair_delim)){
            toks.push_back(tok);
        }
        if (toks.size() == 1) toks.push_back(""); //pad empty value
        if (toks.size() == 2 and not trim(toks[0]).empty()){ //only valid combination
            this->set(trim(toks[0]), trim(toks[1]));
        }
        else throw shd::value_error("invalid args string: "+args); //otherwise error
    }
}

std::string device_addr_t::to_pp_string(void) const{
    if (this->size() == 0) return "Empty Device Address";

    std::stringstream ss;
    ss << "Device Address:" << std::endl;
    BOOST_FOREACH(std::string key, this->keys()){
        ss << boost::format("    %s: %s") % key % this->get(key) << std::endl;
    }
    return ss.str();
}

std::string device_addr_t::to_string(void) const{
    std::string args_str;
    size_t count = 0;
    BOOST_FOREACH(const std::string &key, this->keys()){
        args_str += ((count++)? arg_delim : "") + key + pair_delim + this->get(key);
    }
    return args_str;
}

#include <shd/utils/msg.hpp>

device_addrs_t shd::separate_device_addr(const device_addr_t &dev_addr){
    //------------ support old deprecated way and print warning --------
    if (dev_addr.has_key("addr") and not dev_addr["addr"].empty()){
        std::vector<std::string> addrs; boost::split(addrs, dev_addr["addr"], boost::is_any_of(" "));
        if (addrs.size() > 1){
            device_addr_t fixed_dev_addr = dev_addr;
            fixed_dev_addr.pop("addr");
            for (size_t i = 0; i < addrs.size(); i++){
                fixed_dev_addr[str(boost::format("addr%d") % i)] = addrs[i];
            }
            SHD_MSG(warning) <<
                "addr = <space separated list of ip addresses> is deprecated.\n"
                "To address a multi-device, use multiple <key><index> = <val>.\n"
                "See the SMINI-NXXX application notes. Two device example:\n"
                "    addr0 = 192.168.10.2\n"
                "    addr1 = 192.168.10.3\n"
            ;
            return separate_device_addr(fixed_dev_addr);
        }
    }
    //------------------------------------------------------------------
    device_addrs_t dev_addrs(1); //must be at least one (obviously)
    std::vector<std::string> global_keys; //keys that apply to all (no numerical suffix)
    BOOST_FOREACH(const std::string &key, dev_addr.keys()){
        boost::cmatch matches;
        if (not boost::regex_match(key.c_str(), matches, boost::regex("^(\\D+)(\\d*)$"))){
            throw std::runtime_error("unknown key format: " + key);
        }
        std::string key_part(matches[1].first, matches[1].second);
        std::string num_part(matches[2].first, matches[2].second);
        if (num_part.empty()){ //no number? save it for later
            global_keys.push_back(key);
            continue;
        }
        const size_t num = boost::lexical_cast<size_t>(num_part);
        dev_addrs.resize(std::max(num+1, dev_addrs.size()));
        dev_addrs[num][key_part] = dev_addr[key];
    }

    //copy the global settings across all device addresses
    BOOST_FOREACH(device_addr_t &my_dev_addr, dev_addrs){
        BOOST_FOREACH(const std::string &global_key, global_keys){
            my_dev_addr[global_key] = dev_addr[global_key];
        }
    }
    return dev_addrs;
}

device_addr_t shd::combine_device_addrs(const device_addrs_t &dev_addrs){
    device_addr_t dev_addr;
    for (size_t i = 0; i < dev_addrs.size(); i++){
        BOOST_FOREACH(const std::string &key, dev_addrs[i].keys()){
            dev_addr[str(boost::format("%s%d") % key % i)] = dev_addrs[i][key];
        }
    }
    return dev_addr;
}
