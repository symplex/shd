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

#ifndef INCLUDED_E300_REMOTE_CODEC_CTRL_HPP
#define INCLUDED_E300_REMOTE_CODEC_CTRL_HPP

#include "ad9361_ctrl.hpp"
#include <shd/transport/zero_copy.hpp>

namespace shd { namespace smini { namespace e300 {

class e300_remote_codec_ctrl : public shd::smini::ad9361_ctrl
{
public:
    struct transaction_t {
        uint32_t     action;
        uint32_t     which;
        union {
            double          rate;
            double          gain;
            double          freq;
            double          rssi;
            double          temp;
            double          bw;
            uint32_t use_dc_correction;
            uint32_t use_iq_correction;
            uint32_t use_agc;
            uint32_t agc_mode;
            uint64_t bits;
        };

        //Actions
        static const uint32_t ACTION_SET_GAIN            = 10;
        static const uint32_t ACTION_SET_CLOCK_RATE      = 11;
        static const uint32_t ACTION_SET_ACTIVE_CHANS    = 12;
        static const uint32_t ACTION_TUNE                = 13;
        static const uint32_t ACTION_SET_LOOPBACK        = 14;
        static const uint32_t ACTION_GET_RSSI            = 15;
        static const uint32_t ACTION_GET_TEMPERATURE     = 16;
        static const uint32_t ACTION_SET_DC_OFFSET_AUTO  = 17;
        static const uint32_t ACTION_SET_IQ_BALANCE_AUTO = 18;
        static const uint32_t ACTION_SET_AGC             = 19;
        static const uint32_t ACTION_SET_AGC_MODE        = 20;
        static const uint32_t ACTION_SET_BW              = 21;
        static const uint32_t ACTION_GET_FREQ            = 22;

        //Values for "which"
        static const uint32_t CHAIN_NONE = 0;
        static const uint32_t CHAIN_TX1  = 1;
        static const uint32_t CHAIN_TX2  = 2;
        static const uint32_t CHAIN_RX1  = 3;
        static const uint32_t CHAIN_RX2  = 4;
    };

    static sptr make(shd::transport::zero_copy_if::sptr xport);
};

}}};

#endif /* INCLUDED_E300_REMOTE_CODEC_CTRL_HPP */
