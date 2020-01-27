#ifndef wifi_com_h
#define wifi_com_h

#include <Arduino.h>
#include <ESP8266WiFi.h>

WiFiClient espClient;

int getWiFiQuality();
void startWiFi();

#endif