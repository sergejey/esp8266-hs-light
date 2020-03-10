#ifndef mqtt_com_h
#define mqtt_com_h

#include <Arduino.h>
#include <PubSubClient.h>
#include <wifi_com.h>
#include <utils.h>
#include <io.h>
#include <pixel.h>

void publishEvent(String event_name, String payload);
void callback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT();
void handleMQTT();
void startMQTT();

bool isMQTTConnected();
String getMQTTServer();
String getDeviceTitle();
String getMQTTServer();
String getMQTTUser();
String getMQTTPassword();
String getMQTTTopicBase();

#endif