//
// Copyright 2015 Ettus Research LLC
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

#include <shd/error.h>
#include <shd/utils/thread_priority.h>
#include <shd/utils/thread_priority.hpp>
#include <shd/utils/msg.hpp>
#include <shd/exception.hpp>
#include <boost/format.hpp>
#include <iostream>

shd_error shd_set_thread_priority(
    float priority,
    bool realtime
){
    SHD_SAFE_C(
        shd::set_thread_priority(priority, realtime);
    )
}
