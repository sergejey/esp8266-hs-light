#include <Arduino.h>
#include <OneWire.h>

float getTemp(uint8 pinNum) {

  OneWire ds(pinNum);

  byte wire_addr[8];
  byte data[12];
  
  if (!ds.search(wire_addr)) {
    Serial.println("No more addresses.");
    while (1);
  }
  ds.reset_search();
  if (OneWire::crc8(wire_addr, 7) != wire_addr[7]) {
    Serial.println("CRC is not valid!");
    while (1);
  }
  ds.reset();
  ds.select(wire_addr);
  ds.write(0x44);
  delay(1000);
  ds.reset();
  ds.select(wire_addr);
  ds.write(0xBE);
  for (int i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
  int raw = (data[1] << 8) | data[0];
  if (data[7] == 0x10) raw = (raw & 0xFFF0) + 12 - data[6];
  return raw / 16.0;
}