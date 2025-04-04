qemu-system-x86_64 -cdrom build/iso/FractionsOS.iso \
  -m 2G \
  -serial stdio \
  -d int,cpu_reset \
  -D qemu.log