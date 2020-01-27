#include <Arduino.h>

#define PIN_PIXEL D4 // Connect RGB light
#define PIN_RELAY1 D7 // Relay 1
#define PIN_RELAY2 D3 // Relay 2
#define PIN_SWITCH1 D1 // Switch 1
#define PIN_SWITCH2 D2 // Switch 2
#define PIN_TEMP D5 // Connect temp sensor
//#define PIN_MOTION D6 // Motion sensor

#include <utils.h>
#include <wifi_com.h>
#include <ota.h>
#include <temperature.h>

#include <PubSubClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <NeoPixelBus.h>

float temperature = 0;
float old_temperature = 0;

ESP8266WebServer server(80);
PubSubClient client(espClient);


const uint16_t PixelCount = 1;

NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod> strip(PixelCount, PIN_PIXEL);

#define colorSaturation 128
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

const int EEPROM_MQTT_DEVICE_TITLE = 50;
const int EEPROM_MQTT_USER = 100;
const int EEPROM_MQTT_PASSWORD = 150;
const int EEPROM_MQTT_SERVER = 200;
const int EEPROM_MQTT_BASE = 250;

String device_title = "";
String latestColor = "black";
bool isBlinking = false;
bool latestBlink = false;
long blinkPassed = 0;
long latestTempRequested = 0;

String mqtt_server;
String mqtt_user;
String mqtt_password;
String mqtt_topic_base;
String mqtt_topic_io;
String mqtt_topic_out;
String mqtt_topic_ping;

#include <Ticker.h>
Ticker ticker;

long lastMsg = 0;
char msg[50];

int switch1State = 0;
int switch2State = 0;

int switch1StateOld = 0;
int switch2StateOld = 0;

int motionState = 0;
int motionStateOld = 0;

int lastConnectionAttempt = 0;
int lastPingSent = 0;

void publishEvent(String event_name, String payload) {
  String topic;
  topic = mqtt_topic_out + "/" + event_name;
  Serial.print("Publishing to ");
  Serial.print(topic);
  Serial.print(" ");
  Serial.println(payload);
  client.publish(topic.c_str(), payload.c_str());

}

void setPixelColor(String colorName) {
  Serial.print("Setting color to ");
  Serial.println(colorName);
  if (!colorName.equals("black")) {
   latestColor = colorName;
  }
  if (colorName.equals("red")) {
   strip.SetPixelColor(0, red);
  } else if (colorName.equals("green")) {   
   strip.SetPixelColor(0, green);    
  } else if (colorName.equals("white")) {   
   strip.SetPixelColor(0, white);    
  } else if (colorName.equals("blue")) {       
   strip.SetPixelColor(0, blue);       
  } else if (colorName.equals("black")) {
   strip.SetPixelColor(0, black);
  }

  if (!colorName.equals("black")) {
   publishEvent("color",colorName);
  }
  strip.Show();
}

void turnRelay(uint8 relay_num, bool state) {

  if (state) {
    setPixelColor("red");
  } else {
    setPixelColor("blue");
  }

#ifdef PIN_RELAY1
  if (relay_num==1) {
    if (state) {
      Serial.println("Setting Relay 1 to ON");
      digitalWrite(PIN_RELAY1,LOW);
      publishEvent("relay1","1");
    } else {
      Serial.println("Setting Relay 1 to OFF");
      digitalWrite(PIN_RELAY1,HIGH);
      publishEvent("relay1","0");
    }
  }
#endif  

#ifdef PIN_RELAY2
  if (relay_num==2) {
    if (state) {
      Serial.println("Setting Relay 2 to ON");
      digitalWrite(PIN_RELAY2,LOW);
      publishEvent("relay2","1");
    } else {
      Serial.println("Setting Relay 2 to OFF");
      digitalWrite(PIN_RELAY2,HIGH);
      publishEvent("relay2","0");
    }
  }
#endif  
}

void callback(char* topic, byte* payload, unsigned int length) {
  String strTopic = String(topic);
  String strPayload = "";
  Serial.print("Message arrived [");
  Serial.print(strTopic);
  Serial.print("] ");
  for (uint16_t i = 0; i < length; i++) {
    strPayload+=(char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (strTopic.endsWith("color/set")) {
    setPixelColor(strPayload);
  }
  if (strTopic.endsWith("blink/set")) {
    if (strPayload.equals("1")) {
      Serial.println("Starting blinking...");
      isBlinking = true;
    } else {
      Serial.println("Stopping blinking...");
      isBlinking = false;
      setPixelColor(latestColor);
    }
  }  
  if (strTopic.endsWith("relay1/set")) {
    if (strPayload.equals("1")) {
      turnRelay(1,true);
    } else {
      turnRelay(1,false);
    }
  }
    if (strTopic.endsWith("relay2/set")) {
    if (strPayload.equals("1")) {
      turnRelay(2,true);
    } else {
      turnRelay(2,false);
    }
  }
  // Switch on the LED if an 1 was received as first character
}

void reconnectMQTT() {
  // Loop until we're reconnected
  while (!client.connected()) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Protoype-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    client.setServer(mqtt_server.c_str(), 1883);
    if (client.connect(clientId.c_str(), mqtt_user.c_str(), mqtt_password.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqtt_topic_out.c_str(), "hello world");
      // ... and resubscribe
      client.subscribe(mqtt_topic_io.c_str());
      digitalWrite(LED_BUILTIN, LOW);
      setPixelColor("green");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      return;
      //delay(5000);
    }
  }
}


void handleRoot() {

  String message = "<html><head><title>MalinoHS</title></head><body><h1>Device id: " + device_title + "</h1>\n";
  message += "<p><h2>Status</h2><span id='latestData'>";
  if (client.connected()) {
    message += "<font color='green'>Connected</font>";
  } else {
    message += "<font color='red'>Disconnected</font>";
  }
  message += "</span>\n";

  message += "<h2>CONTROL:</h2>";
  message += "<p><a href='/reboot' onclick='return confirm(\"Are you sure? Please confirm\");'>Reboot</a></p>";

  message += "<h2>Settings</h2><form action='/save' method='get'>";
  message += "MQTT server:<br/><input type='text' name='mqtt_server' value='"; message += mqtt_server; message += "' size='60'><br/>";
  message += "Username:<br/><input type='text' name='mqtt_user' value='"; message += mqtt_user; message += "' size='60'><br/>";
  message += "Password:<br/><input type='text' name='mqtt_password' value='"; message += mqtt_password; message += "' size='60'><br/>";
  message += "MQTT base topic:<br/><input type='text' name='mqtt_base' value='"; message += mqtt_topic_base; message += "' size='60'><br/>";
  message += "Device ID:<br/><input type='text' name='device_title' value='"; message += device_title; message += "' size='60'><br/>";
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

  if (client.connected()) {
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

void setup() {

  startEEPROM();

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

#ifdef PIN_SWITCH1
  pinMode(PIN_SWITCH1, INPUT);
#endif

#ifdef PIN_SWITCH2  
  pinMode(PIN_SWITCH2, INPUT);
#endif

#ifdef PIN_MOTION
  pinMode(PIN_MOTION, INPUT);
#endif  

#ifdef PIN_RELAY1
  pinMode(PIN_RELAY1, OUTPUT);
  turnRelay(1, false);
#endif  

#ifdef PIN_RELAY2
  pinMode(PIN_RELAY2, OUTPUT);
  turnRelay(2, false);
#endif  


  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);

  delay(10);

  startWiFi();

  String device_id = WiFi.macAddress();
  device_id.replace(":", "");
  String access_point_name = "MHS"+device_id;

  mqtt_server = readStringEEPROM(EEPROM_MQTT_SERVER);
  device_title = readStringEEPROM(EEPROM_MQTT_DEVICE_TITLE);
  if (device_title == "") {
    device_title = device_id;
  }

  mqtt_user = readStringEEPROM(EEPROM_MQTT_USER);
  mqtt_password = readStringEEPROM(EEPROM_MQTT_PASSWORD);
  mqtt_topic_base = readStringEEPROM(EEPROM_MQTT_BASE);
  if (mqtt_topic_base == "") {
    mqtt_topic_base = "devices/";
  }

  mqtt_topic_io = mqtt_topic_base + device_title + "/io/#";
  mqtt_topic_out = mqtt_topic_base + device_title + "/io";
  mqtt_topic_ping = mqtt_topic_base + device_title + "/ping";

  client.setServer(mqtt_server.c_str(), 1883);
  client.setCallback(callback);

  server.on("/", handleRoot);
  server.on("/reboot", handleReboot);
  server.on("/data", handleData);
  server.on("/save", handleUpdateConfig);
  server.begin();

  startOTA(access_point_name);

  strip.Begin();
  strip.Show();

  setPixelColor("red");

}


void loop() {

  int tm = round(millis() / 1000);

  handleOTA();

  if ((tm-blinkPassed)>1 && isBlinking) {
    blinkPassed=tm;
     if (latestBlink) {
      latestBlink = false;
      setPixelColor("black");
     } else {
      latestBlink = true;
      setPixelColor(latestColor);
     }
  }

  server.handleClient();
  if (!client.connected()) {
    if ((round(millis() / 1000) - lastConnectionAttempt) > 10) {
      lastConnectionAttempt = round(millis() / 1000);
      Serial.println("Reconnecting");
      reconnectMQTT();
    }
  }
  client.loop();

#ifdef PIN_SWITCH1
  switch1State = debounceRead(PIN_SWITCH1, switch2StateOld);
  if (switch1State!=switch1StateOld) {
    // SWITCH 1 changed
    switch1StateOld = switch1State;
    Serial.print("Switch 1 changed to ");
    Serial.println(switch1State);
    if (switch1State) {
      turnRelay(1,true);
    } else {
      turnRelay(1,false);
    }
    publishEvent("switch1",String(switch1State));
  }
#endif  

#ifdef PIN_SWITCH2
  switch2State = debounceRead(PIN_SWITCH2, switch2StateOld);
  if (switch2State!=switch2StateOld) {
    // SWITCH 2 changed
    switch2StateOld = switch2State;
    Serial.print("Switch 2 changed to ");
    Serial.println(switch2State);
    if (switch2State) {
      turnRelay(2,true);
    } else {
      turnRelay(2,false);
    }
    publishEvent("switch2",String(switch2State));
  }
#endif  

#ifdef PIN_MOTION
  motionState = digitalRead(PIN_MOTION);
  if (motionState!=motionStateOld) {
    // MOTION DETECTED
    motionStateOld=motionState;
    Serial.print("Motion changed to ");
    Serial.println(motionState);
    publishEvent("motion",String(motionState));
  }
#endif

#ifdef PIN_TEMP
if ((tm - latestTempRequested) > 15) {
  temperature = getTemp(PIN_TEMP);
  if (abs(temperature-old_temperature)>0.2) {
    Serial.print("Temperature changed to ");
    Serial.println(temperature);
    old_temperature = temperature;
    publishEvent("temp",String(temperature));
  }
  latestTempRequested = tm;
}
#endif

  if ((tm - lastPingSent) > 60) {
    String payload;
    payload = "{'ip':'";
    payload += WiFi.localIP().toString();
    payload += "', 'uptime':";
    payload += tm;
    payload += ",'signal':";
    payload += getWiFiQuality();
    payload += "}";
    publishEvent("ping",payload);
    lastPingSent = tm;
  }

}
