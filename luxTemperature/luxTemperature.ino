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
#include <math.h>
BMP280 bmp280;

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define HIGH_TEMP_MAX 30
//#define NORM_TEMP_MAX 24.5 //FIX ME: Just for test
//#define LOW_TEMP_MAX 26.0
#define LOW_TEMP_MIN 18

#define DATA_PIN    12
#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    12
#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  200
#define PI 3.1416
#define HUE_RANGE 110

CRGB leds[NUM_LEDS];
uint8_t gHue = 255;
float temperature;
float pascal;
static float meters, metersold;
static float T_OBOROT_S = 0.5;

#define POSITION(x) ( (((x)*FRAMES_PER_SECOND)/NUM_LEDS) )
#define TEMP_QUANT (( HIGH_TEMP_MAX - LOW_TEMP_MIN )/(float)NUM_LEDS)


void setup() {
  delay(2000);
  
  Serial.begin(9600);
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



void calculateView()
{
  static int pos;
  static int num_on_leds = 0;
   if( temperature < HIGH_TEMP_MAX  )
   {
     pos = 0;
     if( temperature > LOW_TEMP_MIN )
     {
        num_on_leds = (( temperature - LOW_TEMP_MIN)/TEMP_QUANT);
        if( num_on_leds > NUM_LEDS)
        {
           num_on_leds = NUM_LEDS;
           Serial.print(" Greshka pri Temperaturata\n");
        }
        normTempAnimation(num_on_leds, HUE_RANGE );
     }
     else
     {
        Serial.print(" LOW Temperaturata\n");
        extremCaseTempAnimation(  2, HUE_RANGE );
     }
   }
   else
   {
     
     extremCaseTempAnimation(  NUM_LEDS, HUE_RANGE );
   }
}

void loop() {
    
  EVERY_N_MILLISECONDS( 1000 ) {
    bmpTest();
  }

  EVERY_N_MILLISECONDS( 20 ) { 
    //gHue++; 
  } // slowly cycle the "base color" through the rainbow
  
  calculateView();
  
  FastLED.show();
  FastLED.delay( 1000 / FRAMES_PER_SECOND ); 
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

void extremCaseTempAnimation(  int num_on_leds, int hue_range ) {
  int k = hue_range;
  int hue = num_on_leds > 0 ?(hue_range/num_on_leds):0;
  static int ctr = 0;
  static int pos = 0;
  ctr++;
  if( ctr >= POSITION(T_OBOROT_S)  )
  {
     ctr = 0;
     pos = (pos +1)%NUM_LEDS;
  }
  
    if( pos + num_on_leds < NUM_LEDS )
    {
      for( int i=0; i < NUM_LEDS; i++ )
      {
         if( i >= pos && i < (pos + num_on_leds) )
         {
            fill_solid( &leds[i],1, CHSV( k, 255, 192) );
            k -= hue;
         }
         else
         {
            fill_solid( &leds[i],1, CHSV( 0, 0, 0) );
         }
      }
    }
    else
    {
        int i;
        int end = (pos+num_on_leds)%NUM_LEDS;
        for(  i = pos; i < NUM_LEDS; i++ )
        {
              fill_solid( &leds[i],1, CHSV( k, 255, 192) );
              k -= hue;
        }
        
       for(  i=0; i < end; i++ )
        {
              fill_solid( &leds[i],1, CHSV( k, 255, 192) );
              k -= hue;
        }
        for(  i=end; i < pos; i++ )
        {
              fill_solid( &leds[i],1, CHSV( 0, 0, 0) );
        }
    }
    
#if 0 //Validate color space     
    fill_solid( leds, 1,CRGB(255,0,0));
    fill_solid( &leds[1], 1,CRGB(0,255,0));
    fill_solid( &leds[2], 1,CRGB(0,0,255));
#endif
}

void highTempAnimation() {

  fadeToBlackBy( leds, NUM_LEDS, 10);
 // int pos = beatsin16(13, 0, NUM_LEDS);
//  leds[pos] += CHSV( 0, 255, 192);
}

void normTempAnimation( int num_on_leds, int hue_range ) 
{
  if( num_on_leds > 0 && num_on_leds <= NUM_LEDS)
  {
    int hue = (hue_range/NUM_LEDS);
    int k = hue_range;
    for( int i = 0; i < num_on_leds; i++ )
    {
       fill_solid( &leds[i],1, CHSV( k, 255, 192) );
       k -= hue;
    }
    fill_solid( &leds[num_on_leds],(NUM_LEDS - num_on_leds), CHSV( 0, 0, 0) );
  }
}


void lowTempAnimation() {

  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
}



