#include "../../include/simple_mqtt_client/simple_mqtt_client.h"
#include <bios_logger/bios_logger.h>

namespace BiosSimpleMqttClient {

  SimpleMQTTClient::SimpleMQTTClient(std::string brokerAddress, std::string clientId) {
    isConnected = false;
    numberOfConnectionRetries = 0;

    connectionOptions.set_keep_alive_interval(20);
    connectionOptions.set_clean_session(true);

    client = new mqtt::async_client(brokerAddress, clientId);
    client->set_callback(*this);

    subscriptionTopic = "";
    messageHandler = nullptr;
    connect();
  }

  SimpleMQTTClient::~SimpleMQTTClient(void) {
    disconnect();
    delete client;
  }

  void SimpleMQTTClient::subscribe(std::string topic, IMQTTMessageHandler * messageHandler) {
    this->subscriptionTopic = topic;
    this->messageHandler = messageHandler;

    if (isConnected) {
      BiosLogger::DoLog.info("Subscribing to topic '" + subscriptionTopic + "' using QoS" + std::to_string(QOS));
      client->subscribe(subscriptionTopic, QOS, (void*)(&subscribeContext), *this);
    } else {
      BiosLogger::DoLog.warning("Cannot subscribe to " + subscriptionTopic + " - client not connected to broker. Will try again on connected");
    }
  }

  void SimpleMQTTClient::publish(MQTTMessage message) {
    if (!isConnected) {
      BiosLogger::DoLog.warning("Cannot publish - not connected to broker");
      return;
    }

    mqtt::message_ptr pubmsg = mqtt::make_message(message.get_topic(), message.get_message());
    pubmsg->set_qos(QOS);

    try {
      mqtt::delivery_token_ptr pubtok = client->publish(pubmsg, (void*)(&publishContext), *this);
    	if (!pubtok->wait_for(std::chrono::seconds(TIMEOUT_SECONDS))) {
        BiosLogger::DoLog.warning("Publish not completed within timeout");
      }
  	}
  	catch (const mqtt::exception& exc) {
      BiosLogger::DoLog.error("Failed to publish mqtt message " + std::string(exc.what()));
  	}
  }

  void SimpleMQTTClient::connect(void) {
  	try {
      BiosLogger::DoLog.info("Trying to connect to MQTT broker");
  		client->connect(connectionOptions, (void*)(&connectionContext), *this);
  	}
  	catch (const mqtt::exception& exc) {
      BiosLogger::DoLog.error("Connect failed with " + std::string(exc.what()));
  	}
  }

  void SimpleMQTTClient::reconnect() {
  	std::this_thread::sleep_for(std::chrono::milliseconds(2500));
    BiosLogger::DoLog.info("Reconnecting to MQTT broker");
  	connect();
  }

  void SimpleMQTTClient::disconnect(void) {
  	// Double check that there are no pending tokens
  	auto toks = client->get_pending_delivery_tokens();
  	if (!toks.empty()) {
      BiosLogger::DoLog.warning("There are pending delivery tokens");
    }

  	try {
      BiosLogger::DoLog.info("Disconnecting from the MQTT broker ...");
  		client->disconnect()->wait();
      BiosLogger::DoLog.info("Disconnected from the MQTT broker");
      isConnected = false;
  	}
  	catch (const mqtt::exception& exc) {
      BiosLogger::DoLog.error("Disconnect failed with " + std::string(exc.what()));
  	}
  }

  void SimpleMQTTClient::on_failure(const mqtt::token& tok) {
    if (tok.get_user_context() == &connectionContext) {
      BiosLogger::DoLog.warning("Connection attempt to MQTT broker failed");
      isConnected = false;
    	if (++numberOfConnectionRetries > N_RETRY_ATTEMPTS) {
    		return;
      }
    	reconnect();
    } else if (tok.get_user_context() == &subscribeContext) {
      BiosLogger::DoLog.warning("Subscription failed for topic " + subscriptionTopic);
    } else if (tok.get_user_context() == &publishContext) {
  		auto top = tok.get_topics();
  		if (top && !top->empty()) {
        BiosLogger::DoLog.warning("Publish failed for topic " + (*top)[0]);
      } else {
        BiosLogger::DoLog.warning("Publish failed");
      }
    }
  }

  void SimpleMQTTClient::on_success(const mqtt::token& tok) {
    // We can't use connectionContext here. For some reason this callback
    // is activated twice for a single connection that is made. Luckely there is
    // the connected() callback that works just fine.
    if (tok.get_user_context() == &subscribeContext) {
      BiosLogger::DoLog.info("Subscription success for topic " + subscriptionTopic);
    } else if (tok.get_user_context() == &publishContext) {
  		auto top = tok.get_topics();
  		if (top && !top->empty()) {
        BiosLogger::DoLog.info("Publish successfull for topic " + (*top)[0]);
      } else {
        BiosLogger::DoLog.info("Publish successfull");
      }
    }
  }

  void SimpleMQTTClient::connected(const std::string& cause) {
    BiosLogger::DoLog.info("Connection successfully made to MQTT broker");
    isConnected = true;
    if (messageHandler) {
      subscribe(subscriptionTopic, messageHandler);
    }
  }

  void SimpleMQTTClient::connection_lost(const std::string& cause) {
    BiosLogger::DoLog.warning("Connection to MQTT broker lost");
    isConnected = false;
    if (!cause.empty()) {
      BiosLogger::DoLog.warning("\tcause: " + cause);
    }

    BiosLogger::DoLog.info("Trying to reconnect to MQTT broker ...");
    numberOfConnectionRetries = 0;
    reconnect();
  }

  void SimpleMQTTClient::message_arrived(mqtt::const_message_ptr msg) {
    MQTTMessage message(msg->get_topic(), msg->to_string());
    messageHandler->handle_mqtt_message(message);
  }

  void SimpleMQTTClient::delivery_complete(mqtt::delivery_token_ptr token) { }

};
