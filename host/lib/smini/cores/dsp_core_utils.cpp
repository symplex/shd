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

#include "dsp_core_utils.hpp"
#include <shd/utils/math.hpp>
#include <shd/exception.hpp>
#include <boost/math/special_functions/round.hpp>
#include <boost/math/special_functions/sign.hpp>

static const int32_t MAX_FREQ_WORD = boost::numeric::bounds<int32_t>::highest();
static const int32_t MIN_FREQ_WORD = boost::numeric::bounds<int32_t>::lowest();

void get_freq_and_freq_word(
        const double requested_freq,
        const double tick_rate,
        double &actual_freq,
        int32_t &freq_word
) {
    //correct for outside of rate (wrap around)
    double freq = std::fmod(requested_freq, tick_rate);
    if (std::abs(freq) > tick_rate/2.0)
        freq -= boost::math::sign(freq) * tick_rate;

    //confirm that the target frequency is within range of the CORDIC
    SHD_ASSERT_THROW(std::abs(freq) <= tick_rate/2.0);

    /* Now calculate the frequency word. It is possible for this calculation
     * to cause an overflow. As the requested DSP frequency approaches the
     * master clock rate, that ratio multiplied by the scaling factor (2^32)
     * will generally overflow within the last few kHz of tunable range.
     * Thus, we check to see if the operation will overflow before doing it,
     * and if it will, we set it to the integer min or max of this system.
     */
    freq_word = 0;

    static const double scale_factor = std::pow(2.0, 32);
    if ((freq / tick_rate) >= (MAX_FREQ_WORD / scale_factor)) {
        /* Operation would have caused a positive overflow of int32. */
        freq_word = MAX_FREQ_WORD;

    } else if ((freq / tick_rate) <= (MIN_FREQ_WORD / scale_factor)) {
        /* Operation would have caused a negative overflow of int32. */
        freq_word = MIN_FREQ_WORD;

    } else {
        /* The operation is safe. Perform normally. */
        freq_word = int32_t(boost::math::round((freq / tick_rate) * scale_factor));
    }

    actual_freq = (double(freq_word) / scale_factor) * tick_rate;
}

