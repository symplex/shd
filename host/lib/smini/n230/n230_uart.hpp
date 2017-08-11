//
// Copyright 2013 Ettus Research LLC
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

#ifndef INCLUDED_N230_UART_HPP
#define INCLUDED_N230_UART_HPP

#include <shd/transport/zero_copy.hpp>
#include <shd/types/serial.hpp> //uart iface
#include <shd/utils/tasks.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
namespace shd { namespace smini { namespace n230 {

class n230_uart: boost::noncopyable, public shd::uart_iface
{
public:
    typedef boost::shared_ptr<n230_uart> sptr;
    static sptr make(shd::transport::zero_copy_if::sptr, const uint32_t sid);
    virtual void set_baud_divider(const double baud_div) = 0;
};

}}} //namespace

#endif /* INCLUDED_N230_UART_HPP */
