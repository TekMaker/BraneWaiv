#include <Adafruit_NeoPixel.h>

//#define LED_BUILTIN 2  // just for indication on board
#define PIN_WS2812B 8  // The ESP32 pin GPIO16 connected to WS2812B
#define NUM_PIXELS 25  // The number of LEDs (pixels) on WS2812B LED strip

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG    

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__WIFI_POINT

#include <WiFi.h>

// RemoteXY connection settings 
#define REMOTEXY_WIFI_SSID "RemoteXY"
#define REMOTEXY_WIFI_PASSWORD "12345678"
#define REMOTEXY_SERVER_PORT 6377
#define REMOTEXY_ACCESS_PASSWORD "12345678"


#include <RemoteXY.h>

// RemoteXY GUI configuration  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 253 bytes
  { 255,3,0,3,0,246,0,19,0,0,0,66,114,97,110,101,87,97,105,118,
  0,31,1,106,200,1,1,8,0,65,44,4,18,18,112,129,9,31,91,12,
  64,6,66,114,97,110,101,87,97,105,118,32,77,111,111,100,0,129,34,61,
  39,12,64,6,80,97,116,116,101,114,110,0,129,38,96,29,12,64,6,87,
  97,118,101,0,12,29,74,54,11,195,30,26,49,32,76,69,68,0,53,32,
  76,69,68,115,0,57,32,76,69,68,115,0,49,51,32,76,69,68,115,0,
  50,53,32,76,69,68,115,0,12,23,110,63,11,196,30,26,49,45,65,109,
  98,105,101,110,116,0,50,45,71,97,109,109,97,32,50,45,53,32,72,122,
  0,51,45,66,101,116,97,32,49,51,45,50,53,32,72,122,0,52,45,65,
  108,112,104,97,32,56,45,49,50,32,72,122,0,53,45,84,104,101,116,97,
  32,52,45,56,32,72,122,0,54,45,68,101,108,116,97,32,49,45,53,32,
  72,122,0,10,43,152,24,24,48,12,26,31,79,78,0,31,79,70,70,0,
  129,39,134,34,12,64,6,80,111,119,101,114,0 };
  
// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t patternSelector; // from 0 to 5
  uint8_t waveSelector; // from 0 to 6
  uint8_t btnPower; // =1 if state is ON, else =0, from 0 to 1

    // output variables
  uint8_t led_01_r; // =0..255 LED Red brightness, from 0 to 255
  uint8_t led_01_g; // =0..255 LED Green brightness, from 0 to 255
  uint8_t led_01_b; // =0..255 LED Green brightness, from 0 to 255

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////



#define numWaves 6
String waveName[numWaves] = { "Ambient", "Gamma 2-5", "Beta 13-25", "Alpha 8-12", "Theta 4-8", "Delta 1-3" };

// these are the frequencies for each mode
#define ambientFreq 150
int mode[numWaves] = {
  ambientFreq,
  2,
  5,
  10,
  21,
  40
};

int colour[numWaves][3] = {
  { 255, 206, 161 },  // 4500K (slightly cool neutral)
  { 16, 233, 32 },    // green
  { 255, 16, 32 },    // red
  { 255, 16, 255 },   // purple
  { 32, 64, 255 },    // blue
  { 255, 127, 0 }     // yellow/orange
  // { 255, 180, 107 },  // 3000K (warm white, halogen)
  // { 255, 188, 132 },  // 3500K (warm neutral)
  // { 255, 197, 143 },  // 4000K (neutral white)
  // { 255, 206, 161 },  // 4500K (slightly cool neutral)
  // { 255, 214, 179 },  // 5000K (daylight white)
  // { 255, 219, 193 },  // 5500K (noon daylight)
  // { 255, 225, 209 },  // 6000K (cool daylight)
  // { 255, 229, 229 }   // 6500K (standard daylight / D65)
};

/*
level 1-1 led
0 0 0 0 0 
0 0 0 0 0 
0 0 1 0 0 
0 0 0 0 0 
0 0 0 0 0 

level 2 -5 leds 
0 0 0 0 0 
0 1 0 1 0 
0 0 1 0 0 
0 1 0 1 0 
0 0 0 0 0 

level 3-9 leds
0 0 1 0 0
0 1 0 1 0 
1 0 1 0 1
0 1 0 1 0 
0 0 1 0 0

level 4-13 leds
1 0 1 0 1
0 1 0 1 0 
1 0 1 0 1
0 1 0 1 0 
1 0 1 0 1

level 5-13 leds
1 1 1 1 1 
1 1 1 1 1 
1 1 1 1 1
1 1 1 1 1 
1 1 1 1 1
*/
/* Pixel patterns for different brightness */
#define numPatterns 5
int patterns[numPatterns][25] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0 },
  { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
};


int pattern = 0;  // 0 indexed
int wave = 0;     // 0 indexed

// define ws2812 object
Adafruit_NeoPixel ws2812b(NUM_PIXELS, PIN_WS2812B, NEO_GRB + NEO_KHZ800);

void setup() {

  RemoteXY_Init();
  Serial.begin(19200);

  // built in led is [0]
  ws2812b.begin();  // initialize WS2812B strip object (REQUIRED)
  ws2812b.clear();  // set all pixel colors to 'off'. It only takes effect if pixels.show() is called
  ws2812b.show();
  RemoteXY_delay(3000);  // allow serial to settle down
}

// main program loop
void loop() {
  RemoteXY_Handler();
  // // get controls
  // if (RemoteXY.btnPatternUp == 1) {
  //   pattern++;
  //   RemoteXY.btnPatternUp = 0;
  // }



  // if (RemoteXY.btnPatternDwn == 1) {
  //   pattern--;
  //   RemoteXY.btnPatternDwn = 0;
  // }

  // if (pattern >= numPatterns) pattern = numPatterns;
  // if (pattern <= 0) pattern = 0;

  pattern = RemoteXY.patternSelector;
  wave = RemoteXY.waveSelector;

  // RemoteXY.value_01 = pattern + 1;

  // if (RemoteXY.btnWaveUp == 1) {
  //   wave++;
  //   RemoteXY.btnWaveUp = 0;
  // }

  // if (RemoteXY.btnWaveDwn == 1) {
  //   wave--;
  //   RemoteXY.btnWaveDwn = 0;
  // }
  // if (wave >= numWaves) wave = numWaves;
  // if (wave <= 0) wave = 0;

  // RemoteXY.value_02 = wave + 1;

  // need to convert String wavename[wave] to char[] array tor RemoteXY.text01
  // String myString = waveName[wave];
  // int len = myString.length();
  // char myArray[len];
  // myString.toCharArray(myArray, sizeof(myArray));
  // Serial.println(myString);
  // strcpy(RemoteXY.text01, myArray);
  flashLEDs(colour[wave][0], colour[wave][1], colour[wave][2], mode[wave], pattern);
}


void flashLEDs(int r, int g, int b, float freqHZ, int index) {

  float dly = 1000 / freqHZ;  // period;

  for (int pixel = 0; pixel < NUM_PIXELS; pixel++) {
    if (patterns[index][pixel] == 1 && RemoteXY.btnPower == 1) {
      RemoteXY.led_01_r = r;
      RemoteXY.led_01_g = g;
      RemoteXY.led_01_b = b;
      ws2812b.setPixelColor(pixel, ws2812b.Color(r, g, b));
    } else {
      if (freqHZ < 50) {
        ws2812b.setPixelColor(pixel, ws2812b.Color(0, 0, 0));
      }
    }
  }
  ws2812b.show();
  RemoteXY_delay(dly / 2);  // hold LEDs ON

  RemoteXY.led_01_r = 0;
  RemoteXY.led_01_g = 0;
  RemoteXY.led_01_b = 0;
  ws2812b.clear();
  ws2812b.show();
  RemoteXY_delay(dly / 2);  // hold LEDs OFF

  //Serial.println(dly);
}
