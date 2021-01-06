#include "MqttModule.h"

MqttModule::MqttModule(WiFiModule *wifiModule, const char *clientId, const char *hostname, uint16_t port) :
    _hostname(hostname), _port(port), _clientId(clientId) {
    _wifi = wifiModule;
    _wifiClient = _wifi->client();
    _client = PubSubClient(_wifiClient);
}

MqttModule::MqttModule(WiFiModule *wifiModule, const char *clientId, const IPAddress ip, uint16_t port) :
    _ip(ip), _hostname(nullptr), _port(port), _clientId(clientId)  {
    _wifi = wifiModule;
    _wifiClient = _wifi->client();
    _client = PubSubClient(_wifiClient);
    _client.setServer(_ip, _port);
}

void MqttModule::setup() {
    _client.setCallback([this] (char *topic, uint8_t *data, unsigned int length) {
        this->callback(topic, data, length);
    });
}

void MqttModule::update(const unsigned long t) {
    if (!_client.connected()) {
        if (_stayConnected && t > _nextRetry) {
            if (reconnect()) {
                #ifdef SERIAL_DEBUG
                Serial.println("MQTT connected");
                #endif
                _retryCount = 0;
            } else if (!_wifi->isConnected()) {
                #ifdef SERIAL_DEBUG
                Serial.println("MQTT waiting for WiFi to connect");
                #endif
                _nextRetry = -1;
                _wifi->onConnect([this](WiFiClass *wifi) { this->_nextRetry = millis(); });
            } else {
                unsigned int sec = _retryCount < 12 ? pow(2, _retryCount) : 4096;
                #ifdef SERIAL_DEBUG
                Serial.printf("MQTT connection failed. Waiting %d seconds\n", sec);
                #endif
                _retryCount++;
                _nextRetry = t + sec * 1000;
            }
        }
    } else {
        if (_stayConnected) {
            _client.loop();
        } else {
            _client.disconnect();
        }
    }
}

bool MqttModule::publish(const char *topic, const char *payload) {
    if (!_client.connected())
        return false;

    return _client.publish(topic, payload);
}

void MqttModule::connect() {
    _stayConnected = true;
    _nextRetry = 0;
}

void MqttModule::disconnect() {
    _stayConnected = false;
}


bool MqttModule::isConnected() {
    return _client.connected();
}

void MqttModule::subscribe(const char *topic, CallbackFn callback) {
    //TODO Subscribe to more than one topic
    _subscribedTopic = topic;
    _subscribedCallback = callback;
}

bool MqttModule::reconnect() {
    if (_wifi->isConnected()) {
        if (!_ip) {
            if (!resolveHostname()) {
                #ifdef SERIAL_DEBUG
                Serial.println("Mqtt broker hostname not resolved");
                #endif
                return false;
            }
            #ifdef SERIAL_DEBUG
            Serial.println("Mqtt broker hostname resolved");
            #endif
            _client.setServer(_ip, _port);
        }

        if (_client.connect(_clientId)) {
            resubscribe();
            return true;
        } else {
            return false;
        }
    }
    return false;
}

bool MqttModule::resubscribe() {
    if (!_client.connected())
        return false;

    if (!_subscribedTopic)
        return false;

    return _client.subscribe(_subscribedTopic);
}

void MqttModule::callback(char *topic, uint8_t *data, unsigned int length) {
    if (_subscribedCallback && topicMatches(_subscribedTopic, topic)) {
        _subscribedCallback(topic, data, length);
    }
}

bool MqttModule::topicMatches(const char *subscribed, const char *topic) {
    return strcmp(subscribed, topic) == 0;
}

bool MqttModule::resolveHostname() {
    IPAddress ip;
    if (_wifi->resolveHostname(_hostname, ip)) {
        _ip = ip;
        return true;
    }
    return false;
}
