# Generation 2 SMINI Build Documentation

## Dependencies and Requirements

### Dependencies

The SMINI FPGA build system requires a UNIX-like environment with the following dependencies

- [Xilinx ISE 12.2](http://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/design-tools/v12_2.html)
- [GNU Make](https://www.gnu.org/software/make/)
- (Recommended) [GNU Bash](https://www.gnu.org/software/bash/)

### Requirements

- [Xilinx ISE Platform Requirements](http://www.xilinx.com/support/documentation/sw_manuals/xilinx12_2/irn.pdf)

### What FPGA does my SMINI have?

- SMINI N200: Spartan&reg; 3A-DSP 1800
- SMINI N210: Spartan&reg; 3A-DSP 3400
- SMINI E100: Spartan&reg; 3A-DSP 1800
- SMINI E110: Spartan&reg; 3A-DSP 3400

## Build Instructions

- Download and install [Xilinx ISE 12.2](http://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/design-tools/v12_2.html)

- To add xtclsh to the PATH and to setup up the Xilinx build environment run
  + `source <install_dir>/Xilinx/12.2/ISE_DS/settings64.sh` (64-bit platform)
  + `source <install_dir>/Xilinx/12.2/ISE_DS/settings32.sh` (32-bit platform)

- Navigate to `smini2/top/{project}` where project is:
  + N2x0: For SMINI N200 and SMINI N210
  + E1x0: For SMINI E100 and SMINI E110

- To build a binary configuration bitstream run `make <target>`
  where the target is specific to each product. To get a list of supported targets run
  `make help`.

- The build output will be specific to the product and will be located in the
  `smini2/top/{project}/build` directory. Run `make help` for more information.

### N2x0 Targets and Outputs

#### Supported Targets
- N200R3:  Builds the SMINI N200 Rev 3 design.
- N200R4:  Builds the SMINI N200 Rev 4 design.
- N210R3:  Builds the SMINI N210 Rev 3 design.
- N210R4:  Builds the SMINI N210 Rev 4 design.

#### Outputs
- `build-<target>/u2plus.bit` : Configuration bitstream with header
- `build-<target>/u2plus.bin` : Configuration bitstream without header
- `build-<target>/u2plus.syr` : Xilinx system report
- `build-<target>/u2plus.twr` : Xilinx timing report

### E1x0 Targets and Outputs

#### Supported Targets
- E100:  Builds the SMINI E100 design.
- E110:  Builds the SMINI E110 design.

#### Outputs
- `build-<target>/E1x0.bit` : Configuration bitstream with header
- `build-<target>/E1x0.bin` : Configuration bitstream without header
- `build-<target>/E1x0.syr` : Xilinx system report
- `build-<target>/E1x0.twr` : Xilinx timing report

