#include <Arduino.h>
#include <EEPROM.h>

const int EEPROM_STRING_MAX = 50;

void startEEPROM() {
    EEPROM.begin(512);
}

int debounceRead(uint8 pinButton, int lastButton) {
  int current = digitalRead(pinButton);
  if (current != lastButton) {
    delay(300);
    current = digitalRead(pinButton);
  }
  return current;
}

String URLEncode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  //char code2;
  for (uint16_t i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      //code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
      //encodedString+=code2;
    }
    yield();
  }
  return encodedString;
}

String readStringEEPROM(int startIdx) {


  String res = "";
  for (int i = 0; i < EEPROM_STRING_MAX; ++i)
  {
    byte sm = EEPROM.read(startIdx + i);
    if (sm > 0) {
      if (sm >= 32 && sm <= 127) {
        res += char(sm);
      }
    } else {
      break;
    }
  }
  return res;
}

void writeStringEEPROM(String str, int startIdx) {
  for (int i = 0; i < EEPROM_STRING_MAX; ++i)
  {
    EEPROM.write(startIdx + i, 0);
  }
  for (unsigned int i = 0; i < str.length(); ++i)
  {
    EEPROM.write(startIdx + i, str[i]);
  }
  EEPROM.commit();
  delay(500);
}

//This function will write a 2 byte integer to the eeprom at the specified address and address + 1
void writeIntEEPROM(unsigned int p_value, int p_address) {
  byte lowByte = ((p_value >> 0) & 0xFF);
  byte highByte = ((p_value >> 8) & 0xFF);

  EEPROM.write(p_address, lowByte);
  EEPROM.write(p_address + 1, highByte);
}

//This function will read a 2 byte integer from the eeprom at the specified address and address + 1
unsigned int readIntEEPROM(int p_address) {
  byte lowByte = EEPROM.read(p_address);
  byte highByte = EEPROM.read(p_address + 1);

  return ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
}

String macToString(const unsigned char* mac) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}
