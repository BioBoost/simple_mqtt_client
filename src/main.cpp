#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <thread>
#include <chrono>
#include <unistd.h>
#include "mqtt/async_client.h"

class MQTTMessage {
  private:
    std::string topic;
    std::string message;

  public:
    MQTTMessage(std::string topic, std::string message) {
      this->topic = topic;
      this->message = message;
    }

    std::string get_topic(void) {
      return topic;
    }

    std::string get_message(void) {
      return message;
    }
};

class IMQTTMessageHandler {
  public:
    virtual void handle_mqtt_message(MQTTMessage mqttMessage) = 0;
};


// Simple MQTT client can only subscribe to a single topic and only register a single message handler.
// If you need multiple topics, you need to instantiate multiple clients
class SimpleMQTTClient : public virtual mqtt::callback, public virtual mqtt::iaction_listener {

  private:
    const int	QOS = 1;
    const int	N_RETRY_ATTEMPTS = 5;
    const int TIMEOUT_SECONDS = 10;

  private:
  	int numberOfConnectionRetries;
  	mqtt::connect_options connectionOptions;
    mqtt::async_client * client;

    // Message handling
    bool isConnected;
    std::string topic;
    IMQTTMessageHandler * messageHandler;

    // Context variables are used to differentiate callback invocations from
    // one another.
    int connectionContext;
    int subscribeContext;
    int publishContext;

  public:
    SimpleMQTTClient(std::string brokerAddress, std::string clientId);
    ~SimpleMQTTClient(void);

  public:
    void subscribe(std::string topic, IMQTTMessageHandler * messageHandler);
    void publish(MQTTMessage message);

  public:
  	// Connection callbacks
  	void on_failure(const mqtt::token& tok) override;
  	void on_success(const mqtt::token& tok) override;
    void connected(const std::string& cause) override;
  	void connection_lost(const std::string& cause) override;

  	// Callback for when a message arrives or is delivered
  	void message_arrived(mqtt::const_message_ptr msg) override;
  	void delivery_complete(mqtt::delivery_token_ptr token) override;

  private:
    void connect(void);
    void reconnect(void);
    void disconnect(void);
};


SimpleMQTTClient::SimpleMQTTClient(std::string brokerAddress, std::string clientId) {
  isConnected = false;
  numberOfConnectionRetries = 0;

  connectionOptions.set_keep_alive_interval(20);
  connectionOptions.set_clean_session(true);

  client = new mqtt::async_client(brokerAddress, clientId);
  client->set_callback(*this);

  topic = "";
  messageHandler = nullptr;
  connect();
}

SimpleMQTTClient::~SimpleMQTTClient(void) {
  disconnect();
  delete client;
}

void SimpleMQTTClient::subscribe(std::string topic, IMQTTMessageHandler * messageHandler) {
  this->topic = topic;
  this->messageHandler = messageHandler;

  if (isConnected) {
    std::cout << "Subscribing to topic '" << topic << "' using QoS" << QOS << std::endl;
    client->subscribe(topic, QOS, (void*)(&subscribeContext), *this);
  } else {
    std::cout << "Cannot subscribe to " << topic << " - client not connected to broker" << std::endl;
  }
}

void SimpleMQTTClient::publish(MQTTMessage message) {
  mqtt::message_ptr pubmsg = mqtt::make_message(message.get_topic(), message.get_message());
  pubmsg->set_qos(QOS);

  mqtt::delivery_token_ptr pubtok = client->publish(pubmsg, (void*)(&publishContext), *this);
	if (!pubtok->wait_for(std::chrono::seconds(TIMEOUT_SECONDS))) {
    std::cout << "Publish not completed within timeout" << std::endl;
  }
}

void SimpleMQTTClient::connect(void) {
	try {
    std::cout << "Trying to connect to MQTT broker" << std::endl;
		client->connect(connectionOptions, (void*)(&connectionContext), *this);
	}
	catch (const mqtt::exception& exc) {
		std::cout << "Connect failed with " << exc.what() << std::endl;
	}
}

void SimpleMQTTClient::reconnect() {
	std::this_thread::sleep_for(std::chrono::milliseconds(2500));
  std::cout << "Reconnecting to MQTT broker" << std::endl;
	connect();
}

void SimpleMQTTClient::disconnect(void) {
	// Double check that there are no pending tokens
	auto toks = client->get_pending_delivery_tokens();
	if (!toks.empty()) {
		std::cout << "Error: There are pending delivery tokens!" << std::endl;
  }

	try {
		std::cout << "Disconnecting from the MQTT broker ..." << std::endl;
		client->disconnect()->wait();
		std::cout << "Disconnected from the MQTT broker" << std::endl;
    isConnected = false;
	}
	catch (const mqtt::exception& exc) {
		std::cout << "Disconnect failed with " << exc.what() << std::endl;
	}
}

void SimpleMQTTClient::on_failure(const mqtt::token& tok) {
  if (tok.get_user_context() == &connectionContext) {
  	std::cout << "Connection attempt to MQTT broker failed" << std::endl;
    isConnected = false;
  	if (++numberOfConnectionRetries > N_RETRY_ATTEMPTS) {
  		exit(1);
    }
  	reconnect();
  } else if (tok.get_user_context() == &subscribeContext) {
    std::cout << "Subscription failed for topic " + topic << std::endl;
  } else if (tok.get_user_context() == &publishContext) {
		auto top = tok.get_topics();
		if (top && !top->empty()) {
      std::cout << "Publish failed for topic " + (*top)[0] << std::endl;
    } else {
      std::cout << "Publish failed" << std::endl;
    }
  }
}

void SimpleMQTTClient::on_success(const mqtt::token& tok) {
  // We can't use connectionContext here. For some reason this callback
  // is activated twice for a single connection that is made. Luckely there is
  // the connected() callback that works just fine.
  if (tok.get_user_context() == &subscribeContext) {
    std::cout << "Subscription success for topic " + topic << std::endl;
  } else if (tok.get_user_context() == &publishContext) {
		auto top = tok.get_topics();
		if (top && !top->empty()) {
      std::cout << "Publish successfull for topic " + (*top)[0] << std::endl;
    } else {
      std::cout << "Publish successfull" << std::endl;
    }
  }
}

void SimpleMQTTClient::connected(const std::string& cause) {
  std::cout << "Connection successfully made to MQTT broker" << std::endl;
  isConnected = true;
  if (messageHandler) {
    subscribe(topic, messageHandler);
  }
}

void SimpleMQTTClient::connection_lost(const std::string& cause) {
  std::cout << "Connection to MQTT broker lost" << std::endl;
  isConnected = false;
  if (!cause.empty()) {
    std::cout << "\tcause: " << cause << std::endl;
  }

  std::cout << "Reconnecting ..." << std::endl;
  numberOfConnectionRetries = 0;
  reconnect();
}

void SimpleMQTTClient::message_arrived(mqtt::const_message_ptr msg) {
  MQTTMessage message(msg->get_topic(), msg->to_string());
  messageHandler->handle_mqtt_message(message);
}

void SimpleMQTTClient::delivery_complete(mqtt::delivery_token_ptr token) {

}

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
  sleep(5);
  MQTTMessage message(TOPIC, "Hello @ ALL");
  simpleClient.publish(message);

	// Just block till user tells us to quit.
  std::cout << "Press Q to quit" << std::endl;
	while (std::tolower(std::cin.get()) != 'q');

 	return 0;
}
