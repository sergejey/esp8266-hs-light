#ifndef utils_h
#define utils_h

void startEEPROM();
int debounceRead(uint8 pinButton, int lastButton);
String URLEncode(String str);
String readStringEEPROM(int startIdx);
void writeStringEEPROM(String str, int startIdx);
void writeIntEEPROM(unsigned int p_value, int p_address);
unsigned int readIntEEPROM(int p_address);
String macToString(const unsigned char* mac);

#endif