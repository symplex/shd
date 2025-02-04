//
// Copyright 2011-2012,2014 Ettus Research LLC
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

#ifndef INCLUDED_LIBSHD_SMINI_I2C_CORE_200_HPP
#define INCLUDED_LIBSHD_SMINI_I2C_CORE_200_HPP

#include <shd/config.hpp>
#include <shd/types/serial.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <shd/types/wb_iface.hpp>

class i2c_core_200 : boost::noncopyable, public shd::i2c_iface{
public:
    typedef boost::shared_ptr<i2c_core_200> sptr;

    virtual ~i2c_core_200(void) = 0;

    //! makes a new i2c core from iface and slave base
    static sptr make(shd::wb_iface::sptr iface, const size_t base, const size_t readback);
};

#endif /* INCLUDED_LIBSHD_SMINI_I2C_CORE_200_HPP */
