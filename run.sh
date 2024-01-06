#!/bin/sh

set -e

export DESTDIR=$(pwd)

meson compile -C build
qemu-system-i386 -kernel build/kernel/los.bin -hda test.iso
