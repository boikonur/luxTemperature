/*
   Required libraries:

   - I2C-Sensor-Lib (iLib)
   - FastLed

  Used Hardware:

  - Pro Mini Atmega328 3.3V 8M  
    A4 (I2C - SDA), 
    A5 (I2C - SCL),
    12 (Neopixel out)
*/

#include <Wire.h>
#include "i2c.h"
#include "i2c_BMP280.h"
#include "FastLED.h"

BMP280 bmp280;

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define HIGH_TEMP_MAX 30
#define NORM_TEMP_MAX 24
#define LOW_TEMP_MAX 23
#define LOW_TEMP_MIN 18

#define DATA_PIN    12
#define LED_TYPE    WS2812
#define COLOR_ORDER RGB
#define NUM_LEDS    12
#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

CRGB leds[NUM_LEDS];
uint8_t gHue = 0;
float temperature;
float pascal;
static float meters, metersold;



void setup() {
  delay(2000);


  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  Serial.print("Probe BMP280: ");
  if (bmp280.initialize()) Serial.println("Sensor found");
  else
  {
    Serial.println("Sensor missing");
    while (1) {}
  }

  // onetime-measure:
  bmp280.setEnabled(0);
  bmp280.triggerMeasurement();
}


void loop() {
  
  bmpTest();

  if (temperature > NORM_TEMP_MAX &&
      temperature < HIGH_TEMP_MAX)
  {
    highTempAnimation();
  }
  else if (temperature > LOW_TEMP_MAX &&
           temperature < NORM_TEMP_MAX )
  {
    normTempAnimation();
  } else if (temperature > LOW_TEMP_MIN &&
             temperature < LOW_TEMP_MAX)
  {
    lowTempAnimation();
  }
  
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND); 
}

void bmpTest() {

  bmp280.awaitMeasurement();
  bmp280.getTemperature(temperature);
  bmp280.getPressure(pascal);
  bmp280.getAltitude(meters);
  metersold = (metersold * 10 + meters) / 11;
  bmp280.triggerMeasurement();

  Serial.print(" HeightPT1: ");
  Serial.print(metersold);
  Serial.print(" m; Height: ");
  Serial.print(meters);
  Serial.print(" Pressure: ");
  Serial.print(pascal);
  Serial.print(" Pa; T: ");
  Serial.print(temperature);
  Serial.println(" C");
}


void highTempAnimation() {

  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

void normTempAnimation() {

  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}
void lowTempAnimation() {

  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}



