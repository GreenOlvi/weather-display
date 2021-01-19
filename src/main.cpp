#include <Arduino.h>

#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_7C.h>

#include <Fonts/FreeMono9pt7b.h>

#undef SERIAL_DEBUG

#include "secrets.h"
#include "WiFiModule.h"
#include "OpenWeatherClient.h"
#include "MqttModule.h"

#define CS_PIN SS
#define DC_PIN 17
#define RST_PIN 16
#define BUSY_PIN 4
#define LED_PIN 19
#define BAT_TEST_PIN 35

GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT> display(GxEPD2_213_B72(CS_PIN, DC_PIN, RST_PIN, BUSY_PIN));

WiFiModule wifi(HOSTNAME, STASSID, STAPSK);
MqttModule mqtt(&wifi, "weatherDisplay", MQTT_HOST, MQTT_PORT);

const char weatherApiKey[] = OPEN_WEATHER_MAP_API_KEY;
OpenWeatherClient weather(LATITUDE, LONGITUDE, weatherApiKey);

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
unsigned long TimeToSleep = 600; /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int unreportedQueryCount = 0;
RTC_DATA_ATTR int failedQueries = 0;

float battertyVoltage;

unsigned long nextDisplayUpdate = -1;
unsigned long gotoSleepAt = -1;

bool displayedResults = false;
bool reportedResults = false;

void goToSleep() {
    #ifdef SERIAL_DEBUG
    Serial.printf("Going to sleep for %lu seconds\n", TimeToSleep);
    Serial.printf("Millis = %lu", millis());
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
    wifi.connect();

    mqtt.setup();
    mqtt.connect();

    wifi.onConnect([](WiFiClass *wifi) { nextDisplayUpdate = millis() + 100; });

    display.init(115200);
    display.setRotation(1);
    display.setFont(&FreeMono9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setFullWindow();

    pinMode(LED_PIN, OUTPUT);
}

void printTemp() {
    auto current = weather.getCurrentData();
    auto tomorrow = weather.getTomorrowData();

    display.setCursor(0, 40);
    if (current.dt > 0) {
        display.printf("%.1f°C  ", current.temp);
        display.println(current.description);

        display.setCursor(0, 60);
        display.printf("%.1f°C  ", tomorrow.temp);
        display.println(tomorrow.description);

        int offset = weather.getTimezoneOffset();
        display.setCursor(0, 80);
        display.printf("Sunrise %02d:%02d", hour(current.sunrise + offset), minute(current.sunrise + offset));

        display.setCursor(0, 100);
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

    display.setCursor(195, 20);
    display.printf("%.2fV", battertyVoltage);
}

void updateDisplay(const unsigned long t) {
    if (t < nextDisplayUpdate || !weather.madeFetchAttempt()) return;

    auto ip = WiFi.localIP().toString().c_str();

    display.fillScreen(GxEPD_WHITE);
    display.setCursor(0, 20);
    display.print(ip);

    printTemp();
    printBattery();

    display.display(true);

    nextDisplayUpdate = 1000;
    displayedResults = true;
}

unsigned long nextReport = 0;

void reportData(const unsigned long t) {
    if (t < nextReport || !WiFi.isConnected() || !mqtt.isConnected() || !weather.madeFetchAttempt()) return;

    if (weather.isUpToDate()) {
        failedQueries = 0;
        auto current = weather.getCurrentData();
        char payload[6];
        sprintf(payload, "%.1f", current.temp);
        mqtt.publish("env/weatherDisplay/temp_out", payload);
    } else {
        // might as well go to sleep
        TimeToSleep = pow(2, failedQueries) * 60;
        failedQueries++;
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
    reportedResults = true;
}

void loop() {
    wifi.update(millis());
    weather.update(millis());
    mqtt.update(millis());
    updateDisplay(millis());
    reportData(millis());

    if (gotoSleepAt == -1 && displayedResults && reportedResults) {
        gotoSleepAt = millis() + 100;
    }

    if (millis() >= gotoSleepAt) {
        goToSleep();
    }
}