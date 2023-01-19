#include <WiFiClient.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "auth.h"


int leftButton = 0;
int rightButton = 26;

DynamicJsonDocument reading(512);

uint32_t now, updatePeriod = 60000;

TFT_eSPI tft = TFT_eSPI();
void setup(void) {
    Serial.begin(115200);
    Serial.println("\n\nthingspeak display");

    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);

    tft.setCursor(0, 0, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.println("Wifi Connecting");
    tft.println(WIFI_SSID);

    pinMode(leftButton, INPUT_PULLUP);
    pinMode(rightButton, INPUT_PULLUP);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("WiFi Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());


    ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else  // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 2);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.setTextSize(2);
      tft.println("OTA in progress");
    })
    .onEnd([]() {
      Serial.println("\nEnd");
      tft.fillScreen(TFT_BLACK);
      tft.setCursor(0, 0, 2);
      tft.println("OTA complete");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      tft.setCursor(0, 35, 2);
      tft.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();

    tft.println(WiFi.localIP());
    tft.println("updating");
    update();
}

void display(String label, String value) {
    float v = value.toFloat();

    tft.fillScreen(TFT_BLACK);

    tft.setCursor(0, 0, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.println(label);

    tft.setTextColor(TFT_GREEN);
    if (v < 50.0)
        tft.setTextColor(TFT_YELLOW);
    if (v < 43.0)
        tft.setTextColor(TFT_RED);

    tft.setTextFont(7);
    tft.setTextSize(2);
    if (v > 100) { 
      tft.printf("%d", int(v));
    } else {
      tft.printf("%0.1f", v);
    }
}

void displayIP() {
    tft.fillScreen(TFT_BLACK);

    tft.setCursor(0, 0, 2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.println(WIFI_SSID);
    tft.println(WiFi.localIP());
    tft.printf("signal %ddBm\n",WiFi.RSSI());
}

void displayError(String label) {
    tft.fillScreen(TFT_RED);

    tft.setCursor(0, 0, 2);
    tft.setTextColor(TFT_BLACK, TFT_RED);
    tft.setTextSize(2);
    tft.println(label);
}

void update() {
    char label[30];
    HTTPClient http;
    http.begin("https://thingspeak.com/channels/258548/feed/last.json?timezone=America/Denver");
    int httpCode = http.GET();
    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            deserializeJson(reading, payload);
            Serial.println(payload);
            String ts = reading["created_at"];
            sprintf(label, "Greenhouse %s", ts.substring(11, 16));
            display(label, reading["field2"]);
        }
    } else {
        Serial.printf("HTTP error: %s\n", http.errorToString(httpCode).c_str());
    }
}

void loop() {
    if ((millis() - now) > updatePeriod) {
        update();
        now = millis();
    }
    if (!digitalRead(leftButton)) {
        displayIP();
        while (!digitalRead(leftButton)) ;
        tft.println("updating");
        update();
    }
    ArduinoOTA.handle();
}
