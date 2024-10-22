//
// Copyright 2014,2016 Ettus Research LLC
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

#include <boost/asio.hpp>
#include <boost/assign.hpp>
#include <stdint.h>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>

#include <shd/device.hpp>
#include <shd/exception.hpp>
#include <shd/transport/udp_simple.hpp>
#include <shd/transport/if_addrs.hpp>
#include <shd/types/dict.hpp>
#include <shd/smini/gps_ctrl.hpp>
#include <shd/smini_clock/octoclock_eeprom.hpp>
#include <shd/utils/byteswap.hpp>
#include <shd/utils/paths.hpp>
#include <shd/utils/msg.hpp>
#include <shd/utils/paths.hpp>
#include <shd/utils/static.hpp>

#include "octoclock_impl.hpp"
#include "octoclock_uart.hpp"
#include "common.h"

using namespace shd;
using namespace shd::smini_clock;
using namespace shd::transport;
namespace asio = boost::asio;
namespace fs = boost::filesystem;

/***********************************************************************
 * Discovery
 **********************************************************************/
device_addrs_t octoclock_find(const device_addr_t &hint){
    //Handle the multi-device discovery
    device_addrs_t hints = separate_device_addr(hint);
    if (hints.size() > 1){
        device_addrs_t found_devices;
        std::string error_msg;
        BOOST_FOREACH(const device_addr_t &hint_i, hints){
            device_addrs_t found_devices_i = octoclock_find(hint_i);
            if (found_devices_i.size() != 1) error_msg += str(boost::format(
                "Could not resolve device hint \"%s\" to a single device."
            ) % hint_i.to_string());
            else found_devices.push_back(found_devices_i[0]);
        }
        if (found_devices.empty()) return device_addrs_t();
        if (not error_msg.empty()) throw shd::value_error(error_msg);
        return device_addrs_t(1, combine_device_addrs(found_devices));
    }

    //Initialize the hint for a single device case
    SHD_ASSERT_THROW(hints.size() <= 1);
    hints.resize(1); //In case it was empty
    device_addr_t _hint = hints[0];
    device_addrs_t octoclock_addrs;

    //return an empty list of addresses when type is set to non-OctoClock
    if (hint.has_key("type") and hint["type"].find("octoclock") == std::string::npos) return octoclock_addrs;

    //Return an empty list of addresses when a resource is specified,
    //since a resource is intended for a different, non-USB, device.
    if (hint.has_key("resource")) return octoclock_addrs;

    //If no address was specified, send a broadcast on each interface
    if (not _hint.has_key("addr")){
        BOOST_FOREACH(const if_addrs_t &if_addrs, get_if_addrs()){
            //avoid the loopback device
            if (if_addrs.inet == asio::ip::address_v4::loopback().to_string()) continue;

            //create a new hint with this broadcast address
            device_addr_t new_hint = hint;
            new_hint["addr"] = if_addrs.bcast;

            //call discover with the new hint and append results
            device_addrs_t new_octoclock_addrs = octoclock_find(new_hint);
            octoclock_addrs.insert(octoclock_addrs.begin(),
                new_octoclock_addrs.begin(), new_octoclock_addrs.end()
            );
        }
        return octoclock_addrs;
    }

    //Create a UDP transport to communicate
    udp_simple::sptr udp_transport = udp_simple::make_broadcast(
                                        _hint["addr"],
                                        BOOST_STRINGIZE(OCTOCLOCK_UDP_CTRL_PORT)
                                     );

    //Send a query packet
    octoclock_packet_t pkt_out;
    pkt_out.proto_ver = OCTOCLOCK_FW_COMPAT_NUM;
    // To avoid replicating sequence numbers between sessions
    pkt_out.sequence = uint32_t(std::rand());
    pkt_out.len = 0;
    pkt_out.code = OCTOCLOCK_QUERY_CMD;
    try{
        udp_transport->send(boost::asio::buffer(&pkt_out, sizeof(pkt_out)));
    }
    catch(const std::exception &ex){
        SHD_MSG(error) << "OctoClock network discovery error - " << ex.what() << std::endl;
    }
    catch(...){
        SHD_MSG(error) << "OctoClock network discovery unknown error" << std::endl;
    }

    uint8_t octoclock_data[udp_simple::mtu];
    const octoclock_packet_t *pkt_in = reinterpret_cast<octoclock_packet_t*>(octoclock_data);

    while(true){
        size_t len = udp_transport->recv(asio::buffer(octoclock_data));
        if(SHD_OCTOCLOCK_PACKET_MATCHES(OCTOCLOCK_QUERY_ACK, pkt_out, pkt_in, len)){
            device_addr_t new_addr;
            new_addr["addr"] = udp_transport->get_recv_addr();

            //Attempt direct communication with OctoClock
            udp_simple::sptr ctrl_xport = udp_simple::make_connected(
                                              new_addr["addr"],
                                              BOOST_STRINGIZE(OCTOCLOCK_UDP_CTRL_PORT)
                                          );
            SHD_OCTOCLOCK_SEND_AND_RECV(ctrl_xport, OCTOCLOCK_FW_COMPAT_NUM, OCTOCLOCK_QUERY_CMD, pkt_out, len, octoclock_data);
            if(SHD_OCTOCLOCK_PACKET_MATCHES(OCTOCLOCK_QUERY_ACK, pkt_out, pkt_in, len)){
                //If the OctoClock is in its bootloader, don't ask for details
                if(pkt_in->proto_ver == OCTOCLOCK_BOOTLOADER_PROTO_VER){
                    new_addr["type"] = "octoclock-bootloader";
                    octoclock_addrs.push_back(new_addr);
                }
                else {
                    new_addr["type"] = "octoclock";

                    if(pkt_in->proto_ver >= OCTOCLOCK_FW_MIN_COMPAT_NUM and pkt_in->proto_ver <= OCTOCLOCK_FW_COMPAT_NUM) {
                        octoclock_eeprom_t oc_eeprom(ctrl_xport, pkt_in->proto_ver);
                        new_addr["name"] = oc_eeprom["name"];
                        new_addr["serial"] = oc_eeprom["serial"];
                    } else {
                        new_addr["name"] = "";
                        new_addr["serial"] = "";
                    }

                    //Filter based on optional keys (if any)
                    if(
                       (not _hint.has_key("name") or (_hint["name"] == new_addr["name"])) and
                       (not _hint.has_key("serial") or (_hint["serial"] == new_addr["serial"]))
                    ){
                        octoclock_addrs.push_back(new_addr);
                    }
                }
            }
            else continue;
        }

        if(len == 0) break;
    }

    return octoclock_addrs;
}

device::sptr octoclock_make(const device_addr_t &device_addr){
    return device::sptr(new octoclock_impl(device_addr));
}

SHD_STATIC_BLOCK(register_octoclock_device){
    device::register_device(&octoclock_find, &octoclock_make, device::CLOCK);
}

/***********************************************************************
 * Structors
 **********************************************************************/
octoclock_impl::octoclock_impl(const device_addr_t &_device_addr){
    SHD_MSG(status) << "Opening an OctoClock device..." << std::endl;
    _type = device::CLOCK;
    device_addrs_t device_args = separate_device_addr(_device_addr);
    // To avoid replicating sequence numbers between sessions
    _sequence = uint32_t(std::rand());

    ////////////////////////////////////////////////////////////////////
    // Initialize the property tree
    ////////////////////////////////////////////////////////////////////
    _tree = property_tree::make();
    _tree->create<std::string>("/name").set("OctoClock Device");

    for(size_t oci = 0; oci < device_args.size(); oci++){
        const device_addr_t device_args_i = device_args[oci];
        const std::string addr = device_args_i["addr"];
        //Can't make a device out of an OctoClock in bootloader state
        if(device_args_i["type"] == "octoclock-bootloader"){
            throw shd::runtime_error(str(boost::format(
                    "\n\nThis device is in its bootloader state and cannot be used by SHD.\n"
                    "This may mean the firmware on the device has been corrupted and will\n"
                    "need to be burned again.\n\n"
                    "%s\n"
                ) % _get_images_help_message(addr)));
        }

        const std::string oc = boost::lexical_cast<std::string>(oci);

        ////////////////////////////////////////////////////////////////////
        // Set up UDP transports
        ////////////////////////////////////////////////////////////////////
        _oc_dict[oc].ctrl_xport = udp_simple::make_connected(addr, BOOST_STRINGIZE(OCTOCLOCK_UDP_CTRL_PORT));
        _oc_dict[oc].gpsdo_xport = udp_simple::make_connected(addr, BOOST_STRINGIZE(OCTOCLOCK_UDP_GPSDO_PORT));

        const fs_path oc_path = "/mboards/" + oc;
        _tree->create<std::string>(oc_path / "name").set("OctoClock");

        ////////////////////////////////////////////////////////////////////
        // Check the firmware compatibility number
        ////////////////////////////////////////////////////////////////////
        _proto_ver = _get_fw_version(oc);
        if(_proto_ver < OCTOCLOCK_FW_MIN_COMPAT_NUM or _proto_ver > OCTOCLOCK_FW_COMPAT_NUM){
            throw shd::runtime_error(str(boost::format(
                    "\n\nPlease update your OctoClock's firmware.\n"
                    "Expected firmware compatibility number %d, but got %d:\n"
                    "The firmware build is not compatible with the host code build.\n\n"
                    "%s\n"
                ) % int(OCTOCLOCK_FW_COMPAT_NUM) % int(_proto_ver) % _get_images_help_message(addr)));
        }
        _tree->create<std::string>(oc_path / "fw_version").set(boost::lexical_cast<std::string>(int(_proto_ver)));

        ////////////////////////////////////////////////////////////////////
        // Set up EEPROM
        ////////////////////////////////////////////////////////////////////
        _oc_dict[oc].eeprom = octoclock_eeprom_t(_oc_dict[oc].ctrl_xport, _proto_ver);
        _tree->create<octoclock_eeprom_t>(oc_path / "eeprom")
            .set(_oc_dict[oc].eeprom)
            .add_coerced_subscriber(boost::bind(&octoclock_impl::_set_eeprom, this, oc, _1));

        ////////////////////////////////////////////////////////////////////
        // Initialize non-GPSDO sensors
        ////////////////////////////////////////////////////////////////////
        _tree->create<uint32_t>(oc_path / "time")
            .set_publisher(boost::bind(&octoclock_impl::_get_time, this, oc));
        _tree->create<sensor_value_t>(oc_path / "sensors/ext_ref_detected")
            .set_publisher(boost::bind(&octoclock_impl::_ext_ref_detected, this, oc));
        _tree->create<sensor_value_t>(oc_path / "sensors/gps_detected")
            .set_publisher(boost::bind(&octoclock_impl::_gps_detected, this, oc));
        _tree->create<sensor_value_t>(oc_path / "sensors/using_ref")
            .set_publisher(boost::bind(&octoclock_impl::_which_ref, this, oc));
        _tree->create<sensor_value_t>(oc_path / "sensors/switch_pos")
            .set_publisher(boost::bind(&octoclock_impl::_switch_pos, this, oc));

        ////////////////////////////////////////////////////////////////////
        // Check reference and GPSDO
        ////////////////////////////////////////////////////////////////////
        std::string asterisk = (device_args.size() > 1) ? " * " : "";

        if(device_args.size() > 1){
            SHD_MSG(status) << std::endl << "Checking status of " << addr << ":" << std::endl;
        }
        SHD_MSG(status) << boost::format("%sDetecting internal GPSDO...") % asterisk << std::flush;

        _get_state(oc);
        if(_oc_dict[oc].state.gps_detected){
            try{
                _oc_dict[oc].gps = gps_ctrl::make(octoclock_make_uart_iface(_oc_dict[oc].gpsdo_xport, _proto_ver));

                if(_oc_dict[oc].gps and _oc_dict[oc].gps->gps_detected()){
                    BOOST_FOREACH(const std::string &name, _oc_dict[oc].gps->get_sensors()){
                        _tree->create<sensor_value_t>(oc_path / "sensors" / name)
                            .set_publisher(boost::bind(&gps_ctrl::get_sensor, _oc_dict[oc].gps, name));
                    }
                }
                else{
                    //If GPSDO communication failed, set gps_detected to false
                    _oc_dict[oc].state.gps_detected = 0;
                    SHD_MSG(warning) << "Device reports that it has a GPSDO, but we cannot communicate with it." << std::endl;
                    std::cout << std::endl;
                }
            }
            catch(std::exception &e){
                SHD_MSG(error) << "An error occurred making GPSDO control: " << e.what() << std::endl;
            }
        }
        else SHD_MSG(status) << "No GPSDO found" << std::endl;
        SHD_MSG(status) << boost::format("%sDetecting external reference...%s") % asterisk
                                                                                % _ext_ref_detected(oc).value
                        << std::endl;
        SHD_MSG(status) << boost::format("%sDetecting switch position...%s") % asterisk
                                                                             % _switch_pos(oc).value
                        << std::endl;
        std::string ref = _which_ref(oc).value;
        if(ref == "none") SHD_MSG(status) << boost::format("%sDevice is not using any reference") % asterisk << std::endl;
        else SHD_MSG(status) << boost::format("%sDevice is using %s reference") % asterisk
                                                                                % _which_ref(oc).value
                             << std::endl;
    }
}

rx_streamer::sptr octoclock_impl::get_rx_stream(SHD_UNUSED(const stream_args_t &args)){
    throw shd::not_implemented_error("This function is incompatible with this device.");
}

tx_streamer::sptr octoclock_impl::get_tx_stream(SHD_UNUSED(const stream_args_t &args)){
    throw shd::not_implemented_error("This function is incompatible with this device.");
}

bool octoclock_impl::recv_async_msg(SHD_UNUSED(shd::async_metadata_t&), SHD_UNUSED(double)){
    throw shd::not_implemented_error("This function is incompatible with this device.");
}

void octoclock_impl::_set_eeprom(const std::string &oc, const octoclock_eeprom_t &oc_eeprom){
    /*
     * The OctoClock needs a full octoclock_eeprom_t so as to not erase
     * what it currently has in the EEPROM, so store the relevant values
     * from the user's input and send that instead.
     */
    BOOST_FOREACH(const std::string &key, oc_eeprom.keys()){
        if(_oc_dict[oc].eeprom.has_key(key)) _oc_dict[oc].eeprom[key] = oc_eeprom[key];
    }
    _oc_dict[oc].eeprom.commit();
}

uint32_t octoclock_impl::_get_fw_version(const std::string &oc){
    octoclock_packet_t pkt_out;
    pkt_out.sequence = shd::htonx<uint32_t>(++_sequence);
    pkt_out.len = 0;
    size_t len;

    uint8_t octoclock_data[udp_simple::mtu];
    const octoclock_packet_t *pkt_in = reinterpret_cast<octoclock_packet_t*>(octoclock_data);

    SHD_OCTOCLOCK_SEND_AND_RECV(_oc_dict[oc].ctrl_xport, OCTOCLOCK_FW_COMPAT_NUM, OCTOCLOCK_QUERY_CMD, pkt_out, len, octoclock_data);
    if(SHD_OCTOCLOCK_PACKET_MATCHES(OCTOCLOCK_QUERY_ACK, pkt_out, pkt_in, len)){
        return pkt_in->proto_ver;
    }
    else throw shd::runtime_error("Failed to retrieve firmware version from OctoClock.");
}

void octoclock_impl::_get_state(const std::string &oc){
    octoclock_packet_t pkt_out;
    pkt_out.sequence = shd::htonx<uint32_t>(++_sequence);
    pkt_out.len = 0;
    size_t len = 0;

    uint8_t octoclock_data[udp_simple::mtu];
    const octoclock_packet_t *pkt_in = reinterpret_cast<octoclock_packet_t*>(octoclock_data);

    SHD_OCTOCLOCK_SEND_AND_RECV(_oc_dict[oc].ctrl_xport, _proto_ver, SEND_STATE_CMD, pkt_out, len, octoclock_data);
    if(SHD_OCTOCLOCK_PACKET_MATCHES(SEND_STATE_ACK, pkt_out, pkt_in, len)){
        const octoclock_state_t *state = reinterpret_cast<const octoclock_state_t*>(pkt_in->data);
        _oc_dict[oc].state = *state;
    }
    else throw shd::runtime_error("Failed to retrieve state information from OctoClock.");
}

shd::dict<ref_t, std::string> _ref_strings = boost::assign::map_list_of
    (NO_REF,   "none")
    (INTERNAL, "internal")
    (EXTERNAL, "external")
;

shd::dict<switch_pos_t, std::string> _switch_pos_strings = boost::assign::map_list_of
    (PREFER_INTERNAL, "Prefer internal")
    (PREFER_EXTERNAL, "Prefer external")
;

sensor_value_t octoclock_impl::_ext_ref_detected(const std::string &oc){
    _get_state(oc);

    return sensor_value_t("External reference detected", (_oc_dict[oc].state.external_detected > 0),
                          "true", "false");
}

sensor_value_t octoclock_impl::_gps_detected(const std::string &oc){
    //Don't check, this shouldn't change once device is turned on

    return sensor_value_t("GPSDO detected", (_oc_dict[oc].state.gps_detected > 0),
                          "true", "false");
}

sensor_value_t octoclock_impl::_which_ref(const std::string &oc){
    _get_state(oc);

    if(not _ref_strings.has_key(ref_t(_oc_dict[oc].state.which_ref))){
        throw shd::runtime_error("Invalid reference detected.");
    }

    return sensor_value_t("Using reference", _ref_strings[ref_t(_oc_dict[oc].state.which_ref)], "");
}

sensor_value_t octoclock_impl::_switch_pos(const std::string &oc){
    _get_state(oc);

    if(not _switch_pos_strings.has_key(switch_pos_t(_oc_dict[oc].state.switch_pos))){
        throw shd::runtime_error("Invalid switch position detected.");
    }

    return sensor_value_t("Switch position", _switch_pos_strings[switch_pos_t(_oc_dict[oc].state.switch_pos)], "");
}

uint32_t octoclock_impl::_get_time(const std::string &oc){
    if(_oc_dict[oc].state.gps_detected){
        std::string time_str = _oc_dict[oc].gps->get_sensor("gps_time").value;
        return boost::lexical_cast<uint32_t>(time_str);
    }
    else throw shd::runtime_error("This device cannot return a time.");
}

std::string octoclock_impl::_get_images_help_message(const std::string &addr){
    const std::string image_name = "octoclock_r4_fw.hex";

    //Check to see if image is in default location
    std::string image_location;
    try{
        image_location = shd::find_image_path(image_name);
    }
    catch(const std::exception &){
        return str(boost::format("Could not find %s in your images path.\n%s")
                   % image_name
                   % shd::print_utility_error("shd_images_downloader.py"));
    }

    //Get escape character
    #ifdef SHD_PLATFORM_WIN32
    const std::string ml = "^\n    ";
    #else
    const std::string ml = "\\\n    ";
    #endif

    //Get burner command
    const std::string burner_path = (fs::path(shd::get_pkg_path()) / "bin" / "shd_image_loader").string();
    const std::string burner_cmd = str(boost::format("%s %s--addr=\"%s\"") % burner_path % ml % addr);
    return str(boost::format("%s\n%s") % shd::print_utility_error("shd_images_downloader.py") % burner_cmd);
}
