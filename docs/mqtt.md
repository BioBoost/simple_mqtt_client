# Mqtt

## Installing MQTT libraries

Install dependencies to allow for compilation of mqtt

```shell
sudo apt-get update
sudo apt-get install git build-essential gcc make cmake cmake-gui cmake-curses-gui libssl-dev
```

Next clone, compile and install paho C client library of MQTT

```shell
cd /usr/local/src
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
make
sudo make install
```

The Paho C client comprises four shared libraries:

 * libmqttv3a.so - asynchronous
 * libmqttv3as.so - asynchronous with SSL
 * libmqttv3c.so - "classic" / synchronous
 * libmqttv3cs.so - "classic" / synchronous with SSL

You can check if the libraries are correctly installed by using the following command:

```shell
ldconfig -p | grep mqtt
```

Should show:
```shell
libpaho-mqtt3cs.so.1 (libc6,hard-float) => /usr/local/lib/libpaho-mqtt3cs.so.1
libpaho-mqtt3cs.so (libc6,hard-float) => /usr/local/lib/libpaho-mqtt3cs.so
libpaho-mqtt3c.so.1 (libc6,hard-float) => /usr/local/lib/libpaho-mqtt3c.so.1
libpaho-mqtt3c.so (libc6,hard-float) => /usr/local/lib/libpaho-mqtt3c.so
libpaho-mqtt3as.so.1 (libc6,hard-float) => /usr/local/lib/libpaho-mqtt3as.so.1
libpaho-mqtt3as.so (libc6,hard-float) => /usr/local/lib/libpaho-mqtt3as.so
libpaho-mqtt3a.so.1 (libc6,hard-float) => /usr/local/lib/libpaho-mqtt3a.so.1
libpaho-mqtt3a.so (libc6,hard-float) => /usr/local/lib/libpaho-mqtt3a.so
```

or you could invoke the compiler. It should state undefined reference to main:
```shell
g++ -lpaho-mqtt3a
/usr/lib/gcc/arm-linux-gnueabihf/6/../../../arm-linux-gnueabihf/crt1.o: In function `_start':
(.text+0x34): undefined reference to 'main'
collect2: error: ld returned 1 exit status
```

This project currently makes use of the classic synchronous client.

Next we need to compile and install the C++ wrapper library for the MQTT libraries

```shell
cd /usr/local/src
git clone https://github.com/eclipse/paho.mqtt.cpp.git
cd paho.mqtt.cpp
make
```

Unfortunately this repo's Makefile does not contain an install target. So we will have to this
manually. The libraries need to be installed to `/usr/local/lib` and the header files should be placed in `/usr/local/include`

```shell
sudo su
cd /usr/local/src/paho.mqtt.cpp
install -m 644 lib/libpaho-mqttpp3.so.1.0.0 /usr/local/lib/
/sbin/ldconfig /usr/local/lib
cd /usr/local/lib && ln -s libpaho-mqttpp3.so.1 libpaho-mqttpp3.so
cp -r /usr/local/src/paho.mqtt.cpp/src/mqtt /usr/local/include
chmod 644 /usr/local/include/mqtt/*
ldconfig
```

`ls -al /usr/local/lib` should show:

```shell
lrwxrwxrwx  1 root staff     20 Oct 16 12:13 libpaho-mqttpp3.so -> libpaho-mqttpp3.so.1
lrwxrwxrwx  1 root staff     24 Oct 16 12:10 libpaho-mqttpp3.so.1 -> libpaho-mqttpp3.so.1.0.0
-rw-r--r--  1 root staff 308648 Oct 16 12:10 libpaho-mqttpp3.so.1.0.0
```

You can check if the libraries are correctly installed by using the following command:

```shell
ldconfig -p | grep mqtt
```

Should show:
```shell
libpaho-mqttpp3.so.1 (libc6,hard-float) => /usr/local/lib/libpaho-mqttpp3.so.1
libpaho-mqttpp3.so (libc6,hard-float) => /usr/local/lib/libpaho-mqttpp3.so
```

or you could invoke the compiler. It should state undefined reference to main:
```shell
g++ -lpaho-mqttpp3
/usr/lib/gcc/arm-linux-gnueabihf/6/../../../arm-linux-gnueabihf/crt1.o: In function `_start':
(.text+0x34): undefined reference to `main'
collect2: error: ld returned 1 exit status
```
