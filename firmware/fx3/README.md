INSTRUCTIONS
================================

# Building the B2xx FX3 Firmware

The SMINI B200 and B210 each use the Cypress FX3 USB3 PHY for USB3 connectivity.
This device has an ARM core on it, which is programmed in C. This README will
show you how to build our firmware source

**A brief "Theory of Operations":**
The host sends commands to the FX3, our USB3 PHY, which has an on-board ARM
which runs the FX3 firmware code (hex file). That code is responsible for
managing the transport from the host to the FPGA by configuring IO and DMA.

## Setting up the Cypress SDK

In order to compile the SMINI B200 and B210 firmware, you will need the FX3 SDK
distributed by the FX3 manufacturer, Cypress Semiconductor. You can download the
[FX3 SDK from here](http://www.cypress.com/documentation/software-and-drivers/ez-usb-fx3-sdk-archives)
*Note*: You *must* use SDK version 1.2.3!

Once you have downloaded it, extract the ARM cross-compiler from the tarball
`ARM_GCC.tar.gz` and put it somewhere useful. The highest level directory you
need is `arm-2013.03/`.

Now that you have extracted the cross compilation toolchain, you need to set up
some environment variables to tell the B2xx `makefile` where to look for the
tools. These variables are:

```
    $ export ARMGCC_INSTALL_PATH=<your path>/arm-2013.03
    $ export ARMGCC_VERSION=4.5.2
```

Now, you'll need to set-up the Cypress SDK, as well. In the SDK, navigate to
the `firmware` directory, and copy the following sub-directories into
`shd.git/firmware/fx3`: `common/`, `lpp_source/`, `u3p_firmware/`.

Your directory structure should now look like:

```
shd.git/
       |
       --firmware/
                 |
                 --fx3/
                      |
                      --b200/               # From SHD
                      --common/             # From Cypress SDK
                      --gpif2_designer/     # From SHD
                      --lpp_source/         # From Cypress SDK
                      --u3p_firmware/       # From Cypress SDK
                      --README.md           # From SHD
```


## Applying the Patch to the Toolchain

Now, you'll need to apply a patch to a couple of files in the Cypress SDK. Head
into the `common/` directory you just copied from the Cypress SDK, and apply the
patch `b200/fx3_mem_map.patch`.

```
    # cd shd.git/firmware/fx3/common/
    $ patch -p2 < ../b200/fx3_mem_map.patch
```

If you don't see any errors print on the screen, then the patch was successful.

## Building the Firmware

Now, you should be able to head into the `b200/` directory and simply build the
firmware:

```
    $ cd shd.git/firmware/fx3/b200
    $ make
```

It will generate a `smini_b200_fw.hex` file, which you can then give to SHD to
program your SMINI B200 or SMINI B210.

