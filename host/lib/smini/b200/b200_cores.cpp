//
// Copyright 2013-2014 Ettus Research LLC
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

#include "b200_cores.hpp"
#include "b200_regs.hpp"
#include "b200_impl.hpp"

b200_local_spi_core::b200_local_spi_core(
    shd::wb_iface::sptr iface,
    perif_t default_perif) :
    _spi_core(spi_core_3000::make(iface, TOREG(SR_CORE_SPI), RB32_CORE_SPI)),
    _current_perif(default_perif),
    _last_perif(default_perif)
{
    change_perif(default_perif);
}

uint32_t b200_local_spi_core::transact_spi(
    int which_slave,
    const shd::spi_config_t &config,
    uint32_t data,
    size_t num_bits,
    bool readback)
{
    boost::mutex::scoped_lock lock(_mutex);
    return _spi_core->transact_spi(which_slave, config, data, num_bits, readback);
}

void b200_local_spi_core::change_perif(perif_t perif)
{
    boost::mutex::scoped_lock lock(_mutex);
    _last_perif = _current_perif;
    _current_perif = perif;

    switch (_current_perif) {
        case CODEC:
            _spi_core->set_divider(B200_BUS_CLOCK_RATE/AD9361_SPI_RATE);
            break;
        case PLL:
            _spi_core->set_divider(B200_BUS_CLOCK_RATE/ADF4001_SPI_RATE);
            break;
    }
}

void b200_local_spi_core::restore_perif()
{
    change_perif(_last_perif);
}

b200_ref_pll_ctrl::b200_ref_pll_ctrl(b200_local_spi_core::sptr spi) :
    shd::smini::adf4001_ctrl(spi, ADF4001_SLAVENO),
    _spi(spi)
{
}

void b200_ref_pll_ctrl::set_lock_to_ext_ref(bool external)
{
    _spi->change_perif(b200_local_spi_core::PLL);
    adf4001_ctrl::set_lock_to_ext_ref(external);
    _spi->restore_perif();
}


b200_local_spi_core::sptr b200_local_spi_core::make(
    shd::wb_iface::sptr iface, b200_local_spi_core::perif_t default_perif)
{
    return sptr(new b200_local_spi_core(iface, default_perif));
}

