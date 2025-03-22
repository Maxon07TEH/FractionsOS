#!/bin/bash

mkdir -p isodir/boot/grub
cp FractionsOS.x64.bin isodir/boot/
cp boot/grub/grub.cfg isodir/boot/grub/
grub-mkrescue -o FractionsOS.iso isodir
