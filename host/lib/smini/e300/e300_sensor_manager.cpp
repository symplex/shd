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

#include "e300_sensor_manager.hpp"

#include <boost/thread.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/format.hpp>

#include <cstring>
#include <shd/exception.hpp>
#include <shd/utils/byteswap.hpp>

namespace shd { namespace smini { namespace e300 {

class e300_sensor_proxy : public e300_sensor_manager
{
public:
    e300_sensor_proxy(
        shd::transport::zero_copy_if::sptr xport) : _xport(xport)
    {
    }

    std::vector<std::string> get_sensors()
    {
        return boost::assign::list_of("temp")("ref_locked");
    }

    shd::sensor_value_t get_sensor(const std::string &key)
    {
        if (key == "temp")
            return get_mb_temp();
        else if (key == "ref_locked")
            return get_ref_lock();
        else
            throw shd::lookup_error(
                str(boost::format("Invalid sensor %s requested.") % key));
    }

    shd::sensor_value_t get_mb_temp(void)
    {
        boost::mutex::scoped_lock(_mutex);
        sensor_transaction_t transaction;
        transaction.which = shd::htonx<uint32_t>(ZYNQ_TEMP);
        {
            shd::transport::managed_send_buffer::sptr buff
                = _xport->get_send_buff(1.0);
            if (not buff or buff->size() < sizeof(transaction)) {
                throw shd::runtime_error("sensor proxy send timeout");
            }
            std::memcpy(
                buff->cast<void *>(),
                &transaction,
                sizeof(transaction));
            buff->commit(sizeof(transaction));
        }
        {
            shd::transport::managed_recv_buffer::sptr buff
                = _xport->get_recv_buff(1.0);

            if (not buff or buff->size() < sizeof(transaction))
                throw shd::runtime_error("sensor proxy recv timeout");

            std::memcpy(
                &transaction,
                buff->cast<const void *>(),
                sizeof(transaction));
        }
        SHD_ASSERT_THROW(shd::ntohx<uint32_t>(transaction.which) == ZYNQ_TEMP);
        // TODO: Use proper serialization here ...
        return sensor_value_t(
            "temp",
            e300_sensor_manager::unpack_float_from_uint32_t(
                shd::ntohx(transaction.value)),
            "C");
    }

    shd::sensor_value_t get_ref_lock(void)
    {
        boost::mutex::scoped_lock(_mutex);
        sensor_transaction_t transaction;
        transaction.which = shd::htonx<uint32_t>(REF_LOCK);
        {
            shd::transport::managed_send_buffer::sptr buff
                = _xport->get_send_buff(1.0);
            if (not buff or buff->size() < sizeof(transaction)) {
                throw shd::runtime_error("sensor proxy send timeout");
            }
            std::memcpy(
                buff->cast<void *>(),
                &transaction,
                sizeof(transaction));
            buff->commit(sizeof(transaction));
        }
        {
            shd::transport::managed_recv_buffer::sptr buff
                = _xport->get_recv_buff(1.0);

            if (not buff or buff->size() < sizeof(transaction))
                throw shd::runtime_error("sensor proxy recv timeout");

            std::memcpy(
                &transaction,
                buff->cast<const void *>(),
                sizeof(transaction));
        }
        SHD_ASSERT_THROW(shd::ntohx<uint32_t>(transaction.which) == REF_LOCK);
        // TODO: Use proper serialization here ...
        return sensor_value_t("Ref", (shd::ntohx(transaction.value) > 0), "locked", "unlocked");
    }

private:
    shd::transport::zero_copy_if::sptr _xport;
    boost::mutex                       _mutex;
};

}}} // namespace

using namespace shd::smini::e300;

e300_sensor_manager::sptr e300_sensor_manager::make_proxy(
    shd::transport::zero_copy_if::sptr xport)
{
    return sptr(new e300_sensor_proxy(xport));
}

#ifdef E300_NATIVE
#include "e300_fifo_config.hpp"

namespace shd { namespace smini { namespace e300 {

static const std::string E300_TEMP_SYSFS = "iio:device0";

class e300_sensor_local : public e300_sensor_manager
{
public:
    e300_sensor_local(global_regs::sptr global_regs) :
        _global_regs(global_regs)
    {
    }

    std::vector<std::string> get_sensors()
    {
        return boost::assign::list_of("temp")("ref_locked");
    }

    shd::sensor_value_t get_sensor(const std::string &key)
    {
        if (key == "temp")
            return get_mb_temp();
        else if (key == "ref_locked")
            return get_ref_lock();
        else
            throw shd::lookup_error(
                str(boost::format("Invalid sensor %s requested.") % key));
    }

    shd::sensor_value_t get_mb_temp(void)
    {
        double scale = boost::lexical_cast<double>(
            e300_get_sysfs_attr(E300_TEMP_SYSFS, "in_temp0_scale"));
        unsigned long raw = boost::lexical_cast<unsigned long>(
            e300_get_sysfs_attr(E300_TEMP_SYSFS, "in_temp0_raw"));
        unsigned long offset = boost::lexical_cast<unsigned long>(
            e300_get_sysfs_attr(E300_TEMP_SYSFS, "in_temp0_offset"));
        return sensor_value_t("temp", (raw + offset) * scale / 1000, "C");
    }

    shd::sensor_value_t get_ref_lock(void)
    {
        //PPSLOOP_LOCKED_MASK is asserted in the following cases:
        //- (Time source = GPS or External) AND (Loop is locked and is in fine adj mode)
        //- Time source is Internal
        static const uint32_t PPSLOOP_LOCKED_MASK = 0x04;
        static const uint32_t REFPLL_LOCKED_MASK = 0x20;

        const uint32_t status =
            _global_regs->peek32(global_regs::RB32_CORE_MISC);
        bool ref_locked = (status & PPSLOOP_LOCKED_MASK) && (status & REFPLL_LOCKED_MASK);

        return sensor_value_t("Ref", ref_locked, "locked", "unlocked");
    }

private:
    global_regs::sptr   _global_regs;
};
}}}

using namespace shd::smini::e300;
e300_sensor_manager::sptr e300_sensor_manager::make_local(
    global_regs::sptr global_regs)
{
    return sptr(new e300_sensor_local(global_regs));
}

#else
using namespace shd::smini::e300;
e300_sensor_manager::sptr e300_sensor_manager::make_local(
    global_regs::sptr)
{
    throw shd::assertion_error("e300_sensor_manager::make_local() !E300_NATIVE");
}
#endif
