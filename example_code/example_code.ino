// This is a sample sketch for the 3x5 LED-Matrix Kit from FabLab Winti.
// See http://www.fablabwinti.ch/fablab-kits/3x5-led-matrix/ for Details.
// 
// This kit uses the Wemos D1 mini ESP8266 board, so please add the following
// URL to the "Additional Boards Manager URLs" in Arduino IDE File>Prefferences:
// http://arduino.esp8266.com/stable/package_esp8266com_index.json
//
// Then go to Tools>Board>Boards Manager and type esp8266 into the filter 
// field and install the esp8266 (by ESP8266 Community) line.
// 
// Then install the following libraries via the Library Manager:
// - Adafruit NeoPixel
// - ESP8266 WiFiManager (by tzapu)
//
// For programming, select "WeMos D1 R2 & mini" in Tools>Boards.
//

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <Adafruit_NeoPixel.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

// Button Definitions
#define POWER_BUTTON_PIN    14  // left button (On/Off)
#define NEXT_PRG_BUTTON_PIN 12  // center button (Next Program)
#define PREV_PRG_BUTTON_PIN 13  // right button (Previous Program)
#define CONFIG_BUTTON_PIN 13    // right button (GoTo WiFi Config)

// NeoPixel Definitions
#define PIXEL_PIN    5    // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 15

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

volatile bool powerState = LOW;
volatile bool powerChanged = false;
volatile int  prgNum = 1;
volatile bool prgChanged = false;

void setup() {
  pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(NEXT_PRG_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CONFIG_BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(POWER_BUTTON_PIN, power_isr, FALLING);
  attachInterrupt(NEXT_PRG_BUTTON_PIN, next_prg_isr, FALLING);
  //attachInterrupt(PREV_PRG_BUTTON_PIN, prev_prg_isr, FALLING);
  strip.begin();
  colorWipe(strip.Color(0, 0, 0), 50);
  strip.show(); // Initialize all pixels to 'off'
}

void power_isr() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 300)
  {
    if (powerState == LOW) {
      //Switch on
      powerState = HIGH;
      delayMicroseconds(20000);
      powerChanged = true;
    } else {
      //Shut off
      powerState = LOW;
      delayMicroseconds(20000);
      powerChanged = true;
    }
    last_interrupt_time = interrupt_time;
  }
}

void next_prg_isr() {
  if (powerState == HIGH) {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 200ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 300)
    {
      //Step to next program
      prgNum++;
      if (prgNum > 9) prgNum=1;
      prgChanged = true;
      last_interrupt_time = interrupt_time;
    }
  }
}

void prev_prg_isr() {
  if (powerState == HIGH) {
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();
    // If interrupts come faster than 200ms, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 300)
    {
      //Step to previous program
      prgNum--;
      if (prgNum < 1) prgNum=9;
      prgChanged = true;
      last_interrupt_time = interrupt_time;  
    }
  }
}

void loop() {
  // WiFi configuration portal requested?
  if ( digitalRead(CONFIG_BUTTON_PIN) == LOW ) {
    //WiFiManager
    WiFiManager wifiManager;

    //reset settings - for testing
    //wifiManager.resetSettings();

    //sets timeout until configuration portal gets turned off
    wifiManager.setTimeout(120);

    //it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration

    //WITHOUT THIS THE AP DOES NOT SEEM TO WORK PROPERLY WITH SDK 1.5 , update to at least 1.5.1
    //WiFi.mode(WIFI_STA);
    
    if (!wifiManager.startConfigPortal("3x5 LED-Matrix")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }
  }
  
  if (powerChanged == true) {
    powerChanged = false;
    if (powerState == LOW) {
      startShow(0);
    } else {
      startShow(prgNum);
    }
  }
  
  if (prgChanged == true) {
    prgChanged = false;
    startShow(prgNum);
  }
}

void startShow(int i) {
  switch(i){
    case 0: colorWipe(strip.Color(0, 0, 0), 50);    // Black/off
            break;
    case 1: colorWipe(strip.Color(255, 0, 0), 50);  // Red
            break;
    case 2: colorWipe(strip.Color(0, 255, 0), 50);  // Green
            break;
    case 3: colorWipe(strip.Color(0, 0, 255), 50);  // Blue
            break;
    case 4: theaterChase(strip.Color(127, 127, 127), 50); // White
            break;
    case 5: theaterChase(strip.Color(127,   0,   0), 50); // Red
            break;
    case 6: theaterChase(strip.Color(  0,   0, 127), 50); // Blue
            break;
    case 7: rainbow(20);
            break;
    case 8: rainbowCycle(20);
            break;
    case 9: theaterChaseRainbow(50);
            break;
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);

    // Break on interrupt
    if (powerChanged == true || prgChanged == true) {
      return;
    }
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);

    // Break on interrupt
    if (powerChanged == true || prgChanged == true) {
      return;
    }
    if (j == 255) j = 0;
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) { // cycle of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);

    // Break on interrupt
    if (powerChanged == true || prgChanged == true) {
      return;
    }
    if (j == 255) j = 0;
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
  
      // Break on interrupt
      if (powerChanged == true || prgChanged == true) {
        return;
      }
    }
    if (j == 9) j = 0;
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
  
      // Break on interrupt
      if (powerChanged == true || prgChanged == true) {
        return;
      }
    }
    if (j == 255) j = 0;
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
