#ifndef io_h
#define io_h

#include <Arduino.h>
#include <mqtt_com.h>

void turnRelay(uint8 relay_num, bool state);

#endif