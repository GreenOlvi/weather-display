#ifndef Mqtt_h
#define Mqtt_h

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESPmDNS.h>

#include "Module.h"

#define RECONNECT_DELAY 10000

class MqttClient : public Module {
    public:
        MqttClient(const char *clientId, const char *hostname, unsigned short port = 1883);
        MqttClient(const char *clientId, const IPAddress ip, unsigned short port = 1883);
        bool isConnected(void);
        void setup(void) override;
        void update(const unsigned long) override;
        bool publish(const char* topic, const char *payload);
        bool publish(const char* topic, float value, const char *format = "%.1f");
    private:
        const char* _clientId;
        const char* _hostname;
        IPAddress _serverIp;
        const unsigned short _port;
        WiFiClient _wifiClient;
        PubSubClient _client;
        unsigned long _lastUpdate = 0;
        void connect(void);
        void callback(char* topic, byte* payload, unsigned int length);
};

#endif