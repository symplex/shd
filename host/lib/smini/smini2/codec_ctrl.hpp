//
// Copyright 2010-2012,2014 Ettus Research LLC
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

#ifndef INCLUDED_CODEC_CTRL_HPP
#define INCLUDED_CODEC_CTRL_HPP

#include "smini2_iface.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

class smini2_codec_ctrl : boost::noncopyable{
public:
    typedef boost::shared_ptr<smini2_codec_ctrl> sptr;

    virtual ~smini2_codec_ctrl(void) = 0;

    /*!
     * Make a codec control for the DAC and ADC.
     * \param iface a pointer to the smini2 interface object
     * \param spiface the interface to spi
     * \return a new codec control object
     */
    static sptr make(smini2_iface::sptr iface, shd::spi_iface::sptr spiface);

    /*!
     * Set the modulation mode for the DAC.
     * Possible modes are 0, +/-1, +/-2, +/-4, +/-8
     * which correspond to shifts of fs/mod_mode.
     * A mode of 0 or +/-1 means no modulation.
     * \param mod_mode the modulation mode
     */
    virtual void set_tx_mod_mode(int mod_mode) = 0;

    /*!
     * Set the analog preamplifier on the SMINI2+ ADC (ADS62P44).
     * \param gain enable or disable the 3.5dB preamp
     */

    virtual void set_rx_analog_gain(bool gain) = 0;

    /*!
     * Set the digital gain on the SMINI2+ ADC (ADS62P44).
     * \param gain from 0-6dB
     */

    virtual void set_rx_digital_gain(double gain) = 0;

    /*!
     * Set the digital gain correction on the SMINI2+ ADC (ADS62P44).
     * \param gain from 0-0.5dB
     */

    virtual void set_rx_digital_fine_gain(double gain) = 0;

};

#endif /* INCLUDED_CODEC_CTRL_HPP */
