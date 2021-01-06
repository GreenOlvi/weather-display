#include <Arduino.h>

#include <GxEPD.h>
#include <GxGDEH0213B72/GxGDEH0213B72.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <Fonts/FreeMono9pt7b.h>
#include <ESPmDNS.h>

#include "WiFiModule.h"
#include "OpenWeatherClient.h"
#include "Mqtt.h"

#define SERIAL_DEBUG

#define CS_PIN SS
#define DC_PIN 17
#define RST_PIN 16
#define BUSY_PIN 4
#define LED_PIN 19
#define BAT_TEST_PIN 35

GxIO_Class io(SPI, CS_PIN, DC_PIN, RST_PIN);
GxEPD_Class display(io, RST_PIN, BUSY_PIN);

const char wifiSsid[] = STASSID;
const char wifiPassword[] = STAPSK;
WiFiModule wifi(wifiSsid, wifiPassword);

const char weatherApiKey[] = OPEN_WEATHER_MAP_API_KEY;
OpenWeatherClient weather(LATITUDE, LONGITUDE, weatherApiKey);

MqttClient mqtt("weatherDisplay", MQTT_HOST, MQTT_PORT);

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
unsigned long TimeToSleep = 600; /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int unreportedQueryCount = 0;

void goToSleep() {
    #ifdef SERIAL_DEBUG
    Serial.printf("Going to sleep for %lu seconds\n", TimeToSleep);
    #endif
    esp_sleep_enable_timer_wakeup(TimeToSleep * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}

void setup() {
    pinMode(BAT_TEST_PIN, INPUT);
    bootCount++;

    #ifdef SERIAL_DEBUG
    Serial.begin(115200);
    Serial.println("Boot number: " + String(bootCount));
    #endif

    wifi.setup();
    if (!MDNS.begin("weatherDisplay")) {
        #ifdef SERIAL_DEBUG
        Serial.println("MDNS.begin failed");
        #endif
    }
    mqtt.setup();

    display.init();
    display.eraseDisplay();
    display.setRotation(1);
    display.setFont(&FreeMono9pt7b);
    display.setTextColor(GxEPD_BLACK);

    pinMode(LED_PIN, OUTPUT);
}

float battertyVoltage;

unsigned long nextDisplayUpdate = 0;
unsigned long gotoSleepAt = -1;

void printTemp() {
    auto current = weather.getCurrentData();
    auto tomorrow = weather.getTomorrowData();

    display.setCursor(0, 24);
    if (current.dt > 0) {
        display.printf("%.1f°C  ", current.temp);
        display.println(current.description);

        display.setCursor(0, 40);
        display.printf("%.1f°C  ", tomorrow.temp);
        display.println(tomorrow.description);

        int offset = weather.getTimezoneOffset();
        display.setCursor(0, 60);
        display.printf("Sunrise %02d:%02d", hour(current.sunrise + offset), minute(current.sunrise + offset));

        display.setCursor(0, 74);
        display.printf("Sunset  %02d:%02d", hour(current.sunset + offset), minute(current.sunset + offset));

        display.setCursor(0, 120);
        display.printf("Last update %02d:%02d", hour(current.dt + offset), minute(current.dt + offset));
    } else {
        display.printf("Failed to fetch data");
    }
}

void printBattery() {
    battertyVoltage = (float)(analogRead(BAT_TEST_PIN)) / 4095 * 2 * 3.3 * 1.1;
#ifdef SERIAL_DEBUG
    Serial.print("VBat = ");
    Serial.printf("%.2f", battertyVoltage);
    Serial.println("V");
#endif

    display.setCursor(195, 10);
    display.printf("%.2fV", battertyVoltage);
}

void clearDisplay() {
    display.fillRect(0, 0, GxEPD_HEIGHT, GxEPD_WIDTH, GxEPD_WHITE);
}

void updateDisplay(const unsigned long t) {
    if (t < nextDisplayUpdate) return;

    clearDisplay();

    display.setCursor(0, 10);
    display.print(WiFi.localIP().toString());

    printTemp();
    printBattery();

    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);

    nextDisplayUpdate = millis() + 5000;
}

unsigned long nextReport = 0;

void reportData(const unsigned long t) {
    if (t < nextReport || !WiFi.isConnected() || !mqtt.isConnected() || !weather.madeFetchAttempt()) return;

    if (weather.isUpToDate()) {
        auto current = weather.getCurrentData();
        mqtt.publish("env/weatherDisplay/temp_out", current.temp);
    } else {
        // retry in a minute
        TimeToSleep = 60;
    }

    float battertyVoltage = (float)(analogRead(BAT_TEST_PIN)) / 4095 * 2 * 3.3 * 1.1;

    char payload[10];
    sprintf(payload, "%.2f", battertyVoltage);
    mqtt.publish("tele/weatherDisplay/batteryVoltage", payload);

    int queries = unreportedQueryCount + weather.getApiQueryCount();
    if (mqtt.publish("tele/weatherDisplay/owApiQueries", String(queries).c_str())) {
        unreportedQueryCount = 0;
        weather.resetApiQueryCount();
    } else {
        unreportedQueryCount = queries;
    }

    nextReport = t + 1000 * 60; // 1 minute

    gotoSleepAt = millis() + 1000; // can go to sleep now
}

void loop() {
    wifi.update(millis());
    weather.update(millis());
    mqtt.update(millis());
    updateDisplay(millis());
    reportData(millis());

    if (millis() >= gotoSleepAt) {
        goToSleep();
    }
}