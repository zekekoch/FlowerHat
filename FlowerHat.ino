#include "FastSPI_LED2.h"
 
#define DATA_PIN 3
#define NUM_LEDS 7
CRGB leds[NUM_LEDS];
 
 
// Anti-aliased light bar example
//   v1 by Mark Kriegsman <kriegsman@tr.org>, November 29, 2013
//
// This example shows the basics of using variable pixel brightness
// as a form of anti-aliasing to animate effects on a sub-pixel level,
// which is useful for effects you want to be particularly "smooth".
//
// This animation shows two bars looping around an LED strip, one moving
// in blocky whole-pixel "integer" steps, and the other one moving
// by smoothly anti-aliased "fractional" (1/16th pixel) steps.
// The use of "16ths" (vs, say, 10ths) is totally arbitrary, but 16ths are
// a good balance of data size, expressive range, and code size and speed.
//
// Notionally, "I" is the Integer Bar, "F" is the Fractional Bar.
 
int     Ipos   = NUM_LEDS / 2; // position of the "integer-based bar"
int     Idelta = 1; // how many pixels to move the Integer Bar
uint8_t Ihue = 10; // color for Integer Bar
 
int     F16pos = 0; // position of the "fraction-based bar"
int     F16delta = 1; // how many 16ths of a pixel to move the Fractional Bar
uint8_t Fhue = 10; // color for Fractional Bar
 
int Width  = 3; // width of each light bar, in whole pixels
 
int InterframeDelay = 10; //ms
  
void setup() {
  delay(1000); // setup guard
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(128);
}
 
// Draw an "Integer Bar" of light starting at pixel 'pos', with given
// width (in whole pixels) and hue.
// This is not the interesting code.
void drawIntegerBar( int intpos, int width, uint8_t hue)
{
  int i = intpos; // start drawing at "I"
  for( int n = 0; n < width; n++) {
    leds[i] += CHSV( hue, 255, 255);
    i++;
    if( i == NUM_LEDS) i = 0; // wrap around
  }
}
 
 
// Draw a "Fractional Bar" of light starting at position 'pos16', which is counted in
// sixteenths of a pixel from the start of the strip.  Fractional positions are
// rendered using 'anti-aliasing' of pixel brightness.
// The bar width is specified in whole pixels.
// Arguably, this is the interesting code.
void drawFractionalBar( int pos16, int width, uint8_t hue)
{
  int i = pos16 / 16; // convert from pos to raw pixel number
  uint8_t frac = pos16 & 0x0F; // extract the 'factional' part of the position
 
  // brightness of the first pixel in the bar is 1.0 - (fractional part of position)
  // e.g., if the light bar starts drawing at pixel "57.9", then
  // pixel #57 should only be lit at 10% brightness, because only 1/10th of it
  // is "in" the light bar:
  //
  //                       57.9 . . . . . . . . . . . . . . . . . 61.9
  //                        v                                      v
  //  ---+---56----+---57----+---58----+---59----+---60----+---61----+---62---->
  //     |         |        X|XXXXXXXXX|XXXXXXXXX|XXXXXXXXX|XXXXXXXX |  
  //  ---+---------+---------+---------+---------+---------+---------+--------->
  //                   10%       100%      100%      100%      90%        
  //
  // the fraction we get is in 16ths and needs to be converted to 256ths,
  // so we multiply by 16.  We subtract from 255 because we want a high
  // fraction (e.g. 0.9) to turn into a low brightness (e.g. 0.1)
  uint8_t firstpixelbrightness = 255 - (frac * 16);
 
  // if the bar is of integer length, the last pixel's brightness is the
  // reverse of the first pixel's; see illustration above.
  uint8_t lastpixelbrightness  = 255 - firstpixelbrightness;
 
  // For a bar of width "N", the code has to consider "N+1" pixel positions,
  // which is why the "<= width" below instead of "< width".
 
  uint8_t bright;
  for( int n = 0; n <= width; n++) {
    if( n == 0) {
      // first pixel in the bar
      bright = firstpixelbrightness;
    } else if( n == width ) {
      // last pixel in the bar
      bright = lastpixelbrightness;
    } else {
      // middle pixels
      bright = 255;
    }
   
    leds[i] += CHSV( hue, 255, bright);
    i++;
    if( i == NUM_LEDS) i = 0; // wrap around
  }
}

byte nextHue()
{
  static byte hue = 0;
  if (hue > 30)
    hue == 0;
    
  hue++;    
  return hue;
}
 
 
void loop()
{
  // Update the "Fraction Bar" by 1/16th pixel every time
  F16pos += F16delta;
 
  // wrap around at end
  // remember that F16pos contains position in "16ths of a pixel"
  // so the 'end of the strip' is (NUM_LEDS * 16)
  if( F16pos >= (NUM_LEDS * 16)) {
    F16pos -= (NUM_LEDS * 16);
  }
   
  // For this demo, we want the Integer Bar and the Fraciton Bar
  // to move at the same speed down the strip.
  // The Fraction Bar moves 1/16th of a pixel each time through the
  // loop, so to get the same speed on the strip for the Integer Bar,
  // we need to move it by 1 full pixel -- but only every 16 times
  // through the loop.  'countdown' is used to tell when it's time
  // to advance the Integer Bar position again.
  static byte countdown = 0;
  if( countdown == 0) {
    countdown = 16; // reset countdown
     
    // advance Integer Bar one full pixel now
    Ipos += 1;
    // wrap around at end
    if( Ipos >= NUM_LEDS) {
      Ipos -= NUM_LEDS;  
    }
   }
   // countdown is decremented every time through the loop
   countdown -= 1;
   
   // Draw everything:
   // clear the pixel buffer
   memset8( leds, 0, NUM_LEDS * sizeof(CRGB));
   // draw the Integer Bar, length=4px, hue=180
   //drawIntegerBar( Ipos, Width, Ihue);
   // draw the Fractional Bar, length=4px, hue=180
   drawFractionalBar( F16pos, Width, nextHue());
   
   FastLED.show();
   delay(InterframeDelay);
}
