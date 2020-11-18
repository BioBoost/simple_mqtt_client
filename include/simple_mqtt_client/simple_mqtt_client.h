#pragma once

#include "i_mqtt_message_handler.h"
#include "mqtt_message.h"
#include <string>
#include <mqtt/async_client.h>

namespace BiosSimpleMqttClient {

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
      std::string subscriptionTopic;
      IMQTTMessageHandler * messageHandler;

      // Context variables are used to differentiate callback invocations from
      // one another.
      int connectionContext;
      int subscribeContext;
      int publishContext;

      // Logging
      std::ostream * logger;
      bool usingInternalLogger = false;

    public:
      SimpleMQTTClient(std::string brokerAddress, std::string clientId);
      SimpleMQTTClient(std::string brokerAddress, std::string clientId, std::ostream * logger);
      ~SimpleMQTTClient(void);

    public:
      void connect(void);
      void disconnect(void);

    public:
      void subscribe(std::string topic, IMQTTMessageHandler * messageHandler);
      void publish(MQTTMessage message);

    public:
      // General success and failure callbacks
    	void on_failure(const mqtt::token& tok) override;
    	void on_success(const mqtt::token& tok) override;

      	// Connection callbacks
      void connected(const std::string& cause) override;
    	void connection_lost(const std::string& cause) override;

    	// Callback for when a message arrives or is delivered
    	void message_arrived(mqtt::const_message_ptr msg) override;
    	void delivery_complete(mqtt::delivery_token_ptr token) override;

    private:
      void reconnect(void);
  };

};
