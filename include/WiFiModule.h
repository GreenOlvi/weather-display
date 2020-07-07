#ifndef WiFiModule_h
#define WiFiModule_h

#include <Arduino.h>
#include <WiFi.h>
#include "Module.h"
#include "secrets.h"

class WiFiModule : public Module {
    public:
        WiFiModule(const char* ssid, const char* password);
        void setup(void) override;
        void update(const unsigned long t) override;
        bool isConnected();

    private:
        const char* _ssid;
        const char* _password;
};

#endif