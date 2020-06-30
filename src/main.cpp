#include <Arduino.h>

#include <GxEPD.h>
#include <GxGDEH0213B72/GxGDEH0213B72.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <Fonts/FreeMono12pt7b.h>

#include "WiFiModule.h"

#define CS_PIN SS
#define DC_PIN 17
#define RST_PIN 16
#define BUSY_PIN 4
#define LED_PIN 19

GxIO_Class io(SPI, CS_PIN, DC_PIN, RST_PIN);
GxEPD_Class display(io, RST_PIN, BUSY_PIN);

WiFiModule wifi;

void drawHelloWorld()
{
    display.setCursor(0, 20);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(&FreeMono12pt7b);
    display.print("Hello, World!");
}

void setup() {
    Serial.begin(115200);
    wifi.setup();

    display.init();
    display.eraseDisplay();
    display.setRotation(1);
    display.setFont(&FreeMono12pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(0, 20);
    display.printf("Hello, world!");
    // display.drawPaged(drawHelloWorld);
    display.update();
    display.updateWindow(0, 0, GxEPD_WIDTH, GxEPD_HEIGHT, false);

    pinMode(LED_PIN, OUTPUT);
}

bool on = false;
bool connected = false;

unsigned long nextDisplayUpdate = 0;

void updateDisplay(const unsigned long t) {
    if (t < nextDisplayUpdate) return;

    display.fillRect(0, 50, 50, 50, GxEPD_WHITE);

    display.setCursor(2, 80);
    if (wifi.isConnected() && !connected) {
        display.setCursor(2, 40);
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