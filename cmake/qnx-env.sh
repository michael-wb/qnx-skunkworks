#!/usr/bin/env bash

if [[ -z "${QNX_BASE}" ]]; then
    echo "ERROR: QNX_BASE is not defined"
    exit 1
fi
export QNX_ARCH="x86_64"
HOST_OS=$(uname -s)
case "$HOST_OS" in
    Linux)
    QNX_HOST=$QNX_BASE/host/linux/${QNX_ARCH}
        ;;
    Darwin)
    QNX_HOST=$QNX_BASE/host/darwin/${QNX_ARCH}
        ;;
    *)
    QNX_HOST=$QNX_BASE/host/win64/${QNX_ARCH}
        ;;
esac
export QNX_HOST    
export QNX_TARGET=$QNX_BASE/target/qnx7
export QNX_CONFIGURATION_EXCLUSIVE=$HOME/.qnx
export QNX_CONFIGURATION=$QNX_CONFIGURATION_EXCLUSIVE
export PATH=$QNX_HOST/usr/bin:$QNX_CONFIGURATION/bin:$QNX_BASE/jre/bin:$QNX_BASE/host/common/bin:$PATH
echo "QNX_HOST=${QNX_HOST}"
echo "QNX_TARGET=${QNX_TARGET}"
