//
// Copyright 2011,2014 Ettus Research LLC
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

#ifndef INCLUDED_LIBSHD_SMINI_TX_DSP_CORE_200_HPP
#define INCLUDED_LIBSHD_SMINI_TX_DSP_CORE_200_HPP

#include <shd/config.hpp>
#include <shd/stream.hpp>
#include <shd/types/ranges.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <shd/types/wb_iface.hpp>

class tx_dsp_core_200 : boost::noncopyable{
public:
    typedef boost::shared_ptr<tx_dsp_core_200> sptr;

    virtual ~tx_dsp_core_200(void) = 0;

    static sptr make(
        shd::wb_iface::sptr iface,
        const size_t dsp_base, const size_t ctrl_base,
        const uint32_t sid
    );

    virtual void clear(void) = 0;

    virtual void set_tick_rate(const double rate) = 0;

    virtual void set_link_rate(const double rate) = 0;

    virtual double set_host_rate(const double rate) = 0;

    virtual shd::meta_range_t get_host_rates(void) = 0;

    virtual double get_scaling_adjustment(void) = 0;

    virtual shd::meta_range_t get_freq_range(void) = 0;

    virtual double set_freq(const double freq) = 0;

    virtual void set_updates(const size_t cycles_per_up, const size_t packets_per_up) = 0;

    virtual void setup(const shd::stream_args_t &stream_args) = 0;
};

#endif /* INCLUDED_LIBSHD_SMINI_TX_DSP_CORE_200_HPP */
