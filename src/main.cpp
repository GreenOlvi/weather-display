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

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  600       /* Time ESP32 will go to sleep (in seconds) */
RTC_DATA_ATTR int bootCount = 0;

void goToSleep() {
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  Serial.println("Going to sleep now");
  Serial.flush();
  esp_deep_sleep_start();
}

void setup() {
    Serial.begin(115200);

    ++bootCount;
    Serial.println("Boot number: " + String(bootCount));

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
unsigned long gotoSleepAt = -1;

void printTemp() {
    float temp = weather.getCurrentTemp();
    String desc = weather.getCurrentDescription();
   
    display.setCursor(0, 40);
    display.printf("%.1f°C", temp);

    display.setCursor(0, 60);
    display.println(desc);
}

void updateDisplay(const unsigned long t) {
    if (t < nextDisplayUpdate) return;

    printTemp();

    if (wifi.isConnected() && !connected) {
        display.setCursor(0, 16);
        display.print(WiFi.localIP().toString());
        connected = true;

        gotoSleepAt = t + 5000;
    }

    display.setCursor(0, 120);
    display.printf("Boot count %d", bootCount);

    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);

    nextDisplayUpdate = t + 1000;
}

void loop() {
    unsigned long t = millis();

    wifi.update(t);
    weather.update(t);
    updateDisplay(t);

    if (t <= gotoSleepAt) {
        goToSleep();
    }
}