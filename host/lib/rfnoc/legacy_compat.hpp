//
// Copyright 2016 Ettus Research LLC
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

#ifndef INCLUDED_RFNOC_LEGACY_COMPAT_HPP
#define INCLUDED_RFNOC_LEGACY_COMPAT_HPP

#include <shd/device3.hpp>
#include <shd/stream.hpp>

namespace shd { namespace rfnoc {

    /*! Legacy compatibility layer class.
     */
    class legacy_compat
    {
    public:
        typedef boost::shared_ptr<legacy_compat> sptr;

        virtual shd::fs_path rx_dsp_root(const size_t mboard_idx, const size_t chan) = 0;

        virtual shd::fs_path tx_dsp_root(const size_t mboard_idx, const size_t chan) = 0;

        virtual shd::fs_path rx_fe_root(const size_t mboard_idx, const size_t chan) = 0;

        virtual shd::fs_path tx_fe_root(const size_t mboard_idx, const size_t chan) = 0;

        virtual void issue_stream_cmd(const shd::stream_cmd_t &stream_cmd, size_t mboard, size_t chan) = 0;

        virtual shd::rx_streamer::sptr get_rx_stream(const shd::stream_args_t &args) = 0;

        virtual shd::tx_streamer::sptr get_tx_stream(const shd::stream_args_t &args) = 0;

        virtual void set_rx_rate(const double rate, const size_t chan) = 0;

        virtual void set_tx_rate(const double rate, const size_t chan) = 0;

        static sptr make(
                shd::device3::sptr device,
                const shd::device_addr_t &args
        );
    };

}} /* namespace shd::rfnoc */

#endif /* INCLUDED_RFNOC_LEGACY_COMPAT_HPP */
