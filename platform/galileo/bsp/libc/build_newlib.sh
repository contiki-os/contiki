#!/bin/bash

JOBS=5
TARGET=i586-elf
VERSION=2.2.0-1
MD5=94114fdc1d8391cdbc2653d89249cccf
TARBALL=newlib-${VERSION}.tar.gz

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# This script will always run on its own basepath, no matter where you call it from.
pushd ${SCRIPT_DIR}

prepare() {
    # If the source tarball doesn't exist of its md5 checksum doesn't match, download it.
    if [ ! -e ./${TARBALL} ] || [ "$(md5sum ./${TARBALL} | cut -d' ' -f1)" != $MD5 ]; then
        wget -c ftp://sources.redhat.com/pub/newlib/${TARBALL}
    fi

    # Clean up the previous source dir, if any.
    if [[ -d ./newlib-${VERSION} ]]; then
        rm -rf ./newlib-${VERSION}
    fi

    # Clean up the previous install dir, if any.
    if [[ -d ./${VERSION} ]]; then
        rm -rf ./${VERSION}
    fi

    tar xf ${TARBALL}
    cd newlib-${VERSION}

    for i in  `ls ../patches/`; do patch -p0 < ../patches/${i}; done
}


build() {
    export AR_FOR_TARGET=ar
    export AS_FOR_TARGET=as
    export CC_FOR_TARGET=cc
    export GCC_FOR_TARGET=gcc
    export CXX_FOR_TARGET=c++
    export RAW_CXX_FOR_TARGET=c++
    export GCJ_FOR_TARGET=gcj
    export GFORTRAN_FOR_TARGET=gfortran
    export GOC_FOR_TARGET=gccgo
    export DLLTOOL_FOR_TARGET=dlltool
    export LD_FOR_TARGET=ld
    export LIPO_FOR_TARGET=lipo
    export NM_FOR_TARGET=nm
    export OBJDUMP_FOR_TARGET=objdump
    export RANLIB_FOR_TARGET=ranlib
    export READELF_FOR_TARGET=readelf
    export STRIP_FOR_TARGET=strip
    export WINDRES_FOR_TARGET=windres
    export WINDMC_FOR_TARGET=windmc
    export COMPILER_AS_FOR_TARGET=as
    export COMPILER_LD_FOR_TARGET=ld
    export COMPILER_NM_FOR_TARGET=nm
    export CFLAGS_FOR_TARGET="-Os -m32 -march=i586 -mtune=i586 -fno-stack-protector -DPREFER_SIZE_OVER_SPEED"
    export CXXFLAGS_FOR_TARGET="-Os -m32 -march=i586 -mtune=i586 -fno-stack-protector -DPREFER_SIZE_OVER_SPEED"

    mkdir -p install
    ./configure --target=${TARGET} \
        --prefix=`pwd`/install \
        --enable-newlib-nano-formatted-io \
        --enable-newlib-nano-malloc \
        --enable-multithread \
        --disable-newlib-fvwrite-in-streamio \
        --disable-newlib-fseek-optimization \
        --disable-newlib-wide-orient \
        --disable-newlib-unbuf-stream-opt \
        --disable-libstdcxx \
        --disable-multilib \
        --disable-newlib-mb \
        --disable-newlib-supplied-syscalls

    make -j${JOBS} all && make install
    cd ..
}

setup() {
    cp -r ./newlib-${VERSION}/install/${TARGET} .
}

cleanup() {
    rm -rf ./newlib-${VERSION}*
}


# By default we always call prepare, build and setup.
prepare && build && setup

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
