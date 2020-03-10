#include <Arduino.h>

#include <config.h>
#include <utils.h>
#include <wifi_com.h>
#include <web_com.h>
#include <mqtt_com.h>
#include <ota.h>
#include <io.h>
#include <pixel.h>

#ifdef PIN_TEMP
#include <temperature.h>
float temperature = 0;
float old_temperature = 0;
#endif

long latestTempRequested = 0;

long lastMsg = 0;
char msg[50];

int switch1State = 0;
int switch2State = 0;

int switch1StateOld = 0;
int switch2StateOld = 0;

int motionState = 0;
int motionStateOld = 0;

int micState = 0;
int micStateOld = 0;
int noiseStatus = 0;
int latestNoiseDetected = 0;
String knockPattern = "";
int latestKnock = 0;

int lastPingSent = 0;
int lastSwitchChange = 0;

void setup() {

  startEEPROM();
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

#ifdef PIN_MIC
  pinMode(PIN_MIC, INPUT);
  noiseStatus = 0;
#endif

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
  startMQTT();
  startWeb();
  startPixel();

  String device_id = WiFi.macAddress();
  device_id.replace(":", "");
  String access_point_name = "MHS"+device_id;
  startOTA(access_point_name);


}


void loop() {

  int tm = round(millis() / 1000);

  handleOTA();
  handleWeb();
  handleMQTT();
  handlePixel();


#ifdef PIN_SWITCH1
  switch1State = debounceRead(PIN_SWITCH1, switch2StateOld);
  if (switch1State!=switch1StateOld) {
    // SWITCH 1 changed
    switch1StateOld = switch1State;
    Serial.print("Switch 1 changed to ");
    Serial.println(switch1State);
    if (switch1State) {
      turnRelay(1,false);
    } else {
      turnRelay(1,true);
    }
    publishEvent("switch1",String(switch1State));
    //publishEvent("switchPassed",String(tm-lastSwitchChange));
    lastSwitchChange = tm;
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
      turnRelay(2,false);
    } else {
      turnRelay(2,true);
    }
    publishEvent("switch2",String(switch2State));
    //publishEvent("switchPassed",String(tm-lastSwitchChange));
    lastSwitchChange = tm;
  }
#endif  

#ifdef PIN_MIC
  micState = digitalRead(PIN_MIC);
  int knockPassed = millis()-latestKnock;
  if (micState>0) {
     // MIC LEVEL CHANGED
    latestNoiseDetected = tm;
    latestKnock = millis();
    if (knockPassed>KNOCK_DEBOUNCE_TIME) { // debounce
      knockPattern+="*"; // short
      Serial.println("Knock!");
    }
    micStateOld=micState;
    if (noiseStatus == 0) { // Making some noise
     noiseStatus = 1;
     publishEvent("noise",String(noiseStatus));
    }
  } else {
    if (latestKnock>0 && knockPattern!="") {
      if (knockPassed>KNOCK_FADEOUT_TIME) {
       latestKnock=0;
       if (knockPattern.substring(knockPattern.length()-1)==" ") {
         knockPattern = knockPattern.substring(0, knockPattern.length()-1);
       }
       if (knockPattern.length()>1) { // skip if ther was only one knock
        publishEvent("knock",knockPattern);
       }
       knockPattern="";
      } else if (knockPassed>KNOCK_DELAY_TIME && knockPattern.substring(knockPattern.length()-1)!=" ") {
        knockPattern+=" ";
      }
    }
    if ((noiseStatus==1) && abs(tm-latestNoiseDetected)>NOISE_TIMEOUT) {
      latestKnock=0;
      noiseStatus=0;
      knockPattern = "";
      publishEvent("noise",String(noiseStatus));
    }
  }
#endif

#ifdef PIN_MOTION
  motionState = digitalRead(PIN_MOTION);
  if (motionState!=motionStateOld) {
    // MOTION DETECTED
    motionStateOld=motionState;
    Serial.print("Motion changed to ");
    Serial.println(motionState);
    if (motionState>0) {
     setPixelColor("green");
    } else {
     setPixelColor("black");
    }
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
    payload += "', 'switches':'";
    payload += switch1StateOld;    
    payload += ",";
    payload += switch2StateOld;        
    payload += "','signal':";
    payload += getWiFiQuality();
    payload += "}";
    publishEvent("ping",payload);
    lastPingSent = tm;
  }

}
