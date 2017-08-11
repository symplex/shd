//
// Copyright 2014 Ettus Research LLC
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

#ifndef INCLUDED_SHD_MULTI_SMINI_CLOCK_HPP
#define INCLUDED_SHD_MULTI_SMINI_CLOCK_HPP

#include <string>
#include <vector>

#include <shd/config.hpp>
#include <shd/device.hpp>
#include <shd/types/device_addr.hpp>
#include <shd/types/sensors.hpp>

namespace shd{ namespace smini_clock{

/*!
 * The Multi-SMINI-Clock device class:
 *
 * This class facilitates ease-of-use for must use-case scenarios when
 * using clock devices with SHD. This class can be used with a
 * single clock device or with multiple clock devices connected to the same
 * host.
 *
 * To create a multi_smini_clock out of a single SMINI Clock:
 *
 * <pre>
 * device_addr_t dev;
 * dev["addr"] = 192.168.10.3;
 * multi_smini_clock::sptr clock = multi_smini_clock::make(dev);
 * </pre>
 *
 * To create a multi_smini_clock out of multiple clock devices:
 *
 * <pre>
 * device_addr_t dev;
 * dev["addr0"] = 192.168.10.3;
 * dev["addr1"] = 192.168.10.4;
 * multi_smini_clock::sptr clock = multi_smini_clock::make(dev);
 * </pre>
 */
class SHD_API multi_smini_clock : boost::noncopyable {
public:
    typedef boost::shared_ptr<multi_smini_clock> sptr;

    virtual ~multi_smini_clock(void) = 0;

    /*!
     * Make a new Multi-SMINI-Clock from the given device address.
     * \param dev_addr the device address
     * \return a new Multi-SMINI-Clock object
     */
    static sptr make(const device_addr_t &dev_addr);

    /*!
     * Return the underlying device.
     * This allows direct access to the EEPROM and sensors.
     * \return the device object within this Multi-SMINI-Clock
     */
    virtual device::sptr get_device(void) = 0;

    /*!
     * Get a printable summary for this SMINI Clock configuration.
     * \return a printable string
     */
    virtual std::string get_pp_string(void) = 0;

    //! Get the number of SMINI Clocks in this configuration.
    virtual size_t get_num_boards(void) = 0;

    //! Get time from device
    virtual uint32_t get_time(size_t board = 0) = 0;

    /*!
     * Get a SMINI Clock sensor value.
     * \param name the name of the sensor
     * \param board the board index (0 to M-1)
     * \return a sensor value object
     */
    virtual sensor_value_t get_sensor(const std::string &name, size_t board = 0) = 0;

    /*!
     * Get a list of possible SMINI Clock sensor names.
     * \param board the board index (0 to M-1)
     * \return a vector of sensor names
     */
    virtual std::vector<std::string> get_sensor_names(size_t board = 0) = 0;
};

} //namespace
} //namespace

#endif /* INCLUDED_SHD_MULTI_SMINI_CLOCK_HPP */
