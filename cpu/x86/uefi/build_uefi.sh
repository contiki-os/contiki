#!/bin/bash

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# This script will always run on its own basepath, no matter where you call it from.
pushd ${SCRIPT_DIR}

if [ "$(uname -m)" = "x86_64" ]; then
    export ARCH=X64
fi

# Download the UEFI tool and library sources:
if [ -e edk2 ]; then
    make -C edk2/BaseTools/Source/C/Common clean
    make -C edk2/BaseTools/Source/C/GenFw clean
else
    git clone --depth=1 https://github.com/tianocore/edk2 || exit
fi
# Build common sources required by the GenFw tool:
make -C edk2/BaseTools/Source/C/Common || exit
# Build the GenFw tool that is used to generate UEFI binaries:
make -C edk2/BaseTools/Source/C/GenFw || exit
# Create a makefile that indicates to the Contiki build system that UEFI support
# should be built:
echo "EN_UEFI = 1" > Makefile.uefi

popd
