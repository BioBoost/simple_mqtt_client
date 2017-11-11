# Simple MQTT Client

The PAHO MQTT client libraries are a bit complex to use when needing a simple MQTT client process.

For this reason this library wraps all the complex stuff in a simple class and make it a walk in the part to implement an MQTT client that can subscribe and publish.

Only limitation is that this client can subscribe to only a single topic.

## Example

Check out the examples subdir.

Making the examples:

```shell
make examples
```

You can check your dynamic loaded libraries for the executable:

```shell
ldd <executable>
.................
```

## Dependencies

* [Install PAHO MQTT Libraries](docs/mqtt.md)
* [BiosLogger](https://github.com/BioBoost/bios_logger)

## Compilation and installation

Just do the make

```shell
sudo su
cd /usr/local/src
git clone https://github.com/BioBoost/simple_mqtt_client.git
cd simple_mqtt_client
make
sudo make install
```

## More info on Compilation

Checkout http://www.yolinux.com/TUTORIALS/LibraryArchives-StaticAndDynamic.html
