#include <Arduino.h>
#include <config.h>
#include <mqtt_com.h>

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
