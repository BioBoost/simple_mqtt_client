#include <iostream>
#include <string>
#include <unistd.h>

#include "../include/simple_mqtt_client/simple_mqtt_client.h"
using namespace BiosSimpleMqttClient;

class SomeMessageHandler : public IMQTTMessageHandler {
  public:
    void handle_mqtt_message(MQTTMessage mqttMessage) override {
      std::cout << "Received message on topic '"
        << mqttMessage.get_topic() << "' with payload: "
        << mqttMessage.get_message() << std::endl;
    }
};

const std::string SERVER_ADDRESS("tcp://10.0.0.100:1883");
const std::string CLIENT_ID("ghj489543jghewr");
const std::string TOPIC("test/hello");

int main(int argc, char* argv[])
{
  std::cout << "Sending hello message via MQTT" << std::endl;

	SimpleMQTTClient simpleClient(SERVER_ADDRESS, CLIENT_ID);
  SomeMessageHandler messageHandler;
  simpleClient.subscribe(TOPIC, &messageHandler);

  // Send a message
  sleep(2);   // Wait a bit for connection
  MQTTMessage message(TOPIC, "Hello @ ALL");
  simpleClient.publish(message);

	// Just block till user tells us to quit.
  std::cout << "Press Q to quit" << std::endl;
	while (std::tolower(std::cin.get()) != 'q');

 	return 0;
}
