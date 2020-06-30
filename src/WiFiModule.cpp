#include "WiFiModule.h"
#include "secrets.h"

void WiFiModule::setup() {
    char* ssid = STASSID;
    char* password = STAPSK;

    WiFi.setHostname(HOSTNAME);
    WiFi.mode(WIFI_STA);
    WiFi.begin(STASSID, STAPSK);
}

bool WiFiModule::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiModule::update(const unsigned long t) {
}