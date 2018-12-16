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

# This helper script downloads and buils QMSI.

set -e

MD5=da3fd230a6ad72917dec5b0524d978a2
VERSION=1.0.1
PKG_NAME=qmsi
SRC_DIR=${PKG_NAME}-${VERSION}
TARBALL=v${VERSION}.tar.gz
DIST_SITE=https://github.com/01org/qmsi/archive/
SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
BSP_DIR=${SCRIPT_DIR}/..

# This script will always run on its own basepath, no matter where you call it from.
pushd ${SCRIPT_DIR} > /dev/null

prepare() {
    # If the source tarball doesn't exist or its md5 checksum doesn't match, download it.
    if [ ! -e ./${TARBALL} ] || [ "$(md5sum ./${TARBALL} | cut -d' ' -f1)" != $MD5 ]; then
        wget -c ${DIST_SITE}/${TARBALL}
    fi
    if [ ! -e ./${TARBALL} ] || [ "$(md5sum ./${TARBALL} | cut -d' ' -f1)" != $MD5 ]; then
        echo "Error obtaining tarball."
	exit 1
    fi

    # Clean up the previous source dir, if any.
    if [[ -d ./${SRC_DIR} ]]; then
        rm -rf ./${SRC_DIR}
    fi

    tar xf ${TARBALL}
}

build() {
    export IAMCU_TOOLCHAIN_DIR=${BSP_DIR}/toolchain/iamcu_toolchain_Linux_issm_2016.0.019/tools/compiler/bin

    cd ${SRC_DIR}
    make SOC=quark_d2000 BUILD=release > /dev/null
    cd ..
}

prepare
build

echo "QMSI built successfully."

popd > /dev/null
