set(CMAKE_SYSTEM_NAME QNX)
set(CMAKE_SYSTEM_VERSION 7.1)
set(CMAKE_CROSSCOMPILING TRUE)
set(QNX TRUE)

set(CMAKE_SYSTEM_NAME QNX)

set(arch x86_64)
set(CMAKE_SYSTEM_PROCESSOR ${arch} CACHE STRING "Target architecture")

set(CMAKE_C_COMPILER qcc)
set(CMAKE_C_COMPILER_TARGET gcc_nto${arch})
set(CMAKE_CXX_COMPILER QCC)
set(CMAKE_CXX_COMPILER_TARGET gcc_nto${arch})

set(CMAKE_SYSROOT $ENV{QNX_TARGET})

# Search for programs in the build host directories.
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Search for libraries and headers in the target directories.
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_C_FLAGS_INIT -D_QNX_SOURCE)
set(CMAKE_CXX_FLAGS_INIT -D_QNX_SOURCE)
