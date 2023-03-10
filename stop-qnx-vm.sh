#!/usr/bin/env bash

set -e

cd qnx-vm/
sudo kill "$(sudo cat output/qemu.pid)"
