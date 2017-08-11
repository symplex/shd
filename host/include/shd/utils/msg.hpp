//
// Copyright 2011-2013 Ettus Research LLC
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

#ifndef INCLUDED_SHD_UTILS_MSG_HPP
#define INCLUDED_SHD_UTILS_MSG_HPP

#include <shd/config.hpp>
#include <shd/utils/pimpl.hpp>
#include <ostream>
#include <iomanip>
#include <string>

/*!
 * A SHD message macro with configurable type.
 * Usage: SHD_MSG(warning) << "some warning message" << std::endl;
 */
#define SHD_MSG(type) \
    shd::msg::_msg(shd::msg::type)()

//! Helpful debug tool to print site info
#define SHD_HERE() \
    SHD_MSG(status) << __FILE__ << ":" << __LINE__ << std::endl

//! Helpful debug tool to print a variable
#define SHD_VAR(var) \
    SHD_MSG(status) << #var << " = " << var << std::endl;

//! Helpful debug tool to print a variable in hex
#define SHD_HEX(var) \
    SHD_MSG(status) << #var << " = 0x" << std::hex << std::setfill('0') << std::setw(8) << var << std::dec << std::endl;

namespace shd{ namespace msg{

    //! Possible message types
    enum type_t{
        status  = 's',
        warning = 'w',
        error   = 'e',
        fastpath= 'f'
    };

    //! Typedef for a user-registered message handler
    typedef void (*handler_t)(type_t, const std::string &);

    /*!
     * Register the handler for shd system messages.
     * Only one handler can be registered at once.
     * This replaces the default std::cout/cerr handler.
     * \param handler a new handler callback function
     */
    SHD_API void register_handler(const handler_t &handler);

    //! Internal message object (called by SHD_MSG macro)
    class SHD_API _msg{
    public:
        _msg(const type_t type);
        ~_msg(void);
        std::ostream &operator()(void);
    private:
        SHD_PIMPL_DECL(impl) _impl;
    };

}} //namespace shd::msg

#endif /* INCLUDED_SHD_UTILS_MSG_HPP */
