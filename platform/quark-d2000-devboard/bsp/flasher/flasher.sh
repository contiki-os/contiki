#!/bin/bash

# Copyright (c) 2016, Intel Corporation
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 3. Neither the name of the Intel Corporation nor the names of its
#    contributors may be used to endorse or promote products derived from this
#    software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL CORPORATION OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

# Help script to flash images on Quark D2000.

set -e

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
BSP_DIR=${SCRIPT_DIR}/..
OPENOCD_DIR=${BSP_DIR}/toolchain/iamcu_toolchain_Linux_issm_2016.0.019/tools/debugger/openocd

# This script will always run on its own basepath, no matter where you call it from.
pushd ${SCRIPT_DIR} &> /dev/null

function usage() {
    echo "Usage: flasher.sh OPTION [PATH_TO_IMAGE]"
    echo "Available options:"
    echo "  -h|--help       Display this help"
    echo "  -f|--flash      Flash Contiki image. Expects an image as argument (MUST BE ELF IMAGE)"
    echo "  -r|--rom        Flash QMSI ROM firmware. Expects an image as argument"
}

function flash() {
    ${OPENOCD_DIR}/bin/openocd -f ${OPENOCD_DIR}/scripts/board/quark_d2000_onboard.cfg \
                               -c "init" \
                               -c "reset halt" \
                               -c "$1" \
                               -c "reset run" \
                               -c "shutdown"
}

function flash_contiki() {
    if [ -z "$1" ]; then
        echo "Contiki image missing"
        popd &> /dev/null
        exit 1
    fi

    flash "load_image $1"
}

function flash_rom() {
    if [ -z "$1" ]; then
        echo "ROM firmware missing"
        popd &> /dev/null
        exit 1
    fi

    flash "load_image $1 0x00000000"
}

case "$1" in
    -f|--flash)
        flash_contiki $2
        echo "Contiki image flashed successfully"
        ;;
    -r|--rom)
        flash_rom $2
        echo "QMSI ROM firmware flashed successfully"
        ;;
    *)
        usage
        exit
        ;;
esac

popd &> /dev/null
exit 0
