//
// Copyright 2013,2015 Ettus Research LLC
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
#include <shd/transport/nirio/nirio_driver_iface.h>

namespace nirio_driver_iface {

nirio_status rio_open(
    SHD_UNUSED(const std::string& device_path),
    SHD_UNUSED(rio_dev_handle_t& device_handle))
{
    return NiRio_Status_FeatureNotSupported;
}

void rio_close(SHD_UNUSED(rio_dev_handle_t& device_handle))
{
}

bool rio_isopen(SHD_UNUSED(rio_dev_handle_t device_handle))
{
    return false;
}

nirio_status rio_ioctl(
    SHD_UNUSED(rio_dev_handle_t device_handle),
    SHD_UNUSED(uint32_t ioctl_code),
    SHD_UNUSED(const void *write_buf),
    SHD_UNUSED(size_t write_buf_len),
    SHD_UNUSED(void *read_buf),
    SHD_UNUSED(size_t read_buf_len))
{
    return NiRio_Status_FeatureNotSupported;
}

nirio_status rio_mmap(
    SHD_UNUSED(rio_dev_handle_t device_handle),
    SHD_UNUSED(uint16_t memory_type),
    SHD_UNUSED(size_t size),
    SHD_UNUSED(bool writable),
    SHD_UNUSED(rio_mmap_t &map))
{
    return NiRio_Status_FeatureNotSupported;
}

nirio_status rio_munmap(SHD_UNUSED(rio_mmap_t &map))
{
    return NiRio_Status_FeatureNotSupported;
}

}
