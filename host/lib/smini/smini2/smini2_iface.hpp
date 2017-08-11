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

#ifndef INCLUDED_SMINI2_IFACE_HPP
#define INCLUDED_SMINI2_IFACE_HPP

#include <shd/transport/udp_simple.hpp>
#include <shd/types/serial.hpp>
#include <shd/smini/mboard_eeprom.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include "smini2_regs.hpp"
#include <shd/types/wb_iface.hpp>
#include <string>

/*!
 * The smini2 interface class:
 * Provides a set of functions to implementation layer.
 * Including spi, peek, poke, control...
 */
class smini2_iface : public shd::timed_wb_iface, public shd::spi_iface, public shd::i2c_iface
{
public:
    typedef boost::shared_ptr<smini2_iface> sptr;
    /*!
     * Make a new smini2 interface with the control transport.
     * \param ctrl_transport the udp transport object
     * \return a new smini2 interface object
     */
    static sptr make(shd::transport::udp_simple::sptr ctrl_transport);

    //! poke a register in the virtual fw table
    virtual void pokefw(wb_addr_type addr, uint32_t data) = 0;

    //! peek a register in the virtual fw table
    virtual uint32_t peekfw(wb_addr_type addr) = 0;

    //! The list of possible revision types
    enum rev_type {
        SMINI2_REV3 = 3,
        SMINI2_REV4 = 4,
        SMINI_N200 = 200,
        SMINI_N200_R4 = 201,
        SMINI_N210 = 210,
        SMINI_N210_R4 = 211,
        SMINI_NXXX = 0
    };

    //! Get the revision type for this device
    virtual rev_type get_rev(void) = 0;

    //! Get the canonical name for this device
    virtual const std::string get_cname(void) = 0;

    //! Lock the device to this iface
    virtual void lock_device(bool lock) = 0;

    //! Is this device locked?
    virtual bool is_device_locked(void) = 0;

    //! A version string for firmware
    virtual const std::string get_fw_version_string(void) = 0;

    //! Construct a helpful warning message for images
    virtual std::string images_warn_help_message(void) = 0;

    //motherboard eeprom map structure
    shd::smini::mboard_eeprom_t mb_eeprom;
};

#endif /* INCLUDED_SMINI2_IFACE_HPP */
