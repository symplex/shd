/*
 * Copyright 2015 Ettus Research LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDED_SHD_TYPES_SMINI_INFO_H
#define INCLUDED_SHD_TYPES_SMINI_INFO_H

#include <shd/config.h>
#include <shd/error.h>

//! SMINI RX info
/*!
 * This struct is populated by shd_smini_get_rx_info().
 */
typedef struct {
    //! Motherboard ID
    char* mboard_id;
    //! Motherboard name
    char* mboard_name;
    //! Motherboard serial
    char* mboard_serial;
    //! RX daughterboard ID
    char* rx_id;
    //! RX subdev name
    char* rx_subdev_name;
    //! RX subdev spec
    char* rx_subdev_spec;
    //! RX daughterboard serial
    char* rx_serial;
    //! RX daughterboard antenna
    char* rx_antenna;
} shd_smini_rx_info_t;

//! SMINI TX info
/*!
 * This struct is populated by shd_smini_get_tx_info().
 */
typedef struct {
    //! Motherboard ID
    char* mboard_id;
    //! Motherboard name
    char* mboard_name;
    //! Motherboard serial
    char* mboard_serial;
    //! TX daughterboard ID
    char* tx_id;
    //! TX subdev name
    char* tx_subdev_name;
    //! TX subdev spec
    char* tx_subdev_spec;
    //! TX daughterboard serial
    char* tx_serial;
    //! TX daughterboard antenna
    char* tx_antenna;
} shd_smini_tx_info_t;

#ifdef __cplusplus
extern "C" {
#endif

//! Clean up a shd_smini_rx_info_t populated by shd_smini_get_rx_info().
/*!
 * NOTE: If this function is passed a shd_smini_rx_info_t that has not
 * been populated by shd_smini_get_rx_info(), it will produce a double-free
 * error.
 */
SHD_API shd_error shd_smini_rx_info_free(shd_smini_rx_info_t *rx_info);

//! Clean up a shd_smini_tx_info_t populated by shd_smini_get_tx_info().
/*!
 * NOTE: If this function is passed a shd_smini_tx_info_t that has not
 * been populated by shd_smini_get_tx_info(), it will produce a double-free
 * error.
 */
SHD_API shd_error shd_smini_tx_info_free(shd_smini_tx_info_t *tx_info);

#ifdef __cplusplus
}
#endif

#endif /* INCLUDED_SHD_TYPES_SMINI_INFO_H */
