#!/usr/bin/env bash

set -e

source cmake/qnx-env.sh

cd qnx-vm
mkqnximage --build
