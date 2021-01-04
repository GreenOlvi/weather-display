#include "Mqtt.h"

MqttClient::MqttClient(const char *clientId, const char *hostname, unsigned short port)
    : _clientId(clientId), _hostname(hostname), _port(port), _wifiClient(),  _client(_wifiClient) {
}

MqttClient::MqttClient(const char *clientId, const IPAddress ip, unsigned short port)
    : _clientId(clientId), _serverIp(ip), _port(port), _wifiClient(),  _client(_wifiClient) {
}

void MqttClient::setup() {
    if (_serverIp) {
        _client.setServer(_serverIp, _port);
    }
}

bool MqttClient::isConnected() {
    return _client.connected();
}

void MqttClient::connect() {
    if (!_serverIp) {
        _serverIp = MDNS.queryHost(_hostname);
        _client.setServer(_serverIp, _port);
    }

    _client.connect(_clientId);
}

void MqttClient::update(unsigned long t) {
    if (t - _lastUpdate >= RECONNECT_DELAY) {
        if (WiFi.isConnected() && !_client.connected()) {
            connect();
        }
        _lastUpdate = t;
    }
    _client.loop();
}

bool MqttClient::publish(const char* topic, const char *payload) {
    if (!_client.connected())
        return false;

    return _client.publish(topic, payload);
}

bool MqttClient::publish(const char* topic, float value, const char *format) {
    if (_client.connected() && value != NAN) {
        char payload[10];
        sprintf(payload, format, value);
        return publish(topic, payload);
    }
    return false;
}

void MqttClient::callback(char *topic, byte *payload, unsigned int length) {
}