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

#ifndef INCLUDED_SMINI1_IFACE_HPP
#define INCLUDED_SMINI1_IFACE_HPP

#include "fx2_ctrl.hpp"
#include <shd/types/wb_iface.hpp>
#include <shd/types/serial.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#define SPI_ENABLE_FPGA  0x01
#define SPI_FMT_HDR_MASK (3 << 5)
#define SPI_FMT_HDR_0    (0 << 5)
#define SPI_FMT_HDR_1    (1 << 5)
#define SPI_FMT_HDR_2    (2 << 5)
#define SPI_FMT_LSB      (1 << 7)
#define SPI_FMT_MSB      (0 << 7)
#define SPI_FMT_xSB_MASK (1 << 7)
#define VRQ_SPI_READ     0x82
#define VRQ_SPI_WRITE    0x09
#define VRQ_FW_COMPAT    0x83


/*!
 * The smini1 interface class:
 * Provides a set of functions to implementation layer.
 * Including spi, peek, poke, control...
 */
class smini1_iface : public shd::wb_iface, public shd::i2c_iface, public shd::spi_iface, boost::noncopyable
{
public:
    typedef boost::shared_ptr<smini1_iface> sptr;

    /*!
     * Make a new smini1 interface with the control transport.
     * \param ctrl_transport the smini controller object
     * \return a new smini1 interface object
     */
    static sptr make(shd::smini::fx2_ctrl::sptr ctrl_transport);
};

#endif /* INCLUDED_SMINI1_IFACE_HPP */
