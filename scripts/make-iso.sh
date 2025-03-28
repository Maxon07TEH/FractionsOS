#!/bin/bash

mkdir -p isodir/boot/grub
cp build/bin/FractionsOS.x64.bin isodir/boot/
cp src/boot/grub/grub.cfg isodir/boot/grub/
grub-mkrescue -o build/iso/FractionsOS.iso isodir
