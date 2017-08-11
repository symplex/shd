#
# Copyright 2010 Ettus Research LLC
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

include_directories(${CMAKE_SOURCE_DIR}/common)

#for smini_common.h and the regs files...
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#now make a lib to link against
set(libsmini1_sources 
    ${CMAKE_SOURCE_DIR}/common/delay.c
    ${CMAKE_SOURCE_DIR}/common/fx2utils.c
    ${CMAKE_SOURCE_DIR}/common/i2c.c
    ${CMAKE_SOURCE_DIR}/common/init_gpif.c
    ${CMAKE_SOURCE_DIR}/common/isr.c
    ${CMAKE_SOURCE_DIR}/common/timer.c
    ${CMAKE_SOURCE_DIR}/common/usb_common.c
#    ${CMAKE_SOURCE_DIR}/common/spi.c
#    ${CMAKE_SOURCE_DIR}/common/vectors.a51
)

#file(GLOB libsmini1_c_sources ${CMAKE_SOURCE_DIR}/common/*.c)
#file(GLOB libsmini1_a51_sources ${CMAKE_SOURCE_DIR}/common/*.a51)
#list(APPEND libsmini1_sources ${libsmini1_c_sources} ${libsmini1_a51_sources})

add_library(libsmini1 STATIC ${libsmini1_sources})

# edit-gpif hacks up gpif.c for our purposes. no major surgery, just moving stuff around.
set(GPIF_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/gpif.c)
set(GPIF_SOURCE_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/smini_gpif.c)
set(GPIF_HEADER_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/smini_gpif_inline.h)

add_custom_command(
        OUTPUT ${GPIF_SOURCE_OUTPUT}
        DEPENDS ${EDIT_GPIF_SMINI1}
        COMMAND ${PYTHON_EXECUTABLE} ${EDIT_GPIF_SMINI1} ${GPIF_SOURCE} ${GPIF_SOURCE_OUTPUT} ${GPIF_HEADER_OUTPUT}
        COMMENT "Generating ${GPIF_SOURCE_OUTPUT}"
)

#file(GLOB smini1_sources *.c)
set(smini1_sources 
    ${CMAKE_SOURCE_DIR}/common/vectors.a51
    board_specific.c 
    eeprom_io.c 
    fpga_load.c 
    fpga_rev2.c 
    smini_common.c 
    smini_gpif.c 
    smini_main.c 
    usb_descriptors.a51 
    ${CMAKE_SOURCE_DIR}/common/spi.c
    ${CMAKE_SOURCE_DIR}/common/_startup.a51
)
add_executable(smini1_fw ${smini1_sources})
target_link_libraries(smini1_fw libsmini1)

set(eeprom1_sources
    ${CMAKE_SOURCE_DIR}/common/eeprom_boot.a51
    ${CMAKE_SOURCE_DIR}/common/eeprom_init.c
    ${CMAKE_SOURCE_DIR}/common/_startup.a51
)

add_custom_target(smini1_eeprom ALL
    DEPENDS smini1_boot
    COMMAND objcopy -I ihex -O binary smini1_boot.ihx smini1_boot.bin
    COMMAND ${PYTHON_EXECUTABLE} ${BUILD_EEPROM} -r1 smini1_boot.bin smini1_eeprom.bin
)

add_executable(smini1_boot ${eeprom1_sources})
target_link_libraries(smini1_boot libsmini1)
