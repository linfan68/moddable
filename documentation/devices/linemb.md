# Using the Moddable SDK with embedded Linux (Raspberry Pi Zero)
This is a demo document to demostration how to bring up an embedded Linux device with the Moddable SDK.

# Prepare
check the general get started guide [Moddable SDK - Getting Started](../Moddable SDK - Getting Started.md)
Notice this is only tested with Ubuntu 22.04 for the host machine.

Key steps:
```bash
sudo 
export MODDABLE=/workspaces/linfan/moddable
export PATH=$PATH:$MODDABLE/build/bin/lin/release
cd $MODDABLE/build/makefiles/lin
make
```

# Intall tool chain
There are 3 types of CPU supported by linemb (Linux Embedded), and use the following command to install the tool chain for each CPU type.

## ARM(32-bit)
```
sudo apt-get update
sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

```
## ARM(64-bit)
sudo apt-get update
sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

## amd64(x86_64, the host)
sudo apt-get install build-essential

# The sample project
this is sample project for ARM32 CPU:
```
cd $MODDABLE/examples/devices/linemb
mcconfig -m -p linemb
```

The generated executable is here:
`$MODDABLE/build/bin/linemb/release/hello-linemb/hello-linemb`

Find a way to copy this file to the target board (like scp):

```
scp $MODDABLE/build/bin/linemb/release/hello-linemb/hello-linemb root@172.32.0.93:/root/
```

should see output like this:
```
# ./hello-linemb
Hello, Linux Embedded!!!
immediate
oneshot
repeat 0
repeat 1
repeat 2
repeat 3
repeat 4


```