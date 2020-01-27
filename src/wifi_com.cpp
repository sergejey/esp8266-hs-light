#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>

int getWiFiQuality() {
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  int dBm = WiFi.RSSI();
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //ticker.attach(0.2, tick);
}

void startWiFi() {
  String device_id = WiFi.macAddress();
  device_id.replace(":", "");
  String access_point_name = "MHS"+device_id;

  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(180);
  wifiManager.setAPCallback(configModeCallback);
  if (!wifiManager.autoConnect(access_point_name.c_str())) {
    Serial.println("Failed to connect!");
    ESP.reset();
    delay(1000);
  }
}

