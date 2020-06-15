#include <Arduino.h>

#include <GxEPD.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

#include <GxGDEH0213B72/GxGDEH0213B72.h>

#define CS_PIN SS
#define DC_PIN 17
#define RST_PIN 16
#define BUSY_PIN 4
#define LED_PIN 19

GxIO_Class io(SPI, CS_PIN, DC_PIN, RST_PIN);
GxEPD_Class display(io, RST_PIN, BUSY_PIN);

void drawHelloWorld()
{
  display.setTextColor(GxEPD_BLACK);
  display.print("Hello, World!");
}

void setup() {
  pinMode(LED_PIN, OUTPUT);

  display.init();
  display.eraseDisplay();
  display.drawPaged(drawHelloWorld);
}

bool on = false;

void loop() {
  if (on) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  on = !on;
  delay(1000);
}