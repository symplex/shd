//
// Copyright 2010-2013 Ettus Research LLC
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

#include <shd/property_tree.hpp>
#include <shd/smini_clock/multi_smini_clock.hpp>
#include <shd/smini_clock/octoclock_eeprom.hpp>

#include <shd/utils/msg.hpp>
#include <shd/exception.hpp>
#include <shd/utils/log.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

using namespace shd;
using namespace shd::smini_clock;

/***********************************************************************
 * Multi SMINI Clock implementation
 **********************************************************************/
class multi_smini_clock_impl : public multi_smini_clock{
public:
    multi_smini_clock_impl(const device_addr_t &addr){
        _dev = device::make(addr, device::CLOCK);
        _tree = _dev->get_tree();
    }

    device::sptr get_device(void){
        return _dev;
    }

    std::string get_pp_string(void){
        std::string buff = str(boost::format("%s SMINI Clock Device\n")
            % ((get_num_boards() > 1) ? "Multi" : "Single")
        );
        for(size_t i = 0; i < get_num_boards(); i++){
            buff += str(boost::format("  Board %s\n") % i);
            buff += str(boost::format("    Reference: %s\n")
                        % (get_sensor("using_ref", i).value)
                    );
        }

        return buff;
    }

    size_t get_num_boards(void){
        return _tree->list("/mboards").size();
    }

    uint32_t get_time(size_t board){
        std::string board_str = str(boost::format("/mboards/%d") % board);

        return _tree->access<uint32_t>(board_str / "time").get();
    }

    sensor_value_t get_sensor(const std::string &name, size_t board){
        std::string board_str = str(boost::format("/mboards/%d") % board);

        return _tree->access<sensor_value_t>(board_str / "sensors" / name).get();
    }

    std::vector<std::string> get_sensor_names(size_t board){
        std::string board_str = str(boost::format("/mboards/%d") % board);

        return _tree->list(board_str / "sensors");
    }

private:
    device::sptr _dev;
    property_tree::sptr _tree;
};

multi_smini_clock::~multi_smini_clock(void){
    /* NOP */
}

/***********************************************************************
 * Multi SMINI Clock factory function
 **********************************************************************/
multi_smini_clock::sptr multi_smini_clock::make(const device_addr_t &dev_addr){
    SHD_LOG << "multi_smini_clock::make with args " << dev_addr.to_pp_string() << std::endl;
    return sptr(new multi_smini_clock_impl(dev_addr));
}
