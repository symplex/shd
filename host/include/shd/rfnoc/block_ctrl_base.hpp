//
// Copyright 2014-2016 Ettus Research LLC
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

#ifndef INCLUDED_LIBSHD_BLOCK_CTRL_BASE_HPP
#define INCLUDED_LIBSHD_BLOCK_CTRL_BASE_HPP

#include <shd/property_tree.hpp>
#include <shd/stream.hpp>
#include <shd/types/sid.hpp>
#include <shd/types/stream_cmd.hpp>
#include <shd/types/wb_iface.hpp>
#include <shd/utils/static.hpp>
#include <shd/rfnoc/node_ctrl_base.hpp>
#include <shd/rfnoc/block_id.hpp>
#include <shd/rfnoc/stream_sig.hpp>
#include <shd/rfnoc/blockdef.hpp>
#include <shd/rfnoc/constants.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <stdint.h>

namespace shd {
    namespace rfnoc {
        namespace nocscript {
            // Forward declaration
            class block_iface;
        }


// TODO: Move this out of public section
struct make_args_t
{
    make_args_t(const std::string &key="") :
        device_index(0),
        is_big_endian(true),
        block_name(""),
        block_key(key)
    {}

    //! A valid interface that allows us to do peeks and pokes
    std::map<size_t, shd::wb_iface::sptr> ctrl_ifaces;
    //! This block's base address (address of block port 0)
    uint32_t base_address;
    //! The device index (or motherboard index).
    size_t device_index;
    //! A property tree for this motherboard. Example: If the root a device's
    //  property tree is /mboards/0, pass a subtree starting at /mboards/0
    //  to the constructor.
    shd::property_tree::sptr tree;
    bool is_big_endian;
    //! The name of the block as it will be addressed
    std::string block_name;
    //! The key of the block, i.e. how it was registered
    std::string block_key;
};

//! This macro must be put in the public section of an RFNoC
// block class
#define SHD_RFNOC_BLOCK_OBJECT(class_name)  \
    typedef boost::shared_ptr< class_name > sptr;

//! Shorthand for block constructor
#define SHD_RFNOC_BLOCK_CONSTRUCTOR(CLASS_NAME) \
    CLASS_NAME##_impl( \
        const make_args_t &make_args \
    ) : block_ctrl_base(make_args)

//! This macro must be placed inside a block implementation file
// after the class definition
#define SHD_RFNOC_BLOCK_REGISTER(CLASS_NAME, BLOCK_NAME) \
    block_ctrl_base::sptr CLASS_NAME##_make( \
        const make_args_t &make_args \
    ) { \
        return block_ctrl_base::sptr(new CLASS_NAME##_impl(make_args)); \
    } \
    SHD_STATIC_BLOCK(register_rfnoc_##CLASS_NAME) \
    { \
        shd::rfnoc::block_ctrl_base::register_block(&CLASS_NAME##_make, BLOCK_NAME); \
    }

/*! \brief Base class for all RFNoC block controller objects.
 *
 * For RFNoC, block controller objects must be derived from
 * shd::rfnoc::block_ctrl_base. This class provides all functions
 * that a block *must* provide. Typically, you would not derive
 * a block controller class directly from block_ctrl_base, but
 * from a class such as shd::smini::rfnoc::source_block_ctrl_base or
 * shd::smini::rfnoc::sink_block_ctrl_base which extends its functionality.
 */
class SHD_RFNOC_API block_ctrl_base;
class block_ctrl_base : virtual public node_ctrl_base
{
public:
    /***********************************************************************
     * Types
     **********************************************************************/
    typedef boost::shared_ptr<block_ctrl_base> sptr;
    typedef boost::function<sptr(const make_args_t &)> make_t;

    /***********************************************************************
     * Factory functions
     **********************************************************************/

    /*! Register a block controller class into the discovery and factory system.
     *
     * Note: It is not recommended to call this function directly.
     * Rather, use the SHD_RFNOC_BLOCK_REGISTER() macro, which will set up
     * the discovery and factory system correctly.
     *
     * \param make A factory function that makes a block controller object
     * \param name A unique block name, e.g. 'FFT'. If a block has this block name,
     *             it will use \p make to generate the block controller class.
     */
    static void register_block(const make_t &make, const std::string &name);

    /*!
     * \brief Create a block controller class given a NoC-ID or a block name.
     *
     * If a block name is given in \p make_args, it will directly try to
     * generate a block of this type. If no block name is given, it will
     * look up a name using the NoC-ID and use that.
     * If it can't find a suitable block controller class, it will generate
     * a shd::rfnoc::block_ctrl. However, if a block name *is* specified,
     * it will throw a shd::runtime_error if this block type is not registered.
     *
     * \param make_args Valid make args.
     * \param noc_id The 64-Bit NoC-ID.
     * \return a shared pointer to a new device instance
     */
    static sptr make(const make_args_t &make_args, uint64_t noc_id = ~0);

    /***********************************************************************
     * Block Communication and Control
     *
     * These functions do not require communication with the FPGA.
     **********************************************************************/

    /*! Returns the 16-Bit address for this block.
     */
    uint32_t get_address(size_t block_port=0);

    /*! Returns the unique block ID for this block (e.g. "0/FFT_1").
     */
    block_id_t get_block_id() const { return _block_id; };

    /*! Shorthand for get_block_id().to_string()
     */
    std::string unique_id() const { return _block_id.to_string(); };

    /***********************************************************************
     * FPGA control & communication
     **********************************************************************/

    /*! Returns a list of valid ports that can be used for sr_write(), sr_read() etc.
     */
    std::vector<size_t> get_ctrl_ports() const;

    /*! Allows setting one register on the settings bus.
     *
     * Note: There is no address translation ("memory mapping") necessary.
     * Register 0 is 0, 1 is 1 etc.
     *
     * \param reg The settings register to write to.
     * \param data New value of this register.
     */
    void sr_write(const uint32_t reg, const uint32_t data, const size_t port = 0);

    /*! Allows setting one register on the settings bus.
     *
     * Like sr_write(), but takes a register name as argument.
     *
     * \param reg The settings register to write to.
     * \param data New value of this register.
     * \param port Port on which to write
     * \throw shd::key_error if \p reg is not a valid register name
     *
     */
    void sr_write(const std::string &reg, const uint32_t data, const size_t port = 0);

    /*! Allows reading one register on the settings bus (64-Bit version).
     *
     * \param reg The settings register to be read.
     * \param port Port on which to read
     *
     * Returns the readback value.
     */
    uint64_t sr_read64(const settingsbus_reg_t reg, const size_t port = 0);

    /*! Allows reading one register on the settings bus (32-Bit version).
     *
     * \param reg The settings register to be read.
     * \param port Port on which to read
     *
     * Returns the readback value.
     */
    uint32_t sr_read32(const settingsbus_reg_t reg, const size_t port = 0);

    /*! Allows reading one user-defined register (64-Bit version).
     *
     * This is a shorthand for setting the requested address
     * through sr_write() and then reading SR_READBACK_REG_USER
     * with sr_read64().
     *
     * \param addr The user register address.
     * \param port Port on which to read
     * \returns the readback value.
     */
    uint64_t user_reg_read64(const uint32_t addr, const size_t port = 0);

    /*! Allows reading one user-defined register (64-Bit version).
     *
     * Identical to user_reg_read64(), but takes a register name
     * instead of a numeric address. The register name must be
     * defined in the block definition file.
     *
     * \param addr The user register address.
     * \param port Port on which to read
     * \returns the readback value.
     * \throws shd::key_error if \p reg is not a valid register name
     */
    uint64_t user_reg_read64(const std::string &reg, const size_t port = 0);

    /*! Allows reading one user-defined register (32-Bit version).
     *
     * This is a shorthand for setting the requested address
     * through sr_write() and then reading SR_READBACK_REG_USER
     * with sr_read32().
     *
     * \param addr The user register address.
     * \param port Port on which to read
     * \returns the readback value.
     */
    uint32_t user_reg_read32(const uint32_t addr, const size_t port = 0);

    /*! Allows reading one user-defined register (32-Bit version).
     *
     * Identical to user_reg_read32(), but takes a register name
     * instead of a numeric address. The register name must be
     * defined in the block definition file.
     *
     * \param reg The user register name.
     * \returns the readback value.
     * \throws shd::key_error if \p reg is not a valid register name
     */
    uint32_t user_reg_read32(const std::string &reg, const size_t port = 0);


    /*! Sets a command time for all future command packets.
     *
     * \throws shd::assertion_error if the underlying interface does not
     *         actually support timing.
     */
    void set_command_time(const time_spec_t &time_spec, const size_t port = ANY_PORT);

    /*! Returns the current command time for all future command packets.
     *
     * \returns the command time as a time_spec_t.
     */
    time_spec_t get_command_time(const size_t port = 0);

    /*! Sets a tick rate for the command timebase.
     *
     * \param the tick rate in Hz
     * \port port Port
     */
    void set_command_tick_rate(const double tick_rate, const size_t port = ANY_PORT);

    /*! Resets the command time.
     * Any command packet after this call will no longer have a time associated
     * with it.
     *
     * \throws shd::assertion_error if the underlying interface does not
     *         actually support timing.
     */
    void clear_command_time(const size_t port);

    /*! Reset block after streaming operation.
     *
     * This does the following:
     * - Reset flow control (sequence numbers etc.)
     * - Clear the list of connected blocks
     *
     * Internally, rfnoc::node_ctrl_base::clear() and _clear() are called
     * (in that order).
     *
     * Between runs, it can be necessary to call this method,
     * or blocks might be left hanging in a streaming state, and can get
     * confused when a new application starts.
     *
     * For custom behaviour, overwrite _clear(). If you do so, you must take
     * take care of resetting flow control yourself.
     *
     * TODO: Find better name (it disconnects, clears FC...)
     */
    void clear();

    /***********************************************************************
     * Argument handling
     **********************************************************************/
    /*! Set multiple block args. Calls set_arg() for all individual items.
     *
     * Note that this function will silently ignore any keys in \p args that
     * aren't already registered as block arguments.
     */
    void set_args(const shd::device_addr_t &args, const size_t port = 0);

    //! Set a specific block argument. \p val is converted to the corresponding
    // data type using by looking up its type in the block definition.
    void set_arg(const std::string &key, const std::string &val, const size_t port = 0);

    //! Direct access to set a block argument.
    template <typename T>
    void set_arg(const std::string &key, const T &val, const size_t port = 0) {
        _tree->access<T>(get_arg_path(key, port) / "value").set(val);
    }

    //! Return all block arguments as a device_addr_t.
    shd::device_addr_t get_args(const size_t port = 0) const;

    //! Return a single block argument in string format.
    std::string get_arg(const std::string &key, const size_t port = 0) const;

    //! Direct access to get a block argument.
    template <typename T>
    T get_arg(const std::string &key, const size_t port = 0) const {
        return _tree->access<T>(get_arg_path(key, port) / "value").get();
    }

    std::string get_arg_type(const std::string &key, const size_t port = 0) const;

protected:
    /***********************************************************************
     * Structors
     **********************************************************************/
    block_ctrl_base(void) {}; // To allow pure virtual (interface) sub-classes
    virtual ~block_ctrl_base();

    /*! Constructor. This is only called from the internal block factory!
     *
     * \param make_args All arguments to this constructor are passed in this object.
     *                  Its details are subject to change. Use the SHD_RFNOC_BLOCK_CONSTRUCTOR()
     *                  macro to set up your block's constructor in a portable fashion.
     */
    block_ctrl_base(
            const make_args_t &make_args
    );

    /***********************************************************************
     * Helpers
     **********************************************************************/
    stream_sig_t _resolve_port_def(const blockdef::port_t &port_def) const;

    //! Return the property tree path to a block argument \p key on \p port
    shd::fs_path get_arg_path(const std::string &key, size_t port = 0) const {
        return _root_path / "args" / port / key;
    };

    //! Get a control interface object for block port \p block_port
    wb_iface::sptr get_ctrl_iface(const size_t block_port);


    /***********************************************************************
     * Hooks & Derivables
     **********************************************************************/

    //! Override this function if your block does something else
    // than reset register SR_CLEAR_TX_FC.
    virtual void _clear(const size_t port = 0);

    //! Override this function if your block needs to specially handle
    // setting the command time
    virtual void _set_command_time(const time_spec_t &time_spec, const size_t port = ANY_PORT);
    /***********************************************************************
     * Protected members
     **********************************************************************/

    //! Property sub-tree
    shd::property_tree::sptr _tree;

    //! Root node of this block's properties
    shd::fs_path _root_path;

    //! Endianness of underlying transport (for data transport)
    bool _transport_is_big_endian;

    //! Block definition (stores info about the block such as ports)
    blockdef::sptr _block_def;

private:
    //! Helper function to initialize the port definition nodes in the prop tree
    void _init_port_defs(
            const std::string &direction,
            blockdef::ports_t ports,
            const size_t first_port_index=0
    );

    //! Helper function to initialize the block args (used by ctor only)
    void _init_block_args();

    /***********************************************************************
     * Private members
     **********************************************************************/
    //! Objects to actually send and receive the commands
    std::map<size_t, wb_iface::sptr> _ctrl_ifaces;

    //! The base address of this block (the address of block port 0)
    uint32_t _base_address;

    //! The (unique) block ID.
    block_id_t _block_id;

    //! Interface to NocScript parser
    boost::shared_ptr<nocscript::block_iface> _nocscript_iface;
}; /* class block_ctrl_base */

}} /* namespace shd::rfnoc */

#endif /* INCLUDED_LIBSHD_BLOCK_CTRL_BASE_HPP */
// vim: sw=4 et:
