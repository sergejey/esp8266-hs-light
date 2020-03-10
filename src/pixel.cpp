#include <Arduino.h>
#include <config.h>
#include <NeoPixelBus.h>
#include <mqtt_com.h>

const uint16_t PixelCount = 1;
NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod> strip(PixelCount, PIN_PIXEL);

#define colorSaturation 128
RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

String latestColor = "black";
bool isBlinking = false;
bool latestBlink = false;
long blinkPassed = 0;

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

void startPixel() {
  strip.Begin();
  strip.Show();
  setPixelColor("red");
}

void handlePixel() {
  int tm = round(millis() / 1000);
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
}

void startPixelBlinking() {
 isBlinking = true;
}

void stopPixelBlinking() {
 isBlinking = false;
 setPixelColor(latestColor);
}