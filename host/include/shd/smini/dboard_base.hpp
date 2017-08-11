//
// Copyright 2010 Ettus Research LLC
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

#ifndef INCLUDED_SHD_SMINI_DBOARD_BASE_HPP
#define INCLUDED_SHD_SMINI_DBOARD_BASE_HPP

#include <shd/config.hpp>
#include <shd/property_tree.hpp>
#include <shd/utils/pimpl.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <shd/smini/dboard_id.hpp>
#include <shd/smini/dboard_iface.hpp>

namespace shd{ namespace smini{

/*!
 * A daughter board dboard_base class for all dboards.
 * Only other dboard dboard_base classes should inherit this.
 */
class SHD_API dboard_base : boost::noncopyable{
public:
    typedef boost::shared_ptr<dboard_base> sptr;
    /*!
     * An opaque type for the dboard constructor args.
     * Derived classes should pass the args into the base class,
     * but should not deal with the internals of the args.
     */
    typedef void * ctor_args_t;

    //structors
    dboard_base(ctor_args_t);
    virtual ~dboard_base() {}

    //post-construction initializer
    virtual void initialize() {}

protected:
    std::string get_subdev_name(void);
    dboard_iface::sptr get_iface(void);
    dboard_id_t get_rx_id(void);
    dboard_id_t get_tx_id(void);
    property_tree::sptr get_rx_subtree(void);
    property_tree::sptr get_tx_subtree(void);

private:
    SHD_PIMPL_DECL(impl) _impl;
};

/*!
 * A xcvr daughter board implements rx and tx methods
 * Sub classes for xcvr boards should inherit this.
 */
class SHD_API xcvr_dboard_base : public dboard_base{
public:
    /*!
     * Create a new xcvr dboard object, override in subclasses.
     */
    xcvr_dboard_base(ctor_args_t);
    virtual ~xcvr_dboard_base() {}
};

/*!
 * A rx daughter board only implements rx methods.
 * Sub classes for rx-only boards should inherit this.
 */
class SHD_API rx_dboard_base : public dboard_base{
public:
    /*!
     * Create a new rx dboard object, override in subclasses.
     */
    rx_dboard_base(ctor_args_t);
    virtual ~rx_dboard_base() {}
};

/*!
 * A tx daughter board only implements tx methods.
 * Sub classes for rx-only boards should inherit this.
 */
class SHD_API tx_dboard_base : public dboard_base{
public:
    /*!
     * Create a new rx dboard object, override in subclasses.
     */
    tx_dboard_base(ctor_args_t);
    virtual ~tx_dboard_base() {}
};

}} //namespace

#endif /* INCLUDED_SHD_SMINI_DBOARD_BASE_HPP */
