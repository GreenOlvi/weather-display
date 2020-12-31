#include <Arduino.h>

#include <GxEPD.h>
#include <GxGDEH0213B72/GxGDEH0213B72.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <Fonts/FreeMono9pt7b.h>

#include <TimeLib.h>

#include "WiFiModule.h"
#include "OpenWeatherClient.h"

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

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  600      /* Time ESP32 will go to sleep (in seconds) */
RTC_DATA_ATTR int bootCount = 0;

void goToSleep() {
    Serial.println("Going to sleep");
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}

void setup() {
    ++bootCount;
    pinMode(BAT_TEST_PIN, INPUT);

    Serial.begin(115200);
    Serial.println("Boot number: " + String(bootCount));

    wifi.setup();

    display.init();
    display.eraseDisplay();
    display.setRotation(1);
    display.setFont(&FreeMono9pt7b);
    display.setTextColor(GxEPD_BLACK);
    // display.update();
    // display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);

    pinMode(LED_PIN, OUTPUT);
}

float battertyVoltage;

unsigned long nextDisplayUpdate = 0;
unsigned long gotoSleepAt = -1;

void printTemp() {
    auto current = weather.getCurrentData();
    auto tomorrow = weather.getTomorrowData();

    if (current.dt > 0) {
        display.setCursor(0, 24);
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

        gotoSleepAt = millis(); // can go to sleep now
    }
}

void printBattery() {
    battertyVoltage = (float)(analogRead(BAT_TEST_PIN)) / 4095 * 2 * 3.3 * 1.1;
    Serial.print("VBat = ");
    Serial.printf("%.2f", battertyVoltage);
    Serial.println("V");

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

    // display.setCursor(0, 120);
    // display.printf("Boot count %d", bootCount);

    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);

    nextDisplayUpdate = t + 1000;
}

void loop() {
    unsigned long t = millis();

    wifi.update(t);
    weather.update(t);
    updateDisplay(t);

    if (t >= gotoSleepAt) {
        goToSleep();
    }
}