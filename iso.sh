#!/bin/sh

set -e

export DESTDIR=$(pwd)/isodir

meson compile -C build
meson install -C build
grub-mkrescue -o los.iso $DESTDIR
