#
# Copyright 2010-2013 Ettus Research LLC
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

all:
	@echo "Pick a specific target"

########################################################################
# Common Variables
########################################################################
TOP_DIR = $(shell pwd)
TOP_FW_DIR = $(TOP_DIR)/../firmware
TOP_FPGA_DIR = $(TOP_DIR)/../fpga
BUILT_IMAGES_DIR = $(TOP_DIR)/images
CMAKE_BUILD_DIR = $(TOP_DIR)/build

##filled in below
IMAGES_LIST =

##little rule to make the images directory
$(BUILT_IMAGES_DIR):
	mkdir $@
	mkdir $@/bit

GLOBAL_DEPS = $(BUILT_IMAGES_DIR)

########################################################################
# Utility Checks
########################################################################
ifeq ($(shell sdcc --help > /dev/null 2>&1 && echo $$?),0)
	HAS_SDCC=1
endif

ifeq ($(shell zpu-elf-gcc --help > /dev/null 2>&1 && echo $$?),0)
	HAS_ZPU_GCC=1
endif

ifeq ($(shell xtclsh -h > /dev/null 2>&1 && echo $$?),0)
	HAS_XTCLSH=1
endif

ifeq ($(shell avr-gcc --version > /dev/null 2>&1 && echo $$?),0)
    ifeq ($(shell avr-objcopy --version > /dev/null 2>&1 && echo $$?),0)
        HAS_AVR_UTILS=1
    endif
endif

########################################################################
# SMINI1 and B100 firmware
########################################################################
ifdef HAS_SDCC

_smini1_fw_dir = $(TOP_FW_DIR)/fx2
_smini1_fw_ihx = $(BUILT_IMAGES_DIR)/smini1_fw.ihx
_smini_b100_fw_ihx = $(BUILT_IMAGES_DIR)/smini_b100_fw.ihx
IMAGES_LIST += $(_smini1_fw_ihx) $(_smini_b100_fw_ihx)

$(_smini1_fw_ihx): $(GLOBAL_DEPS)
	cd $(_smini1_fw_dir) && rm -rf build
	cd $(_smini1_fw_dir) && mkdir build
	cd $(_smini1_fw_dir)/build && cmake ../
	cd $(_smini1_fw_dir)/build && make
	cp $(_smini1_fw_dir)/build/smini1/smini1_fw.ihx $@

$(_smini_b100_fw_ihx): $(_smini1_fw_ihx)
	cp $(_smini1_fw_dir)/build/b100/b100_fw.ihx $(_smini_b100_fw_ihx)

endif

########################################################################
# SMINI1 fpga
########################################################################
_smini1_fpga_dir = $(TOP_FPGA_DIR)/smini1/rbf/rev4
_smini1_fpga_rbf = $(BUILT_IMAGES_DIR)/smini1_fpga.rbf
_smini1_fpga_4rx_rbf = $(BUILT_IMAGES_DIR)/smini1_fpga_4rx.rbf
IMAGES_LIST += $(_smini1_fpga_rbf) $(_smini1_fpga_4rx_rbf)

$(_smini1_fpga_rbf): $(GLOBAL_DEPS)
	cp $(_smini1_fpga_dir)/std_2rxhb_2tx.rbf $@

$(_smini1_fpga_4rx_rbf): $(GLOBAL_DEPS)
	cp $(_smini1_fpga_dir)/std_4rx_0tx.rbf $@

########################################################################
# SMINI-B100 fpga
########################################################################
ifdef HAS_XTCLSH

_smini_b100_fpga_dir = $(TOP_FPGA_DIR)/smini2/top/B100
_smini_b100_fpga_bin = $(BUILT_IMAGES_DIR)/smini_b100_fpga.bin
_smini_b100_fpga_2rx_bin = $(BUILT_IMAGES_DIR)/smini_b100_fpga_2rx.bin
IMAGES_LIST += $(_smini_b100_fpga_bin) $(_smini_b100_fpga_2rx_bin)

$(_smini_b100_fpga_bin): $(GLOBAL_DEPS)
	cd $(_smini_b100_fpga_dir) && make -f Makefile.B100 clean
	cd $(_smini_b100_fpga_dir) && make -f Makefile.B100 bin
	cp $(_smini_b100_fpga_dir)/build-B100/B100.bin $@

$(_smini_b100_fpga_2rx_bin): $(GLOBAL_DEPS)
	cd $(_smini_b100_fpga_dir) && make -f Makefile.B100_2RX clean
	cd $(_smini_b100_fpga_dir) && make -f Makefile.B100_2RX bin
	cp $(_smini_b100_fpga_dir)/build-B100_2RX/B100.bin $@

endif

########################################################################
# SMINI2 and N-Series firmware
########################################################################
ifdef HAS_ZPU_GCC

_smini2_fw_dir = $(TOP_FW_DIR)/zpu
_smini2_fw_bin = $(BUILT_IMAGES_DIR)/smini2_fw.bin
_smini_n200_fw_bin = $(BUILT_IMAGES_DIR)/smini_n200_fw.bin
_smini_n210_fw_bin = $(BUILT_IMAGES_DIR)/smini_n210_fw.bin
IMAGES_LIST += $(_smini2_fw_bin) $(_smini_n200_fw_bin) $(_smini_n210_fw_bin)

$(_smini2_fw_bin): $(GLOBAL_DEPS)
	cd $(_smini2_fw_dir) && rm -rf build
	cd $(_smini2_fw_dir) && mkdir build
	cd $(_smini2_fw_dir)/build && cmake ../
	cd $(_smini2_fw_dir)/build && make
	cp $(_smini2_fw_dir)/build/smini2/smini2_txrx_shd.bin $@

$(_smini_n200_fw_bin): $(_smini2_fw_bin)
	cp $(_smini2_fw_dir)/build/smini2p/smini2p_txrx_shd.bin $@

$(_smini_n210_fw_bin): $(_smini2_fw_bin)
	cp $(_smini2_fw_dir)/build/smini2p/smini2p_txrx_shd.bin $@

endif

########################################################################
# SMINI2 FPGA
########################################################################
ifdef HAS_XTCLSH

_smini2_fpga_dir = $(TOP_FPGA_DIR)/smini2/top/SMINI2
_smini2_fpga_bin = $(BUILT_IMAGES_DIR)/smini2_fpga.bin
IMAGES_LIST += $(_smini2_fpga_bin)

$(_smini2_fpga_bin): $(GLOBAL_DEPS)
	cd $(_smini2_fpga_dir) && make clean
	cd $(_smini2_fpga_dir) && make bin
	cp $(_smini2_fpga_dir)/build/u2_rev3.bin $@

endif

########################################################################
# SMINI N200 R2/R3 FPGA
########################################################################
ifdef HAS_XTCLSH

_smini_n200_r3_fpga_dir = $(TOP_FPGA_DIR)/smini2/top/N2x0
_smini_n200_r3_fpga_bin = $(BUILT_IMAGES_DIR)/smini_n200_r3_fpga.bin
_smini_n200_r3_fpga_bit = $(BUILT_IMAGES_DIR)/bit/smini_n200_r3_fpga.bit
IMAGES_LIST += $(_smini_n200_r3_fpga_bin) $(_smini_n200_r3_fpga_bit)

$(_smini_n200_r3_fpga_bin): $(GLOBAL_DEPS)
	cd $(_smini_n200_r3_fpga_dir) && make -f Makefile.N200R3 clean
	cd $(_smini_n200_r3_fpga_dir) && make -f Makefile.N200R3 bin
	cp $(_smini_n200_r3_fpga_dir)/build-N200R3/u2plus.bin $@

$(_smini_n200_r3_fpga_bit): $(_smini_n200_r3_fpga_bin)
	cp $(_smini_n200_r3_fpga_dir)/build-N200R3/u2plus.bit $@

#its called r2 on the silkscreen, so make an r2
_smini_n200_r2_fpga_bin = $(BUILT_IMAGES_DIR)/smini_n200_r2_fpga.bin
IMAGES_LIST += $(_smini_n200_r2_fpga_bin)
$(_smini_n200_r2_fpga_bin): $(_smini_n200_r3_fpga_bin)
	cp $< $@

endif

########################################################################
# SMINI N210 R2/R3 FPGA
########################################################################
ifdef HAS_XTCLSH

_smini_n210_r3_fpga_dir = $(TOP_FPGA_DIR)/smini2/top/N2x0
_smini_n210_r3_fpga_bin = $(BUILT_IMAGES_DIR)/smini_n210_r3_fpga.bin
_smini_n210_r3_fpga_bit = $(BUILT_IMAGES_DIR)/bit/smini_n210_r3_fpga.bit
IMAGES_LIST += $(_smini_n210_r3_fpga_bin) $(_smini_n210_r3_fpga_bit)

$(_smini_n210_r3_fpga_bin): $(GLOBAL_DEPS)
	cd $(_smini_n210_r3_fpga_dir) && make -f Makefile.N210R3 clean
	cd $(_smini_n210_r3_fpga_dir) && make -f Makefile.N210R3 bin
	cp $(_smini_n210_r3_fpga_dir)/build-N210R3/u2plus.bin $@

$(_smini_n210_r3_fpga_bit): $(_smini_n210_r3_fpga_bin)
	cp $(_smini_n210_r3_fpga_dir)/build-N210R3/u2plus.bit $@

#its called r2 on the silkscreen, so make an r2
_smini_n210_r2_fpga_bin = $(BUILT_IMAGES_DIR)/smini_n210_r2_fpga.bin
IMAGES_LIST += $(_smini_n210_r2_fpga_bin)
$(_smini_n210_r2_fpga_bin): $(_smini_n210_r3_fpga_bin)
	cp $< $@

endif

########################################################################
# SMINI N200 R4 FPGA
########################################################################
ifdef HAS_XTCLSH

_smini_n200_r4_fpga_dir = $(TOP_FPGA_DIR)/smini2/top/N2x0
_smini_n200_r4_fpga_bin = $(BUILT_IMAGES_DIR)/smini_n200_r4_fpga.bin
_smini_n200_r4_fpga_bit = $(BUILT_IMAGES_DIR)/bit/smini_n200_r4_fpga.bit
IMAGES_LIST += $(_smini_n200_r4_fpga_bin) $(_smini_n200_r4_fpga_bit)

$(_smini_n200_r4_fpga_bin): $(GLOBAL_DEPS)
	cd $(_smini_n200_r4_fpga_dir) && make -f Makefile.N200R4 clean
	cd $(_smini_n200_r4_fpga_dir) && make -f Makefile.N200R4 bin
	cp $(_smini_n200_r4_fpga_dir)/build-N200R4/u2plus.bin $@

$(_smini_n200_r4_fpga_bit): $(_smini_n200_r4_fpga_bin)
	cp $(_smini_n200_r4_fpga_dir)/build-N200R4/u2plus.bit $@

endif

########################################################################
# SMINI N210 R4 FPGA
########################################################################
ifdef HAS_XTCLSH

_smini_n210_r4_fpga_dir = $(TOP_FPGA_DIR)/smini2/top/N2x0
_smini_n210_r4_fpga_bin = $(BUILT_IMAGES_DIR)/smini_n210_r4_fpga.bin
_smini_n210_r4_fpga_bit = $(BUILT_IMAGES_DIR)/bit/smini_n210_r4_fpga.bit
IMAGES_LIST += $(_smini_n210_r4_fpga_bin) $(_smini_n210_r4_fpga_bit)

$(_smini_n210_r4_fpga_bin): $(GLOBAL_DEPS)
	cd $(_smini_n210_r4_fpga_dir) && make -f Makefile.N210R4 clean
	cd $(_smini_n210_r4_fpga_dir) && make -f Makefile.N210R4 bin
	cp $(_smini_n210_r4_fpga_dir)/build-N210R4/u2plus.bin $@

$(_smini_n210_r4_fpga_bit): $(_smini_n210_r4_fpga_bin)
	cp $(_smini_n210_r4_fpga_dir)/build-N210R4/u2plus.bit $@

endif

########################################################################
# SMINI E100 FPGA
########################################################################
ifdef HAS_XTCLSH

_smini_e100_fpga_dir = $(TOP_FPGA_DIR)/smini2/top/E1x0
_smini_e100_fpga_bin = $(BUILT_IMAGES_DIR)/smini_e100_fpga_v2.bin
IMAGES_LIST += $(_smini_e100_fpga_bin)

$(_smini_e100_fpga_bin): $(GLOBAL_DEPS)
	cd $(_smini_e100_fpga_dir) && make -f Makefile.E100 clean
	cd $(_smini_e100_fpga_dir) && make -f Makefile.E100 bin
	cp $(_smini_e100_fpga_dir)/build-E100/E1x0.bin $@

endif

########################################################################
# SMINI E110 FPGA
########################################################################
ifdef HAS_XTCLSH

_smini_e110_fpga_dir = $(TOP_FPGA_DIR)/smini2/top/E1x0
_smini_e110_fpga_bin = $(BUILT_IMAGES_DIR)/smini_e110_fpga.bin
IMAGES_LIST += $(_smini_e110_fpga_bin)

$(_smini_e110_fpga_bin): $(GLOBAL_DEPS)
	cd $(_smini_e110_fpga_dir) && make -f Makefile.E110 clean
	cd $(_smini_e110_fpga_dir) && make -f Makefile.E110 bin
	cp $(_smini_e110_fpga_dir)/build-E110/E1x0.bin $@

endif

########################################################################
# OctoClock firmware
########################################################################

ifdef HAS_AVR_UTILS

_octoclock_fw_dir = $(TOP_FW_DIR)/octoclock
_octoclock_fw_bin = $(BUILT_IMAGES_DIR)/octoclock_r4_fw.hex
IMAGES_LIST += $(_octoclock_fw_bin)

$(_octoclock_fw_bin): $(GLOBAL_DEPS)
	cd $(_octoclock_fw_dir) && mkdir build
	cd $(_octoclock_fw_dir)/build && cmake ..
	cd $(_octoclock_fw_dir)/build && make
	cp $(TOP_FW_DIR)/octoclock/build/octoclock_r4_fw.hex $@
endif

########################################################################
# Build rules
########################################################################
images: $(IMAGES_LIST)
	find $(BUILT_IMAGES_DIR) -type f | xargs chmod -x
	find $(TOP_FPGA_DIR) -name "*.twr" | xargs grep constraint | grep met

package:
	mkdir -p $(CMAKE_BUILD_DIR)
	cd $(CMAKE_BUILD_DIR) && cmake -DSHD_RELEASE_MODE=OFF ..
	cd $(CMAKE_BUILD_DIR) && cmake -DCPACK_GENERATOR=TGZ .. && make package
	cd $(CMAKE_BUILD_DIR) && cmake -DCPACK_GENERATOR=ZIP .. && make package

clean:
	$(RM) -rf $(BUILT_IMAGES_DIR)
	$(RM) -rf $(CMAKE_BUILD_DIR)
