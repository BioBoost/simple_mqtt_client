# MQTT

MQTT is an open OASIS and ISO standard lightweight, publish-subscribe network protocol that transports messages between devices. The protocol usually runs over TCP/IP; however, any network protocol that provides ordered, lossless, bi-directional connections can support MQTT.

This chapter will explain how to compile and use the Paho MQTT libraries.

## Libraries

To enable the powerfull yet light-weight messaging protocol we will require the Eclipse Paho MQTT C++ Client Library which can be found at [https://github.com/eclipse/paho.mqtt.cpp](https://github.com/eclipse/paho.mqtt.cpp).

However this library requires the Eclipse Paho C Client Library for the MQTT Protocol which in term can be found at [https://github.com/eclipse/paho.mqtt.c](https://github.com/eclipse/paho.mqtt.c).

Both libraries will need to be compiled before we can make use of them.

## Native Windows

At the moment it is not possible to compile the library for Windows using a normal GCC compiler.

## Linux

Before starting make sure your system is up-to-date:

```bash
sudo apt update
sudo apt upgrade
```

### Compiling the Eclipse Paho C Client Library for the MQTT Protocol library

Start by installing the dependencies to allow for compilation of the library.

```bash
sudo apt update
sudo apt install git build-essential gcc make cmake cmake-gui cmake-curses-gui libssl-dev
```

Next clone the GitHub repository. It's best to use a release version of the source code instead of the current master branch. For this we ill checkout the `v1.3.6` release and build this.

```bash
mkdir paho-mqtt && cd paho-mqtt
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
git checkout v1.3.6
```

This library can be compiled on many systems and has a lot of configuration options available for building, but since we are on a standard distro and using the default built tools all we need to do is execute the `make` command.

```bash
make
```

You may receive some warnings but there should be no compile errors.

If all went well it's time to "install" the libraries so they become available system wide when compiling applications that use them or when running an application that needs them.

```bash
sudo make install
sudo ldconfig
```

If all went well you should get no errors.

The Paho C client comprises four shared libraries:

* libmqttv3a.so - asynchronous
* libmqttv3as.so - asynchronous with SSL
* libmqttv3c.so - "classic" / synchronous
* libmqttv3cs.so - "classic" / synchronous with SSL

You can check if the libraries are correctly installed by using the following command:

```bash
ldconfig -p | grep mqtt
```

Expected output

```text
libpaho-mqtt3cs.so.1 (libc6,x86-64) => /usr/local/lib/libpaho-mqtt3cs.so.1
libpaho-mqtt3cs.so (libc6,x86-64) => /usr/local/lib/libpaho-mqtt3cs.so
libpaho-mqtt3c.so.1 (libc6,x86-64) => /usr/local/lib/libpaho-mqtt3c.so.1
libpaho-mqtt3c.so (libc6,x86-64) => /usr/local/lib/libpaho-mqtt3c.so
libpaho-mqtt3as.so.1 (libc6,x86-64) => /usr/local/lib/libpaho-mqtt3as.so.1
libpaho-mqtt3as.so (libc6,x86-64) => /usr/local/lib/libpaho-mqtt3as.so
libpaho-mqtt3a.so.1 (libc6,x86-64) => /usr/local/lib/libpaho-mqtt3a.so.1
libpaho-mqtt3a.so (libc6,x86-64) => /usr/local/lib/libpaho-mqtt3a.so
```

### Compiling the Eclipse Paho MQTT C++ Client Library

This code builds a library which enables C++11 applications to connect to an MQTT broker, publish messages to the broker, and to subscribe to topics and receive published messages.

Start by installing the dependencies to allow for compilation of the library.

```bash
sudo apt update
sudo apt install git build-essential gcc make cmake cmake-gui cmake-curses-gui libssl-dev
```

Next clone the GitHub repository. Normally it would be best to checkout a stable version but apparently there is a bug in the `v1.1` that makes the library incompatible with the later releases of the C library. At the moment of this writing, the commit `076fc9977d07c25fa9fef1e19026d8be069c8e94` has been tested and works perfectly fine.

```bash
cd ~/paho-mqtt
git clone https://github.com/eclipse/paho.mqtt.cpp.git
cd paho.mqtt.cpp
git checkout 076fc9977d07c25fa9fef1e19026d8be069c8e94
```

For the C++ library GNU Make and autotools were deprecated and removed in the v1.1 release. CMake is a cross-platform build system suitable for Unix and non-Unix platforms such as Microsoft Windows. It is now the only supported build system. On *nix systems CMake creates Makefiles.

This means in contrary to the C-library, the build setup first needs to be configured.

```bash
cmake -Bbuild -H. -DPAHO_BUILD_DOCUMENTATION=FALSE -DPAHO_BUILD_SAMPLES=FALSE
```

Time to build and install the library.

```bash
sudo cmake --build build/ --target install
sudo ldconfig
```

You can check if the libraries are correctly installed by using the following command:

```bash
ldconfig -p | grep mqttpp
```

Expected output

```text
libpaho-mqttpp3.so.1 (libc6,x86-64) => /usr/local/lib/libpaho-mqttpp3.so.1
libpaho-mqttpp3.so (libc6,x86-64) => /usr/local/lib/libpaho-mqttpp3.so
</pre>
```
