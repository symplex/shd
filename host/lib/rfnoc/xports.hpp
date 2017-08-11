//
// Copyright 2016 Ettus Research LLC
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

#include <shd/types/sid.hpp>
#include <shd/transport/zero_copy.hpp>

namespace shd {

    /*! Holds all necessary items for a bidirectional link
     */
    struct both_xports_t
    {
        both_xports_t(): recv_buff_size(0), send_buff_size(0) {}
        shd::transport::zero_copy_if::sptr recv;
        shd::transport::zero_copy_if::sptr send;
        size_t recv_buff_size;
        size_t send_buff_size;
        shd::sid_t send_sid;
        shd::sid_t recv_sid;
    };

};

