//
// Copyright 2014-2016 Ettus Research LLC
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

#include <shd/exception.hpp>
#include <shd/smini_clock/octoclock_eeprom.hpp>
#include <shd/transport/udp_simple.hpp>
#include <shd/smini/mboard_eeprom.hpp>
#include <shd/types/byte_vector.hpp>
#include <shd/types/mac_addr.hpp>
#include <shd/utils/byteswap.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include <iostream>

#include "common.h"

typedef boost::asio::ip::address_v4 ip_v4;

using namespace shd;
using namespace shd::smini_clock;
using namespace shd::transport;

/***********************************************************************
 * Implementation
 **********************************************************************/
void octoclock_eeprom_t::_load(){
    uint8_t octoclock_data[udp_simple::mtu];
    const octoclock_packet_t *pkt_in = reinterpret_cast<const octoclock_packet_t*>(octoclock_data);
    const octoclock_fw_eeprom_t *eeprom_in = reinterpret_cast<const octoclock_fw_eeprom_t*>(pkt_in->data);

    octoclock_packet_t pkt_out;
    // To avoid replicating sequence numbers between sessions
    pkt_out.sequence = uint32_t(std::rand());
    size_t len = 0;

    SHD_OCTOCLOCK_SEND_AND_RECV(xport, _proto_ver, SEND_EEPROM_CMD, pkt_out, len, octoclock_data);
    if(SHD_OCTOCLOCK_PACKET_MATCHES(SEND_EEPROM_ACK, pkt_out, pkt_in, len)){
        //MAC address
        byte_vector_t mac_bytes(eeprom_in->mac_addr, eeprom_in->mac_addr+6);
        (*this)["mac-addr"] = mac_addr_t::from_bytes(mac_bytes).to_string();

        //IP address
        uint32_t ip_addr = shd::htonx<uint32_t>(eeprom_in->ip_addr);
        ip_v4::bytes_type ip_addr_bytes;
        memcpy(&ip_addr_bytes, &ip_addr, 4);
        (*this)["ip-addr"] = ip_v4(ip_addr_bytes).to_string();

        //Default router
        uint32_t dr_addr = shd::htonx<uint32_t>(eeprom_in->dr_addr);
        ip_v4::bytes_type dr_addr_bytes;
        memcpy(&dr_addr_bytes, &dr_addr, 4);
        (*this)["gateway"] = ip_v4(dr_addr_bytes).to_string();

        //Netmask
        uint32_t netmask = shd::htonx<uint32_t>(eeprom_in->netmask);
        ip_v4::bytes_type netmask_bytes;
        memcpy(&netmask_bytes, &netmask, 4);
        (*this)["netmask"] = ip_v4(netmask_bytes).to_string();

        //Serial
        std::string raw_serial((char*)eeprom_in->serial, 10);
        byte_vector_t serial_bytes(raw_serial.begin(), raw_serial.end());
        (*this)["serial"] = bytes_to_string(serial_bytes);

        //Name
        std::string raw_name((char*)eeprom_in->name, 10);
        byte_vector_t name_bytes(raw_name.begin(), raw_name.end());
        (*this)["name"] = bytes_to_string(name_bytes);

        //Revision
        (*this)["revision"] = boost::lexical_cast<std::string>(int(eeprom_in->revision));
    }
    else throw shd::runtime_error("Error loading OctoClock EEPROM.");
}

void octoclock_eeprom_t::_store() const {
    uint8_t octoclock_data[udp_simple::mtu];
    const octoclock_packet_t *pkt_in = reinterpret_cast<const octoclock_packet_t *>(octoclock_data);

    octoclock_packet_t pkt_out;
    // To avoid replicating sequence numbers between sessions
    pkt_out.sequence = uint32_t(std::rand());
    pkt_out.len = sizeof(octoclock_fw_eeprom_t);
    size_t len = 0;

    octoclock_fw_eeprom_t *eeprom_out = reinterpret_cast<octoclock_fw_eeprom_t *>(&pkt_out.data);
    memset(eeprom_out, 0xFF, sizeof(octoclock_fw_eeprom_t));

    //MAC address
    if((*this).has_key("mac-addr")){
        byte_copy(mac_addr_t::from_string((*this)["mac-addr"]).to_bytes(), eeprom_out->mac_addr);
    }

    //IP address
    if((*this).has_key("ip-addr")){
        ip_v4::bytes_type ip_addr_bytes = ip_v4::from_string((*this)["ip-addr"]).to_bytes();
        memcpy(&eeprom_out->ip_addr, &ip_addr_bytes, 4);
        eeprom_out->ip_addr = shd::htonx<uint32_t>(eeprom_out->ip_addr);
    }

    //Default router
    if((*this).has_key("gateway")){
        ip_v4::bytes_type dr_addr_bytes = ip_v4::from_string((*this)["gateway"]).to_bytes();
        memcpy(&eeprom_out->dr_addr, &dr_addr_bytes, 4);
        eeprom_out->dr_addr = shd::htonx<uint32_t>(eeprom_out->dr_addr);
    }

    //Netmask
    if((*this).has_key("netmask")){
        ip_v4::bytes_type netmask_bytes = ip_v4::from_string((*this)["netmask"]).to_bytes();
        memcpy(&eeprom_out->netmask, &netmask_bytes, 4);
        eeprom_out->netmask = shd::htonx<uint32_t>(eeprom_out->netmask);
    }

    //Serial
    if((*this).has_key("serial")){
        byte_copy(byte_vector_t((*this)["serial"].begin(), (*this)["serial"].end()), eeprom_out->serial);
    }

    //Name
    if((*this).has_key("name")){
        byte_copy(byte_vector_t((*this)["name"].begin(), (*this)["name"].end()), eeprom_out->name);
    }

    //Revision
    if((*this).has_key("revision")){
        eeprom_out->revision = (*this)["revision"][0]-'0';
    }

    SHD_OCTOCLOCK_SEND_AND_RECV(xport, _proto_ver, BURN_EEPROM_CMD, pkt_out, len, octoclock_data);
    if(not SHD_OCTOCLOCK_PACKET_MATCHES(BURN_EEPROM_SUCCESS_ACK, pkt_out, pkt_in, len))
        throw shd::runtime_error("Error writing to OctoClock EEPROM.");
}

/***********************************************************************
 * Implementation of OctoClock EEPROM
 **********************************************************************/
octoclock_eeprom_t::octoclock_eeprom_t(void){
    /* NOP */
}

octoclock_eeprom_t::octoclock_eeprom_t(udp_simple::sptr transport, uint32_t proto_ver) :
	xport(transport), _proto_ver(proto_ver)
{
    _load();
}

void octoclock_eeprom_t::commit() const{
    if(!xport) throw shd::runtime_error("There is no set device communication.");
    _store();
}
