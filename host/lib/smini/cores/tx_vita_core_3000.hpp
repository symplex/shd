//
// Copyright 2013-2014 Ettus Research LLC
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

#ifndef INCLUDED_LIBSHD_SMINI_TX_VITA_CORE_3000_HPP
#define INCLUDED_LIBSHD_SMINI_TX_VITA_CORE_3000_HPP

#include <shd/config.hpp>
#include <shd/stream.hpp>
#include <shd/types/ranges.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <shd/types/stream_cmd.hpp>
#include <shd/types/wb_iface.hpp>
#include <string>

class tx_vita_core_3000 : boost::noncopyable
{
public:
    typedef boost::shared_ptr<tx_vita_core_3000> sptr;

    enum fc_monitor_loc {
        FC_DEFAULT,
        FC_PRE_RADIO,
        FC_PRE_FIFO
    };

    virtual ~tx_vita_core_3000(void) = 0;

    static sptr make(
        shd::wb_iface::sptr iface,
        const size_t base,
        fc_monitor_loc fc_location = FC_PRE_RADIO
    );

    static sptr make_no_radio_buff(
        shd::wb_iface::sptr iface,
        const size_t base
    );

    virtual void clear(void) = 0;

    virtual void setup(const shd::stream_args_t &stream_args) = 0;

    virtual void configure_flow_control(const size_t cycs_per_up, const size_t pkts_per_up) = 0;
};

#endif /* INCLUDED_LIBSHD_SMINI_TX_VITA_CORE_3000_HPP */
