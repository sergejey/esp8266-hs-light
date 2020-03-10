#ifndef config_h
#define config_h

#define PIN_PIXEL D4 // Connect RGB light
#define PIN_SWITCH1 D3 // Switch 1
#define PIN_SWITCH2 D2 // Switch 2
#define PIN_RELAY1 D1 // Relay 1
#define PIN_RELAY2 D6 // Relay 2
//#define PIN_TEMP D8 // Connect temp sensor
#define PIN_MOTION D5 // Motion sensor
#define PIN_MIC D7 // Microphone sensor

#define KNOCK_DEBOUNCE_TIME 150 // ms, debounce timer
#define KNOCK_FADEOUT_TIME 1800 // ms, knocking is over timer
#define KNOCK_DELAY_TIME 400 // ms, long delay between knocks
#define NOISE_TIMEOUT 10 // s, noise is over

const int EEPROM_MQTT_DEVICE_TITLE = 50;
const int EEPROM_MQTT_USER = 100;
const int EEPROM_MQTT_PASSWORD = 150;
const int EEPROM_MQTT_SERVER = 200;
const int EEPROM_MQTT_BASE = 250;


#endif