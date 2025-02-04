//
// Copyright 2011-2014 Ettus Research LLC
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

#ifndef INCLUDED_LIBSHD_SMINI_TX_DSP_CORE_3000_HPP
#define INCLUDED_LIBSHD_SMINI_TX_DSP_CORE_3000_HPP

#include <shd/config.hpp>
#include <shd/stream.hpp>
#include <shd/types/ranges.hpp>
#include <shd/types/wb_iface.hpp>
#include <shd/property_tree.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

class tx_dsp_core_3000 : boost::noncopyable{
public:
    static const double DEFAULT_CORDIC_FREQ;
    static const double DEFAULT_RATE;

    typedef boost::shared_ptr<tx_dsp_core_3000> sptr;

    virtual ~tx_dsp_core_3000(void) = 0;

    static sptr make(
        shd::wb_iface::sptr iface,
        const size_t dsp_base
    );

    virtual void set_tick_rate(const double rate) = 0;

    virtual void set_link_rate(const double rate) = 0;

    virtual double set_host_rate(const double rate) = 0;

    virtual shd::meta_range_t get_host_rates(void) = 0;

    virtual double get_scaling_adjustment(void) = 0;

    virtual shd::meta_range_t get_freq_range(void) = 0;

    virtual double set_freq(const double freq) = 0;

    virtual void setup(const shd::stream_args_t &stream_args) = 0;

    virtual void populate_subtree(shd::property_tree::sptr subtree) = 0;
};

#endif /* INCLUDED_LIBSHD_SMINI_TX_DSP_CORE_3000_HPP */
