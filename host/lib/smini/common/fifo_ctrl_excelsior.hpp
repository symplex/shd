//
// Copyright 2012,2015 Ettus Research LLC
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

#ifndef INCLUDED_B200_CTRL_HPP
#define INCLUDED_B200_CTRL_HPP

#include <shd/types/time_spec.hpp>
#include <shd/types/metadata.hpp>
#include <shd/types/serial.hpp>
#include <shd/transport/zero_copy.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <shd/types/wb_iface.hpp>
#include <string>


struct fifo_ctrl_excelsior_config
{
    size_t async_sid_base;
    size_t num_async_chan;
    size_t ctrl_sid_base;
    size_t spi_base;
    size_t spi_rb;
};

/*!
 * Provide access to peek, poke, spi, and async messages.
 */
class fifo_ctrl_excelsior : public shd::timed_wb_iface, public shd::spi_iface
{
public:
    typedef boost::shared_ptr<fifo_ctrl_excelsior> sptr;

    //! Make a new control object
    static sptr make(
        shd::transport::zero_copy_if::sptr xport,
        const fifo_ctrl_excelsior_config &config
    );

    //! Set the tick rate (converting time into ticks)
    virtual void set_tick_rate(const double rate) = 0;

    //! Pop an async message from the queue or timeout
    virtual bool pop_async_msg(shd::async_metadata_t &async_metadata, double timeout) = 0;
};

#endif /* INCLUDED_B200_CTRL_HPP */
