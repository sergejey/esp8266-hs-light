#ifndef web_com_h
#define web_com_h

#include <Arduino.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <utils.h>

void handleRoot();
void handleReboot();
void handleData();
void handleUpdateConfig();
void startWeb();
void handleWeb();

#endif