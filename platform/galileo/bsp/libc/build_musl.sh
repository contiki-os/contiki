#!/bin/bash

set -e

JOBS=5
TARGET=i586-elf
PKG_NAME=musl
SRC_DIR=${PKG_NAME}
DIST_SITE=git://git.musl-libc.org
COMMIT_HASH=e738b8cbe64b6dd3ed9f47b6d4cd7eb2c422b38d

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# This script will always run on its own basepath, no matter where you call it from.
pushd ${SCRIPT_DIR}

prepare() {
    # If the source directory doesn't exist, download it.
    if [[ ! -d ./${SRC_DIR} ]]; then
        git clone ${DIST_SITE}/${PKG_NAME}
        pushd ${SRC_DIR}
        git checkout ${COMMIT_HASH}
        popd
    fi

    # Clean up the previous install dir, if any.
    if [[ -d ./${TARGET} ]]; then
        rm -rf ./${TARGET}
    fi

    cd ${SRC_DIR}
}


build() {
    export CC=clang
    export CFLAGS="-target i586-contiki-unknown -march=i586 -mtune=i586 -fno-omit-frame-pointer -Qunused-arguments -fsanitize=safe-stack"

    mkdir -p install
    ./configure --target=${TARGET} \
        --build=${TARGET} \
        --prefix=`pwd`/install \
        --disable-wrapper \
        --disable-shared

    make clean
    make -j${JOBS} install
    cd ..

    echo "BUILT_LIBC = musl" > Makefile.libc
}

setup() {
    mkdir ${TARGET}
    cp -r ./${SRC_DIR}/install/lib ./${SRC_DIR}/install/include ${TARGET}
}

cleanup() {
    rm -rf ./${SRC_DIR}*
}


# By default we always call prepare, build and setup.
prepare
build
setup

# But we only cleanup if -c is used.
case $1 in
    -c | --cleanup)
        cleanup
        shift
        ;;
    *)
        # unknown option
        ;;
esac

popd
