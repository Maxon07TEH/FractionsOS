#!/bin/bash
qemu-system-x86_64 -cdrom FractionsOS.iso -m 512M -no-reboot -d int -D qemu.log
