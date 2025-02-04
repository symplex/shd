#!/bin/bash
#

### Helper functions ##########################################
function help {
	cat <<EOHELP

Usage: impact_jtag_programmer.sh [--help|-h] [--impact-path=<PATH>] --fpga-path=<FPGA Image File>

-h             - Shows this.
--impact-path  - Path to the iMPACT binary (if not already in PATH).
                 Often something like /opt/Xilinx/14.7/ISE_DS/ISE/bin/lin64/impact
--fpga-path    - Path to the FPGA image.
--nipcie       - When this option is set, this script will fail if it was unable
                 to stop the NI RIO SMINI PCIe driver.

This script uses Xilinx iMPACT to reprogram the FPGA through the USB JTAG connector.
You can use this to unbrick SMINI X3x0 devices.

EOHELP
}

### Go, go, go! ###############################################
echo "======================================="
echo " Copyright 2014 Ettus Research LLC"
echo ""
echo " JTAG Programming Tool"
echo "======================================="
echo ""

IMPACTPATH="impact" # This'll work if impact is in $PATH
FPGAIMG=""
FORCENIPCIECHECK=0

# Go through cmd line options
for i in "$@"
do
case $i in
    -h|--help)
        help
        exit
        ;;
    --impact-path=*)
    IMPACTPATH="${i#*=}"
    ;;
    --fpga-path=*)
    FPGAIMG="${i#*=}"
    ;;
    --nipcie)
        FORCENIPCIECHECK=1
        ;;
    *)
        echo Unrecognized option: $i
        echo
        help
        exit
        break
        ;;
esac
done

# Test impact binary is available
IMPACTEXECUTABLE=`which $IMPACTPATH`
if [ ! $? -eq 0 ]; then
    echo "ERROR: Cannot find 'impact' executable. Make sure you have iMPACT installed"
    echo "and that it is in your PATH, or use the --impact-path option to provide the"
    echo "location of the 'impact' executable."
    exit 1
fi

# Test the FPGA image file is readable
if [ -z $FPGAIMG ]; then
    echo "ERROR: No FPGA image file provided."
    exit 1
fi
if [ ! -r $FPGAIMG ]; then
    echo "ERROR: Can't read the FPGA image file ($FPGAIMG)."
    exit 1
fi

# Test if we need to stop PCIe
NISMINIRIOPCIE=`which nisminirio_pcie`
HASNISMINIRIOPCIE=$?
if [ ! $HASNISMINIRIOPCIE -eq 0 -a $FORCENIPCIECHECK -eq 1 ]; then
    echo "ERROR: Cannot find the nisminirio_pcie executable. Make sure it is in your PATH."
    exit 1
fi

NISMINIRIOWASLOADED=0
# If we do, check if we can run nisminirio_pcie
if [ $HASNISMINIRIOPCIE -eq 0 ]; then
    NISMINIRIOSTATUSOUTPUT=`nisminirio_pcie status`
    NISMINIRIORETVAL=$?
    if [ ! $NISMINIRIORETVAL -eq 0 ]; then
        echo "ERROR: Can't run 'nisminirio_pcie status'. Maybe you forgot to use sudo?"
        exit $NISMINIRIORETVAL
    fi
    NISMINIRIOSTATUSOUTPUTGREP=`nisminirio_pcie status | grep smini`
    if [ ! -z "$NISMINIRIOSTATUSOUTPUTGREP" ]; then
        NISMINIRIOWASLOADED=1
    fi
fi
if [ $NISMINIRIOWASLOADED -eq 1 ]; then
    echo "==== Stopping nisurprio drivers... "
    nisminirio_pcie stop
    NISMINIRIORETVAL=$?
    if [ ! $NISMINIRIORETVAL -eq 0 ]; then
        echo "ERROR: Can't run 'nisminirio_pcie stop'. Maybe you forgot to use sudo?"
        exit $NISMINIRIORETVAL
    else
        echo "Done."
    fi
fi

# Create batch file
CMD_PATH=`mktemp XXXXXXXX.impact.cmd`
echo "==== Generating impact batch file ${CMD_PATH}..."
echo "setmode -bscan" > ${CMD_PATH}
echo "setcable -p auto" >> ${CMD_PATH}
echo "addDevice -p 1 -file ${FPGAIMG}" >> ${CMD_PATH}
echo "program -p 1" >> ${CMD_PATH}
echo "quit" >> ${CMD_PATH}

# Run impact
echo "==== Running impact -- loading ${FPGAIMG} into the FPGA..."
${IMPACTEXECUTABLE} -batch ${CMD_PATH}
RETVAL=$?
if [ ! $RETVAL -eq 0 ]; then
    echo "ERROR: Programming failed. Check output above for hints. Maybe you forgot to use sudo?"
else
    echo "==== Programming complete! =============="
fi

# Remove batch file
rm $CMD_PATH
if [ $NISMINIRIOWASLOADED -eq 1 ]; then
    echo "==== Re-starting nisurprio drivers... "
    nisminirio_pcie start
    NISMINIRIORETVAL=$?
    if [ ! $NISMINIRIORETVAL -eq 0 ]; then
        echo "WARNING: nissminirio drivers were running before, but could not be reloaded after flashing."
        echo "This is usually not a problem with the FPGA image burning."
    fi
fi

exit $RETVAL

# C'est tout
