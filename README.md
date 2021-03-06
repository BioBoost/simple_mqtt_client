# Simple MQTT Client

The PAHO MQTT client libraries are a bit complex to use when needing a simple MQTT client process.

For this reason this library wraps all the complex stuff in a simple class and makes it a walk in the park to implement an MQTT client that can subscribe and publish.

Only limitation is that this client can subscribe to only a single topic at the moment.

## Example

Check out the examples subdir.

Making the examples:

```bash
make examples
```

You can check your dynamic loaded libraries for the executable:

```bash
ldd <executable>
.................
```

## Dependencies

* [Install PAHO MQTT Libraries](docs/mqtt.md)

## Compiling and installing the library

This library can be compiled as a shared library.

Clone the repository and checkout the lasest version:

```bash
git clone https://github.com/BioBoost/simple_mqtt_client.git
cd simple_mqtt_client
git checkout v1.1
```

Next just do the make

```bash
make
sudo make install
```

## More info on Compilation

Checkout http://www.yolinux.com/TUTORIALS/LibraryArchives-StaticAndDynamic.html
