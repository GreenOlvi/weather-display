#include "WiFiModule.h"

WiFiModule::WiFiModule(const char *hostname, const char *ssid, const char *password)
    : _hostname(hostname), _ssid(ssid), _password(password) {
}

void WiFiModule::setup() {
    WiFi.mode(WIFI_OFF);
    WiFi.setHostname(_hostname);
    WiFi.persistent(false);

    onConnect([this](WiFiClass *wifi) { this->mdnsBegin(); });
}

void WiFiModule::connect() {
    _stayConnected = true;
    _retryCount = 0;
    begin();
    _nextRetry = millis() + 5000;
}

void WiFiModule::disconnect() {
    #ifdef SERIAL_DEBUG
    Serial.println("Disconnecting WiFi");
    #endif
    _stayConnected = false;
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    _nextRetry = -1;
}

bool WiFiModule::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

WiFiClient WiFiModule::client() {
    return WiFiClient();
}

void WiFiModule::onStart(WiFiModuleEvent moduleEvent) {
    WiFi.onEvent([moduleEvent](WiFiEvent_t event, WiFiEventInfo_t info) {
        moduleEvent(&WiFi);
    }, WiFiEvent_t::SYSTEM_EVENT_STA_START);
}

void WiFiModule::onStop(WiFiModuleEvent moduleEvent) {
    WiFi.onEvent([moduleEvent](WiFiEvent_t event, WiFiEventInfo_t info) {
        moduleEvent(&WiFi);
    }, WiFiEvent_t::SYSTEM_EVENT_STA_STOP);
}

void WiFiModule::onConnect(WiFiModuleEvent moduleEvent) {
    WiFi.onEvent([moduleEvent](WiFiEvent_t event, WiFiEventInfo_t info) {
        moduleEvent(&WiFi);
    }, WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);
}

void WiFiModule::onDisconnect(WiFiModuleEvent moduleEvent) {
    WiFi.onEvent([moduleEvent](WiFiEvent_t event, WiFiEventInfo_t info) {
        moduleEvent(&WiFi);
    }, WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);
}

void WiFiModule::update(const unsigned long t) {
    if (_stayConnected && !isConnected() && t > _nextRetry)
    {
        reconnect();
    }
}

void WiFiModule::reconnect() {
    wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED)
        return;

    _retryCount++;

    if (_retryCount == 2) {
        #ifdef SERIAL_DEBUG
        Serial.println("WiFi not connected, restarting");
        #endif
        WiFi.disconnect();
        delay(10);
        begin();
        _nextRetry = millis() + 25000;
    }

    if (_retryCount == 3) {
        #ifdef SERIAL_DEBUG
        Serial.println("WiFi still not connected, stopping");
        #endif
        disconnect();
    }
}

void WiFiModule::begin() {
    #ifdef SERIAL_DEBUG
    Serial.println("Connecting to WiFi");
    #endif
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);
}

bool WiFiModule::resolveHostname(const char *hostname, IPAddress &result) {
    #ifdef SERIAL_DEBUG
    Serial.printf("Query A: %s.local\n", hostname);
    #endif

    struct ip4_addr addr;
    addr.addr = 0;

    esp_err_t err = mdns_query_a(hostname, 2000,  &addr);
    if(err){
        if (err == ESP_ERR_NOT_FOUND)
        {
            #ifdef SERIAL_DEBUG
            Serial.println("Host was not found!");
            #endif
            return false;
        }
        #ifdef SERIAL_DEBUG
        Serial.printf("Query Failed: %d\n", err);
        #endif
        return false;
    }

    #ifdef SERIAL_DEBUG
    Serial.printf(IPSTR, IP2STR(&addr));
    Serial.println();
    #endif
    result = IPAddress(addr.addr);
    return true;
}

bool WiFiModule::mdnsBegin() {
    esp_err_t err = mdns_init();
    if (err) {
        #ifdef SERIAL_DEBUG
        Serial.printf("MDNS Init failed: %d\n", err);
        #endif
        return false;
    }

    err = mdns_hostname_set(_hostname);
    if (err) {
        #ifdef SERIAL_DEBUG
        Serial.printf("MDNS hostname set failed: %d\n", err);
        #endif
        return false;
    }

    err = mdns_instance_name_set("TTGO Weather Display");
    if (err) {
        #ifdef SERIAL_DEBUG
        Serial.printf("MDNS instance name set failed: %d\n", err);
        #endif
        return false;
    }

    #ifdef SERIAL_DEBUG
    Serial.println("MDNS started");
    #endif

    return true;
}