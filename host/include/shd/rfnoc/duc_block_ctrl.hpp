//
// Copyright 2016 Ettus Research
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

#ifndef INCLUDED_LIBSHD_RFNOC_DUC_BLOCK_CTRL_HPP
#define INCLUDED_LIBSHD_RFNOC_DUC_BLOCK_CTRL_HPP

#include <shd/rfnoc/source_block_ctrl_base.hpp>
#include <shd/rfnoc/sink_block_ctrl_base.hpp>
#include <shd/rfnoc/rate_node_ctrl.hpp>
#include <shd/rfnoc/scalar_node_ctrl.hpp>

namespace shd {
    namespace rfnoc {

/*! \brief DUC block controller
 *
 * This block provides DSP for Tx operations.
 * Its main component is a DUC chain, which can interpolate over a wide range
 * of interpolation rates (using a CIC and halfband filters).
 *
 * It also includes a CORDIC component to shift signals in frequency.
 */
class SHD_RFNOC_API duc_block_ctrl :
    public source_block_ctrl_base,
    public sink_block_ctrl_base,
    public rate_node_ctrl,
    public scalar_node_ctrl
{
public:
    SHD_RFNOC_BLOCK_OBJECT(duc_block_ctrl)

}; /* class duc_block_ctrl*/

}} /* namespace shd::rfnoc */

#endif /* INCLUDED_LIBSHD_RFNOC_DUC_BLOCK_CTRL_HPP */

