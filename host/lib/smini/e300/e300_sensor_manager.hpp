//
// Copyright 2014-2015 Ettus Research LLC
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

#include <boost/noncopyable.hpp>
#include <stdint.h>

#include <shd/transport/zero_copy.hpp>
#include <shd/types/sensors.hpp>
#include <shd/utils/byteswap.hpp>
#include <shd/smini/gps_ctrl.hpp>
#include "e300_global_regs.hpp"

#ifndef INCLUDED_E300_SENSOR_MANAGER_HPP
#define INCLUDED_E300_SENSOR_MANAGER_HPP

namespace shd { namespace smini { namespace e300 {

struct sensor_transaction_t {
    uint32_t which;
    union {
        uint32_t value;
        uint32_t value64;
    };
};



enum sensor {ZYNQ_TEMP=0, REF_LOCK=4};

class e300_sensor_manager : boost::noncopyable
{
public:
    typedef boost::shared_ptr<e300_sensor_manager> sptr;

    virtual ~e300_sensor_manager() {};

    virtual shd::sensor_value_t get_sensor(const std::string &key) = 0;
    virtual std::vector<std::string> get_sensors(void) = 0;

    virtual shd::sensor_value_t get_mb_temp(void) = 0;
    virtual shd::sensor_value_t get_ref_lock(void) = 0;


    static sptr make_proxy(shd::transport::zero_copy_if::sptr xport);
    static sptr make_local(global_regs::sptr global_regs);

    // Note: This is a hack
    static uint32_t pack_float_in_uint32_t(const float &v)
    {
        const uint32_t *cast = reinterpret_cast<const uint32_t*>(&v);
        return *cast;
    }

    static float unpack_float_from_uint32_t(const uint32_t &v)
    {
        const float *cast = reinterpret_cast<const float*>(&v);
        return *cast;
    }
};


}}} // namespace

#endif // INCLUDED_E300_SENSOR_MANAGER_HPP
