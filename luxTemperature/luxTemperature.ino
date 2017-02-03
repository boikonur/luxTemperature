/*
   Required libraries:
   
   - I2C-Sensor-Lib (iLib)
   - FastLed

*/

#include <Wire.h>
#include "i2c.h"
#include "i2c_BMP280.h"
BMP280 bmp280;


#include "FastLED.h"



FASTLED_USING_NAMESPACE
#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    12 //TODO change

#define LED_TYPE    WS2812
#define COLOR_ORDER RGB
#define NUM_LEDS    12
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120

uint8_t gHue = 0;

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

  FastLED.show();
  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }
  EVERY_N_SECONDS( 10 ) {
    sinelon();  // change patterns periodically
  }

}

void bmpTest() {

  bmp280.awaitMeasurement();

  float temperature;
  bmp280.getTemperature(temperature);

  float pascal;
  bmp280.getPressure(pascal);

  static float meters, metersold;
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

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}

