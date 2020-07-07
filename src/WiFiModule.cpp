#include "WiFiModule.h"
#include "secrets.h"

WiFiModule::WiFiModule(const char* ssid, const char* password) : _ssid(ssid), _password(password) {
}

void WiFiModule::setup() {
    WiFi.setHostname(HOSTNAME);
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);
}

bool WiFiModule::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiModule::update(const unsigned long t) {
}