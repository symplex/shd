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

#ifndef INCLUDED_E300_SPI_HPP
#define INCLUDED_E300_SPI_HPP

#include <shd/types/serial.hpp>

namespace shd { namespace smini { namespace e300 {

class spi : public virtual shd::spi_iface
{
public:
    typedef boost::shared_ptr<spi> sptr;
    static sptr make(const std::string &device);
};

}}};

#endif /* INCLUDED_E300_SPI_HPP */
