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

#ifndef INCLUDED_LIBSHD_RFNOC_BLOCK_CTRL_HPP
#define INCLUDED_LIBSHD_RFNOC_BLOCK_CTRL_HPP

#include <shd/rfnoc/source_block_ctrl_base.hpp>
#include <shd/rfnoc/sink_block_ctrl_base.hpp>

namespace shd {
    namespace rfnoc {

/*! \brief This is the default implementation of a block_ctrl_base.
 *
 * For most blocks, this will be a sufficient implementation. All registers
 * can be set by sr_write(). The default behaviour of functions is documented
 * in shd::rfnoc::block_ctrl_base.
 */
class SHD_RFNOC_API block_ctrl : public source_block_ctrl_base, public sink_block_ctrl_base
{
public:
    // Required macro in RFNoC block classes
    SHD_RFNOC_BLOCK_OBJECT(block_ctrl)

    // Nothing else here -- all function definitions are in block_ctrl_base,
    // source_block_ctrl_base and sink_block_ctrl_base

}; /* class block_ctrl*/

}} /* namespace shd::rfnoc */

#endif /* INCLUDED_LIBSHD_RFNOC_BLOCK_CTRL_HPP */
// vim: sw=4 et:
