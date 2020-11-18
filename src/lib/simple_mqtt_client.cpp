#include "../../include/simple_mqtt_client/simple_mqtt_client.h"
#include "../../include/simple_mqtt_client/null_buffer.h"

// Only known in this compile unit (cpp-file)
#define info (*logger) << "[INFO] "
#define warning (*logger) << "[WARNING] "
#define error (*logger) << "[ERROR] "

namespace BiosSimpleMqttClient {

  SimpleMQTTClient::SimpleMQTTClient(std::string brokerAddress, std::string clientId)
    : SimpleMQTTClient(brokerAddress, clientId, new std::ostream(new Logging::NullBuffer())) {

    usingInternalLogger = true;
  }

  SimpleMQTTClient::SimpleMQTTClient(std::string brokerAddress, std::string clientId, std::ostream * logger) {
    this->logger = logger;

    isConnected = false;
    numberOfConnectionRetries = 0;

    connectionOptions.set_keep_alive_interval(20);
    connectionOptions.set_clean_session(true);

    client = new mqtt::async_client(brokerAddress, clientId);
    client->set_callback(*this);

    subscriptionTopic = "";
    messageHandler = nullptr;
  }

  SimpleMQTTClient::~SimpleMQTTClient(void) {
    info << "Cleaning up ..." << std::endl;
    disconnect();
    delete client;
    if (usingInternalLogger) {
      delete logger->rdbuf();
      delete logger;
      logger = nullptr;
    }
  }

  void SimpleMQTTClient::subscribe(std::string topic, IMQTTMessageHandler * messageHandler) {
    this->subscriptionTopic = topic;
    this->messageHandler = messageHandler;

    if (isConnected) {
      info << "Subscribing to topic '" << subscriptionTopic << "' using QoS" << QOS << std::endl;
      client->subscribe(subscriptionTopic, QOS, (void*)(&subscribeContext), *this);
    } else {
      warning << "Cannot subscribe to " << subscriptionTopic << " - client not connected to broker. Will try again on connected" << std::endl;
    }
  }

  void SimpleMQTTClient::publish(MQTTMessage message) {
    if (!isConnected) {
      warning << "Cannot publish - not connected to broker" << std::endl;
      return;
    }

    mqtt::message_ptr pubmsg = mqtt::make_message(message.get_topic(), message.get_message());
    pubmsg->set_qos(QOS);

    try {
      mqtt::delivery_token_ptr pubtok = client->publish(pubmsg, (void*)(&publishContext), *this);
    	if (!pubtok->wait_for(std::chrono::seconds(TIMEOUT_SECONDS))) {
        warning << "Publish not completed within timeout" << std::endl;
      }
  	}
  	catch (const mqtt::exception& exc) {
      error << "Failed to publish mqtt message " << exc.what() << std::endl;
  	}
  }

  void SimpleMQTTClient::connect(void) {
  	try {
      info << "Trying to connect to MQTT broker ..." << std::endl;
  		client->connect(connectionOptions, (void*)(&connectionContext), *this);
  	}
  	catch (const mqtt::exception& exc) {
      error << "Connect failed with " << exc.what() << std::endl;
  	}
  }

  void SimpleMQTTClient::reconnect() {
  	std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    info << "Reconnecting to MQTT broker" << std::endl;
  	connect();
  }

  void SimpleMQTTClient::disconnect(void) {
  	// Double check that there are no pending tokens
  	auto toks = client->get_pending_delivery_tokens();
  	if (!toks.empty()) {
      warning << "There are pending delivery tokens" << std::endl;
    }

  	try {
      info << "Disconnecting from the MQTT broker ..." << std::endl;
  		client->disconnect()->wait();
      info << "Disconnected from the MQTT broker" << std::endl;
      isConnected = false;
  	}
  	catch (const mqtt::exception& exc) {
      error << "Disconnect failed with " << exc.what() << std::endl;
  	}
  }

  void SimpleMQTTClient::on_failure(const mqtt::token& tok) {
    if (tok.get_user_context() == &connectionContext) {
      warning << "Connection attempt to MQTT broker failed" << std::endl;
      isConnected = false;
    	if (++numberOfConnectionRetries > N_RETRY_ATTEMPTS) {
    		return;
      }
    	reconnect();
    } else if (tok.get_user_context() == &subscribeContext) {
      warning << "Subscription failed for topic " << subscriptionTopic << std::endl;
    } else if (tok.get_user_context() == &publishContext) {
  		auto top = tok.get_topics();
  		if (top && !top->empty()) {
        warning << "Publish failed for topic " << (*top)[0] << std::endl;
      } else {
        warning << "Publish failed" << std::endl;
      }
    }
  }

  void SimpleMQTTClient::on_success(const mqtt::token& tok) {
    // We can't use connectionContext here. For some reason this callback
    // is activated twice for a single connection that is made. Luckely there is
    // the connected() callback that works just fine.
    if (tok.get_user_context() == &subscribeContext) {
      info << "Subscription success for topic " << subscriptionTopic << std::endl;
    } else if (tok.get_user_context() == &publishContext) {
  		auto top = tok.get_topics();
  		if (top && !top->empty()) {
        info << "Publish successfull for topic " << (*top)[0] << std::endl;
      } else {
        info << "Publish successfull" << std::endl;
      }
    }
  }

  void SimpleMQTTClient::connected(const std::string& cause) {
    info << "Connection successfully made to MQTT broker" << std::endl;
    isConnected = true;
    if (messageHandler) {
      subscribe(subscriptionTopic, messageHandler);
    }
  }

  void SimpleMQTTClient::connection_lost(const std::string& cause) {
    warning << "Connection to MQTT broker lost" << std::endl;
    isConnected = false;
    if (!cause.empty()) {
      warning << "\tcause: " << cause << std::endl;
    }

    info << "Trying to reconnect to MQTT broker ..." << std::endl;
    numberOfConnectionRetries = 0;
    reconnect();
  }

  void SimpleMQTTClient::message_arrived(mqtt::const_message_ptr msg) {
    MQTTMessage message(msg->get_topic(), msg->to_string());
    messageHandler->handle_mqtt_message(message);
  }

  void SimpleMQTTClient::delivery_complete(mqtt::delivery_token_ptr token) { }

};
