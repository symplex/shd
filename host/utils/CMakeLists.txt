#
# Copyright 2010-2015 Ettus Research LLC
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

########################################################################
# Utilities that get installed into the runtime path
########################################################################
SET(util_runtime_sources
    shd_config_info.cpp
    shd_find_devices.cpp
    shd_smini_probe.cpp
    shd_image_loader.cpp
    shd_cal_rx_iq_balance.cpp
    shd_cal_tx_dc_offset.cpp
    shd_cal_tx_iq_balance.cpp
    smini_n2xx_simple_net_burner.cpp
)

SET(x3xx_burner_sources
    smini_x3xx_fpga_burner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/smini/x300/cdecode.c
)

find_package(UDev)
IF(ENABLE_E300 AND NOT E300_FORCE_NETWORK)
    IF(UDEV_FOUND)
        LIST(APPEND util_runtime_sources smini_e3x0_network_mode.cpp)
    ENDIF(UDEV_FOUND)
ENDIF(ENABLE_E300 AND NOT E300_FORCE_NETWORK)

#for each source: build an executable and install
FOREACH(util_source ${util_runtime_sources})
    GET_FILENAME_COMPONENT(util_name ${util_source} NAME_WE)
    ADD_EXECUTABLE(${util_name} ${util_source})
    TARGET_LINK_LIBRARIES(${util_name} shd ${Boost_LIBRARIES})
    SHD_INSTALL(TARGETS ${util_name} RUNTIME DESTINATION ${RUNTIME_DIR} COMPONENT utilities)
ENDFOREACH(util_source)

IF(ENABLE_X300)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/smini/x300)
    ADD_EXECUTABLE(smini_x3xx_fpga_burner ${x3xx_burner_sources})
    TARGET_LINK_LIBRARIES(smini_x3xx_fpga_burner shd ${Boost_LIBRARIES})
    SHD_INSTALL(TARGETS smini_x3xx_fpga_burner RUNTIME DESTINATION ${RUNTIME_DIR} COMPONENT utilities)
ENDIF(ENABLE_X300)

########################################################################
# Utilities that get installed into the share path
########################################################################
SET(util_share_sources
    converter_benchmark.cpp
    query_gpsdo_sensors.cpp
    smini_burn_db_eeprom.cpp
    smini_burn_mb_eeprom.cpp
)
SET(util_share_sources_py
    converter_benchmark.py
)
IF(ENABLE_USB)
    LIST(APPEND util_share_sources
        fx2_init_eeprom.cpp
    )
    INCLUDE_DIRECTORIES(${LIBUSB_INCLUDE_DIRS})
    IF(ENABLE_B200)
        LIST(APPEND util_share_sources
            b2xx_fx3_utils.cpp
        )
        # Additional include directories for b2xx_fx3_utils
        INCLUDE_DIRECTORIES(
            ${CMAKE_CURRENT_SOURCE_DIR}/../lib/smini/b200
            ${CMAKE_CURRENT_SOURCE_DIR}/../lib/smini/common
            ${CMAKE_CURRENT_SOURCE_DIR}/../lib/smini/common/ad9361_driver
        )
    ENDIF(ENABLE_B200)
ENDIF(ENABLE_USB)
IF(ENABLE_OCTOCLOCK)
    LIST(APPEND util_share_sources
        octoclock_burn_eeprom.cpp
    )

    SET(octoclock_burner_sources
        octoclock_firmware_burner.cpp
        ${CMAKE_SOURCE_DIR}/lib/utils/ihex.cpp
    )

    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/smini_clock/octoclock)
    ADD_EXECUTABLE(octoclock_firmware_burner ${octoclock_burner_sources})
    TARGET_LINK_LIBRARIES(octoclock_firmware_burner shd ${Boost_LIBRARIES})
    SHD_INSTALL(TARGETS octoclock_firmware_burner RUNTIME DESTINATION ${RUNTIME_DIR} COMPONENT utilities)
ENDIF(ENABLE_OCTOCLOCK)

IF(LINUX AND ENABLE_USB)
    SHD_INSTALL(FILES
        shd-smini.rules
        DESTINATION ${PKG_LIB_DIR}/utils
        COMPONENT utilities
    )
ENDIF(LINUX AND ENABLE_USB)

#for each source: build an executable and install
FOREACH(util_source ${util_share_sources})
    GET_FILENAME_COMPONENT(util_name ${util_source} NAME_WE)
    ADD_EXECUTABLE(${util_name} ${util_source})
    TARGET_LINK_LIBRARIES(${util_name} shd ${Boost_LIBRARIES})
    SHD_INSTALL(TARGETS ${util_name} RUNTIME DESTINATION ${PKG_LIB_DIR}/utils COMPONENT utilities)
ENDFOREACH(util_source)
FOREACH(util_source ${util_share_sources_py})
    SHD_INSTALL(PROGRAMS
        ${CMAKE_CURRENT_SOURCE_DIR}/${util_source}
        DESTINATION ${PKG_LIB_DIR}/utils
        COMPONENT utilities
    )
ENDFOREACH(util_source)

IF(ENABLE_SMINI2)
    SHD_INSTALL(TARGETS smini_n2xx_simple_net_burner RUNTIME DESTINATION ${PKG_LIB_DIR}/utils COMPONENT utilities)
ENDIF(ENABLE_SMINI2)
IF(ENABLE_X300)
    SHD_INSTALL(TARGETS smini_x3xx_fpga_burner RUNTIME DESTINATION ${PKG_LIB_DIR}/utils COMPONENT utilities)
ENDIF(ENABLE_X300)

#SHD images downloader configuration
CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/shd_images_downloader.py.in
    ${CMAKE_CURRENT_BINARY_DIR}/shd_images_downloader.py
@ONLY)
SHD_INSTALL(PROGRAMS
    ${CMAKE_CURRENT_BINARY_DIR}/shd_images_downloader.py
    DESTINATION ${PKG_LIB_DIR}/utils
    COMPONENT utilities
)
IF(LINUX)
    SHD_INSTALL(PROGRAMS
        ${CMAKE_CURRENT_BINARY_DIR}/shd_images_downloader.py
        RENAME shd_images_downloader
        DESTINATION ${RUNTIME_DIR}
        COMPONENT utilities
    )
ENDIF(LINUX)
IF(NOT HAVE_PYTHON_MODULE_REQUESTS)
    MESSAGE(WARNING "Module requests not found -- shd_images_downloader.py will not work without it.")
ENDIF(NOT HAVE_PYTHON_MODULE_REQUESTS)

IF(ENABLE_SMINI2)
    SET(burners
        smini2_card_burner.py
        smini2_card_burner_gui.py
        smini_n2xx_net_burner.py
        smini_n2xx_net_burner_gui.py
    )

    IF(WIN32 AND SHD_RELEASE_MODE) #include dd.exe
        FILE(DOWNLOAD
            "http://files.ettus.com/dd.exe"
            ${CMAKE_CURRENT_BINARY_DIR}/dd.exe
        )
        SHD_INSTALL(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/dd.exe
            DESTINATION ${PKG_LIB_DIR}/utils
            COMPONENT utilities
        )
    ENDIF(WIN32 AND SHD_RELEASE_MODE)
    IF(LINUX)
        SHD_INSTALL(PROGRAMS
            smini2_recovery.py
            DESTINATION ${PKG_LIB_DIR}/utils
            COMPONENT utilities
        )
        SHD_INSTALL(PROGRAMS
            smini2_card_burner.py
            RENAME smini2_card_burner
            DESTINATION ${RUNTIME_DIR}
            COMPONENT utilities
        )
    ENDIF(LINUX)
    FOREACH(burner ${burners})
        SHD_INSTALL(PROGRAMS
            ${burner}
            DESTINATION ${PKG_LIB_DIR}/utils
            COMPONENT utilities
        )
    ENDFOREACH(burner ${burners})

ENDIF(ENABLE_SMINI2)

########################################################################
# Other files that are not utilities or executables
########################################################################
IF(WIN32)
    SET(windows_extra_files
        FastSendDatagramThreshold.reg
    )
    SHD_INSTALL(
        FILES ${windows_extra_files}
        DESTINATION ${PKG_DATA_DIR}
        COMPONENT utilities
    )
ENDIF(WIN32)

ADD_SUBDIRECTORY(latency)
