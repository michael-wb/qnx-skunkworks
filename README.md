# qnx-skunkworks
Simple realm core app that runs on QNX

## Building the archive

Shell scripts have been created to help with building the archive.

### Default build for local host

To use the default build, run the following command from the top level directory:

`./cmake/cmake-build.sh -t Debug`

To just rebuild after making changes, run the following command from the top level directory:

`cmake --build debug`

### Build for QNX

The QNX build requires that the `QNX_BASE` environment variable is set to the location where
the QNX 7.1 Software Development Platform has been installed. For example:

`export QNX_BASE=$HOME/qnx710`

To build for QNX, run the following command from the top level directory:

`./cmake/cmake-build-qnx.sh -t Debug`

Subsequent builds using `cmake --build debug` require the QNX environment to be loaded in the
current shell. This can be done by sourcing the _cmake/qnx-env.sh_ script. For example, run
the following command from the top level directory:

`source cmake/qnx-env.sh`

This will export the necessary QNX variables and print the current `QNX_HOST` and `QNX_TARGET`
values.

## Running the executable on QNX

To run a the QNX executable, QNX must be running on a target or a VM. The scripts provided in
this repository will create and run a QNX VM using QEMU.

### Building the QNX VM

The repository doesn't contain the QNX VM binary, so it must be built before use. Run the
following command from the top level directory to build the VM. Once the build is complete,
the binary image files will be located in the _qnx-vm/_ directory.

`./build-qnx-vm.sh`

NOTES:

* The `QNX_BASE` environment variable must be set to the location where the QNX 7.1 Software
  Development Platform has been installed.
* The QNX VM will be built using the SSH keys found in the _qnx-vm/keys_ directory. Use the
  _qnx-vm/keys/qnx-target_ key as the identity file to connect as `root` via SSH

### Running the QNX VM

To start the QNX VM, run the following command. The QNX VM will start and the console will be
taken over by QEMU for stdin/stdout for the QNX VM. The QEMU process must be killed manually
or with the _stop-qnx-vm.sh_ script to return to the console prompt.

`./start-qnx-vm.sh`

NOTE: `sudo` is required to run the VM due to the vmnet network, so you may need to enter
your password.

### Stopping the QNX VM

To stop the QNX VM, run the following command in a separate console terminal::

`./stop-qnx-vm.sh`

NOTE: `sudo` is required to stop the VM, since it was started using `sudo`. You may need
to enter your password.

### Connecting to the QNX VM

Either the console terminal where QEMU is running can be used to interact with the QNX VM,
or SSH can be used to connect and transfer files. The identity file for the `root` user
can be found in the _qnx-vm/keys/_ directory. The default IP address will likely be
`10.0.2.2`, although this could change depending on the DHCP caching. To find the IP address
of the QEMU VM, run the following command:

```lang=sh
$ arp -an | grep -iE "52:54:0"
? (10.0.2.2) at 52:54:0:12:34:56 on bridge100 ifscope [bridge]
```

The IP address is located in parentheses in the output. To SSH to the QNX VM, run the
following command:

`ssh -i qnx-vm/keys/qnx-target root@10.0.2.2`

NOTE: SSH may complain about the permissions of the identity file. If so, then run the
following command to make the private key file read only:

`chmod 400 qnx-vm/keys/qnx-target`
