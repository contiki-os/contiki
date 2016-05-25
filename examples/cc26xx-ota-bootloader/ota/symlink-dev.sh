#!/bin/sh
# Symlink OTA resources into both application folders
# Useful for debugging

here="$(dirname $(pwd))"

ln -s $here/ota/ota.* $here/bootloader
ln -s $here/ota/ota.* $here/ota-image-example
