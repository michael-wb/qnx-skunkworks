#!/usr/bin/env bash

set -e

cd qnx-vm
#sudo qemu-system-x86_64 -smp 2 -m 4G -drive file=output/disk-qemu.vmdk,if=ide,id=drv0 -pidfile output/qemu.pid \
#  -nographic -kernel output/ifs.bin -serial mon:stdio -object rng-random,filename=/dev/urandom,id=rng0 \
#  -device virtio-rng-pci,rng=rng0 -nic vmnet-host,isolated=off,start-address=10.0.2.1,end-address=10.0.2.31,subnet-mask=255.255.255.0

sudo qemu-system-x86_64 -smp 2 -m 4G -drive file=output/disk-qemu.vmdk,if=ide,id=drv0 -pidfile output/qemu.pid \
  -nographic -kernel output/ifs.bin -serial mon:stdio -object rng-random,filename=/dev/urandom,id=rng0 \
  -device virtio-rng-pci,rng=rng0 -nic vmnet-bridged,ifname=en4
