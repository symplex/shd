#!/usr/bin/env python
#
# Copyright 2010-2011 Ettus Research LLC
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

"""
The smini2 recovery app:

When the smini2 has an unknown or bad ip address in its eeprom,
it may not be possible to communicate with the smini2 over ip/udp.

This app will send a raw ethernet packet to bypass the ip layer.
The packet will contain a known ip address to burn into eeprom.
Because the recovery packet is sent with a broadcast mac address,
only one smini2 should be present on the interface upon execution.

This app requires super-user privileges and only works on linux. 
"""

import socket
import struct
import optparse

BCAST_MAC_ADDR = 'ff:ff:ff:ff:ff:ff'
RECOVERY_ETHERTYPE = 0xbeee
IP_RECOVERY_CODE = 'addr'

def mac_addr_repr_to_binary_string(mac_addr):
    return ''.join([chr(int(x, 16)) for x in mac_addr.split(':')])

if __name__ == '__main__':
    parser = optparse.OptionParser(usage='usage: %prog [options]\n'+__doc__)
    parser.add_option('--ifc', type='string', help='ethernet interface name [default=%default]', default='eth0')
    parser.add_option('--new-ip', type='string', help='ip address to set [default=%default]', default='192.168.10.2')
    (options, args) = parser.parse_args()

    #create the raw socket
    print("Opening raw socket on interface:", options.ifc)
    soc = socket.socket(socket.PF_PACKET, socket.SOCK_RAW)
    soc.bind((options.ifc, RECOVERY_ETHERTYPE))

    #create the recovery packet
    print("Loading packet with ip address:", options.new_ip)
    packet = struct.pack(
        '!6s6sH4s4s',
        mac_addr_repr_to_binary_string(BCAST_MAC_ADDR),
        mac_addr_repr_to_binary_string(BCAST_MAC_ADDR),
        RECOVERY_ETHERTYPE,
        IP_RECOVERY_CODE,
        socket.inet_aton(options.new_ip),
    )

    print("Sending packet (%d bytes)"%len(packet))
    soc.send(packet)
    print("Done")
