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

#include <shd/smini/fe_connection.hpp>
#include <shd/exception.hpp>
#include <boost/regex.hpp>
#include <shd/utils/math.hpp>

using namespace shd::smini;

fe_connection_t::fe_connection_t(
    sampling_t sampling_mode, bool iq_swapped,
    bool i_inverted, bool q_inverted, double if_freq
) : _sampling_mode(sampling_mode), _iq_swapped(iq_swapped),
    _i_inverted(i_inverted), _q_inverted(q_inverted), _if_freq(if_freq)
{
}

fe_connection_t::fe_connection_t(const std::string& conn_str, double if_freq) {
    static const boost::regex conn_regex("([IQ])(b?)(([IQ])(b?))?");
    boost::cmatch matches;
    if (boost::regex_match(conn_str.c_str(), matches, conn_regex)) {
        if (matches[3].length() == 0) {
            //Connection in {I, Q, Ib, Qb}
            _sampling_mode = REAL;
            _iq_swapped = (matches[1].str() == "Q");
            _i_inverted = (matches[2].length() != 0);
            _q_inverted = false;    //IQ is swapped after inversion
        } else {
            //Connection in {I(b?)Q(b?), Q(b?)I(b?), I(b?)I(b?), Q(b?)Q(b?)}
            _sampling_mode = (matches[1].str() == matches[4].str()) ? HETERODYNE : QUADRATURE;
            _iq_swapped = (matches[1].str() == "Q");
            size_t i_idx = _iq_swapped ? 5 : 2, q_idx = _iq_swapped ? 2 : 5;
            _i_inverted = (matches[i_idx].length() != 0);
            _q_inverted = (matches[q_idx].length() != 0);

            if (_sampling_mode == HETERODYNE and _i_inverted != _q_inverted) {
                throw shd::value_error("Invalid connection string: " + conn_str);
            }
        }
        _if_freq = if_freq;
    } else {
        throw shd::value_error("Invalid connection string: " + conn_str);
    }
}

bool shd::smini::operator==(const fe_connection_t &lhs, const fe_connection_t &rhs){
    return ((lhs.get_sampling_mode() == rhs.get_sampling_mode()) and
            (lhs.is_iq_swapped() == rhs.is_iq_swapped()) and
            (lhs.is_i_inverted() == rhs.is_i_inverted()) and
            (lhs.is_q_inverted() == rhs.is_q_inverted()) and
            shd::math::frequencies_are_equal(lhs.get_if_freq(), rhs.get_if_freq()));
}
