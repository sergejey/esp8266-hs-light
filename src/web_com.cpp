#include <Arduino.h>
#include <config.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <utils.h>
#include <wifi_com.h>
#include <mqtt_com.h>

ESP8266WebServer server(80);


void handleRoot() {

  String message = "<html><head><title>MalinoHS</title></head><body><h1>Device id: " + getDeviceTitle() + "</h1>\n";
  message += "<p><h2>Status</h2><span id='latestData'>";
  if (isMQTTConnected()) {
    message += "<font color='green'>Connected</font>";
  } else {
    message += "<font color='red'>Disconnected</font>";
  }
  message += "</span>\n";

  message += "<h2>CONTROL:</h2>";
  message += "<p><a href='/reboot' onclick='return confirm(\"Are you sure? Please confirm\");'>Reboot</a></p>";

  message += "<h2>Settings</h2><form action='/save' method='get'>";
  message += "MQTT server:<br/><input type='text' name='mqtt_server' value='"; message += getMQTTServer(); message += "' size='60'><br/>";
  message += "Username:<br/><input type='text' name='mqtt_user' value='"; message += getMQTTUser(); message += "' size='60'><br/>";
  message += "Password:<br/><input type='text' name='mqtt_password' value='"; message += getMQTTPassword(); message += "' size='60'><br/>";
  message += "MQTT base topic:<br/><input type='text' name='mqtt_base' value='"; message += getMQTTTopicBase(); message += "' size='60'><br/>";
  message += "Device ID:<br/><input type='text' name='device_title' value='"; message += getDeviceTitle(); message += "' size='60'><br/>";
  message += "<input type='submit' value='Save settings'></form>";

  message += "<script language='javascript' src='https://ajax.googleapis.com/ajax/libs/jquery/2.2.4/jquery.min.js'></script>";
  message += "<script language=\"javascript\">\n";
  message += "var status_timer;\n";
  message += "function updateStatus() {\n";
  message += "$.ajax({  url: \"/data\"  }).done(function(data) {  $('#latestData').html(data);status_timer=setTimeout('updateStatus();', 500); });";
  message += "}\n";
  message += "$(document).ready(function() {  updateStatus();   });\n";
  message += "</script>";
  message += "</body></html>";
  server.sendHeader("Connection", "close");

  server.send(200, "text/html", message);
}

void handleReboot() {
  String message = "<html><body><script language='javascript'>tm=setTimeout(\"window.location.href='/';\",2000);</script>Rebooting...</body></html>";
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", message);
  delay(2000);
  ESP.restart();
}

void handleData() {

  String message = "";

  message += "WiFi signal level: ";
  message += getWiFiQuality();
  message += "; ";

  if (isMQTTConnected()) {
    message += "<font color='green'>MQTT Connected</font>; ";
  } else {
    message += "<font color='red'>MQTT Disconnected</font>; ";
  }
  //message += " Sensor status: ";
  //message += buttonState;
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", message);
}

void handleUpdateConfig() {

  String mqtt_server;
  String mqtt_user;
  String mqtt_password;
  String device_title;
  String mqtt_topic_base;

  for (uint16_t i = 0; i < server.args(); i++) {
    if (server.argName(i) == "mqtt_server") {
      mqtt_server = server.arg(i);
    }
    if (server.argName(i) == "mqtt_user") {
      mqtt_user = server.arg(i);
    }
    if (server.argName(i) == "mqtt_password") {
      mqtt_password = server.arg(i);
    }
    if (server.argName(i) == "mqtt_base") {
      mqtt_topic_base = server.arg(i);
    }
    if (server.argName(i) == "device_title") {
      device_title = server.arg(i);
    }
  }
  String message = "<html><body><script language='javascript'>tm=setTimeout(\"window.location.href='/reboot';\",1000);</script>Data saved! Processing to reboot...</body></html>";

  writeStringEEPROM(mqtt_server, EEPROM_MQTT_SERVER);
  writeStringEEPROM(mqtt_user, EEPROM_MQTT_USER);
  writeStringEEPROM(mqtt_password, EEPROM_MQTT_PASSWORD);
  writeStringEEPROM(device_title, EEPROM_MQTT_DEVICE_TITLE);
  writeStringEEPROM(mqtt_topic_base, EEPROM_MQTT_BASE);

  server.sendHeader("Connection", "close");
  server.send(200, "text/html", message);

}

void startWeb() {
  server.on("/", handleRoot);
  server.on("/reboot", handleReboot);
  server.on("/data", handleData);
  server.on("/save", handleUpdateConfig);
  server.begin();
}

void handleWeb() {
  server.handleClient();
}