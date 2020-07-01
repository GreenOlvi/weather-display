#include <Arduino.h>

#include <GxEPD.h>
#include <GxGDEH0213B72/GxGDEH0213B72.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <Fonts/FreeMono12pt7b.h>

#include "WiFiModule.h"
#include "OpenWeatherClient.h"

#define CS_PIN SS
#define DC_PIN 17
#define RST_PIN 16
#define BUSY_PIN 4
#define LED_PIN 19

GxIO_Class io(SPI, CS_PIN, DC_PIN, RST_PIN);
GxEPD_Class display(io, RST_PIN, BUSY_PIN);

WiFiModule wifi;

const char weatherApiKey[] = OPEN_WEATHER_MAP_API_KEY;
OpenWeatherClient weather(CITY_ID, weatherApiKey);

void setup() {
    Serial.begin(115200);
    wifi.setup();

    display.init();
    display.eraseDisplay();
    display.setRotation(1);
    display.setFont(&FreeMono12pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.update();

    pinMode(LED_PIN, OUTPUT);
}

bool on = false;
bool connected = false;
bool hasTemp = false;

unsigned long nextDisplayUpdate = 0;

void printTemp() {
    digitalWrite(LED_PIN, HIGH);

    auto temp = weather.getCurrentTemp();
   
    display.setCursor(0, 40);
    display.printf("%.1f°C", temp);

    digitalWrite(LED_PIN, LOW);
}

void updateDisplay(const unsigned long t) {
    if (t < nextDisplayUpdate) return;

    if (connected && !hasTemp) {
        printTemp();
        hasTemp = true;
    }

    if (wifi.isConnected() && !connected) {
        display.setCursor(0, 16);
        display.print(WiFi.localIP().toString());
        connected = true;
    }

    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);

    nextDisplayUpdate = t + 1000;
}

void loop() {
    unsigned long t = millis();

    wifi.update(t);
    updateDisplay(t);
}