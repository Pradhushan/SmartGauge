#ifndef OTACONFIG_H
#define OTACONFIG_H

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "html.h"

// Declare the getWaterLevel function
extern int getWaterLevel();
// Declare heighttank as extern
extern int heighttank;

// WiFi configuration
const char* ssid = "SmartGauge Jagro";
const char* password = "jagro@4251";
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
AsyncWebServer server(80);

void setupOTA() {
  // Set up WiFi Access Point
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  Serial.println();
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started");

  // Start the DNS server for captive portal redirection
  dnsServer.start(DNS_PORT, "*", apIP); 

  // Print the IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // OTA setup
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_SPIFFS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.begin();
  Serial.println("OTA ready");

  // Handle HTTP requests
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    int waterLevel = getWaterLevel();
    if (waterLevel == -1) {
      request->send_P(200, "text/html", noDataPage);
    } else {
      float waterLevelFeet = waterLevel / 30.48;
      float waterLevelPercent = (waterLevel * 100) / heighttank;
 
      char dataPageBuffer[2200];
      snprintf(dataPageBuffer, sizeof(dataPageBuffer), dataPageTemplate, waterLevelFeet, waterLevelPercent, waterLevelPercent, waterLevelPercent);

      //Serial.println(dataPageBuffer);  // Debug print the buffer to check the content

      request->send_P(200, "text/html", dataPageBuffer);
    }
  });

  server.onNotFound([](AsyncWebServerRequest* request) {
    request->redirect("/");
  });

  server.begin();
}

void handleOTA() {
  dnsServer.processNextRequest();
  ArduinoOTA.handle();
}

#endif
