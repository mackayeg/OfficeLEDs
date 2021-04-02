#include "FastLED.h"
#define GOOD_ORANGE 0xff5a00
#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

//This code is based on TwinkleFox and adapted for use in my "OfficeLED"
//system. Changes are predominantly addition of pallets and external control.
//Credit to Mark Kriegsman for the 99% of the code contained herein.
//Calling this adaptation "Domesticated Twinkle Fox" because I thought
//it was cute and much like domesticated foxes somtimes it pisses itself
//for no reason. -Akio 2020

//  TwinkleFOX: Twinkling 'holiday' lights that fade in and out.
//  Colors are chosen from a palette; a few palettes are provided.
//
//  This December 2015 implementation improves on the December 2014 version
//  in several ways:
//  - smoother fading, compatible with any colors and any palettes
//  - easier control of twinkle speed and twinkle density
//  - supports an optional 'background color'
//  - takes even less RAM: zero RAM overhead per pixel
//  - illustrates a couple of interesting techniques (uh oh...)
//
//  The idea behind this (new) implementation is that there's one
//  basic, repeating pattern that each pixel follows like a waveform:
//  The brightness rises from 0..255 and then falls back down to 0.
//  The brightness at any given point in time can be determined as
//  as a function of time, for example:
//    brightness = sine( time ); // a sine wave of brightness over time
//
//  So the way this implementation works is that every pixel follows
//  the exact same wave function over time.  In this particular case,
//  I chose a sawtooth triangle wave (triwave8) rather than a sine wave,
//  but the idea is the same: brightness = triwave8( time ).  
//  
//  Of course, if all the pixels used the exact same wave form, and 
//  if they all used the exact same 'clock' for their 'time base', all
//  the pixels would brighten and dim at once -- which does not look
//  like twinkling at all.
//
//  So to achieve random-looking twinkling, each pixel is given a 
//  slightly different 'clock' signal.  Some of the clocks run faster, 
//  some run slower, and each 'clock' also has a random offset from zero.
//  The net result is that the 'clocks' for all the pixels are always out 
//  of sync from each other, producing a nice random distribution
//  of twinkles.
//
//  The 'clock speed adjustment' and 'time offset' for each pixel
//  are generated randomly.  One (normal) approach to implementing that
//  would be to randomly generate the clock parameters for each pixel 
//  at startup, and store them in some arrays.  However, that consumes
//  a great deal of precious RAM, and it turns out to be totally
//  unnessary!  If the random number generate is 'seeded' with the
//  same starting value every time, it will generate the same sequence
//  of values every time.  So the clock adjustment parameters for each
//  pixel are 'stored' in a pseudo-random number generator!  The PRNG 
//  is reset, and then the first numbers out of it are the clock 
//  adjustment parameters for the first pixel, the second numbers out
//  of it are the parameters for the second pixel, and so on.
//  In this way, we can 'store' a stable sequence of thousands of
//  random clock adjustment parameters in literally two bytes of RAM.
//
//  There's a little bit of fixed-point math involved in applying the
//  clock speed adjustments, which are expressed in eighths.  Each pixel's
//  clock speed ranges from 8/8ths of the system clock (i.e. 1x) to
//  23/8ths of the system clock (i.e. nearly 3x).
//
//  On a basic Arduino Uno or Leonardo, this code can twinkle 300+ pixels
//  smoothly at over 50 updates per seond.
//
//  -Mark Kriegsman, December 2015

// CRGBArray<NUM_LEDS> leds;

// Overall twinkle speed.
// 0 (VERY slow) to 8 (VERY fast).  
// 4, 5, and 6 are recommended, default is 4.
//#define TWINKLE_SPEED 3  //rm2020 controlled by "speed" parameter now
// uint8_t twinkle_speed = int(speed/100);

// Overall twinkle density.
// 0 (NONE lit) to 8 (ALL lit at once).  
// Default is 5.
//#define TWINKLE_DENSITY 2  //rm2020 controlled by "length" now

// How often to change color palettes.
#define SECONDS_PER_PALETTE  1  //hijaced for other purposes
// Also: toward the bottom of the file is an array 
// called "ActivePaletteList" which controls which color
// palettes are used; you can add or remove color palettes
// from there freely.

// Background color for 'unlit' pixels
// Can be set to CRGB::Black if desired.
CRGB gBackgroundColor = CRGB::Black; 
// Example of dim incandescent fairy light background color
// CRGB gBackgroundColor = CRGB(CRGB::FairyLight).nscale8_video(16);

// If auto_bg is set to 1,
// then for any palette where the first two entries 
// are the same, a dimmed version of that color will
// automatically be used as the background color.
//#define auto_bg 0
bool auto_bg = 0;

// If COOL_LIKE_INCANDESCENT is set to 1, colors will 
// fade out slighted 'reddened', similar to how
// incandescent bulbs change color as they get dim down.
#define COOL_LIKE_INCANDESCENT 1


CRGBPalette16 gCurrentPalette;
CRGBPalette16 gTargetPalette;

// CRGBPalette16 Red_p;
// // CRGBPalette16 RedOrange_p = purplefly_gp;
// CRGBPalette16 RedOrange_p;
// CRGBPalette16 Orange_p;
// CRGBPalette16 Yellow_p;
// CRGBPalette16 YellowGreen_p;
// CRGBPalette16 Green_p;
// CRGBPalette16 GreenBlue_p;
// CRGBPalette16 Blue_p;
// CRGBPalette16 Indigo_p;
// CRGBPalette16 Violet_p;



void tfsetup() {
  // int temp = 0;
  chooseNextColorPalette(gTargetPalette);
  // fill_solid( Red_p, 16, CRGB::Red);
  // RedOrange_p = purplefly_gp;
  // temp = ColorFromPalette(RainbowColors_p, 8, 255, LINEARBLEND);
  // fill_solid( RedOrange_p, 16, temp);
  // fill_solid( Orange_p, 16, CRGB::Red);
  // fill_solid( Yellow_p, 16, CRGB::Red);
  // fill_solid( YellowGreen_p, 16, CRGB::Red);
  // fill_solid( Green_p, 16, CRGB::Red);
  // fill_solid( GreenBlue_p, 16, CRGB::Red);
  // fill_solid( Blue_p, 16, CRGB::Red);
  // fill_solid( Indigo_p, 16, CRGB::Red);
  // fill_solid( Violet_p, 16, CRGB::Red);
}


void dTwinkleFox(bool bg) {
  quick_serial();
  
  EVERY_N_SECONDS( SECONDS_PER_PALETTE ) {   //TODO
    
    // chooseNextColorPalette( gTargetPalette ); 
    changePalette();
    auto_bg = bg;
  }

  //gTargetPalette = *ActivePaletteList[pattern];
  
  EVERY_N_MILLISECONDS( 10 ) {
    nblendPaletteTowardPalette( gCurrentPalette, gTargetPalette, 12);
  }

  drawTwinkles(leds);
  
  FastLED.show();
}


//  This function loops over each pixel, calculates the 
//  adjusted 'clock' that this pixel should use, and calls 
//  "CalculateOneTwinkle" on each pixel.  It then displays
//  either the twinkle color of the background color, 
//  whichever is brighter.
void drawTwinkles( CRGBSet& L)
{
  // "PRNG16" is the pseudorandom number generator
  // It MUST be reset to the same starting value each time
  // this function is called, so that the sequence of 'random'
  // numbers that it generates is (paradoxically) stable.
  uint16_t PRNG16 = 11337;
  
  uint32_t clock32 = millis();

  // Set up the background color, "bg".
  // if auto_bg == 1, and the first two colors of
  // the current palette are identical, then a deeply faded version of
  // that color is used for the background color
  CRGB bg;
  if( (auto_bg == 1) ) {
    bg = gCurrentPalette[0];
    uint8_t bglight = bg.getAverageLight();
    if( bglight > 64) {
      bg.nscale8_video( 16); // very bright, so scale to 1/16th
    } else if( bglight > 16) {
      bg.nscale8_video( 64); // not that bright, so scale to 1/4th
    } else {
      bg.nscale8_video( 86); // dim, scale to 1/3rd.
    }
  } else {
    bg = gBackgroundColor; // just use the explicitly defined background color
  }

  uint8_t backgroundBrightness = bg.getAverageLight();
  
  for( CRGB& pixel: L) {
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    uint16_t myclockoffset16= PRNG16; // use that number as clock offset
    PRNG16 = (uint16_t)(PRNG16 * 2053) + 1384; // next 'random' number
    // use that number as clock speed adjustment factor (in 8ths, from 8/8ths to 23/8ths)
    uint8_t myspeedmultiplierQ5_3 =  ((((PRNG16 & 0xFF)>>4) + (PRNG16 & 0x0F)) & 0x0F) + 0x08;
    uint32_t myclock30 = (uint32_t)((clock32 * myspeedmultiplierQ5_3) >> 3) + myclockoffset16;
    uint8_t  myunique8 = PRNG16 >> 8; // get 'salt' value for this pixel

    // We now have the adjusted 'clock' for this pixel, now we call
    // the function that computes what color the pixel should be based
    // on the "brightness = f( time )" idea.
    CRGB c = computeOneTwinkle( myclock30, myunique8);

    uint8_t cbright = c.getAverageLight();
    int16_t deltabright = cbright - backgroundBrightness;
    if( deltabright >= 32 || (!bg)) {
      // If the new pixel is significantly brighter than the background color, 
      // use the new color.
      pixel = c;
    } else if( deltabright > 0 ) {
      // If the new pixel is just slightly brighter than the background color,
      // mix a blend of the new color and the background color
      pixel = blend( bg, c, deltabright * 8);
    } else { 
      // if the new pixel is not at all brighter than the background color,
      // just use the background color.
      pixel = bg;
    }
  }
}


//  This function takes a time in pseudo-milliseconds,
//  figures out brightness = f( time ), and also hue = f( time )
//  The 'low digits' of the millisecond time are used as 
//  input to the brightness wave function.  
//  The 'high digits' are used to select a color, so that the color
//  does not change over the course of the fade-in, fade-out
//  of one cycle of the brightness wave function.
//  The 'high digits' are also used to determine whether this pixel
//  should light at all during this cycle, based on the TWINKLE_DENSITY.
CRGB computeOneTwinkle( uint32_t ms, uint8_t salt)
{
  uint16_t ticks = ms >> (10-(speed/100));
  uint8_t fastcycle8 = ticks;
  uint16_t slowcycle16 = (ticks >> 8) + salt;
  slowcycle16 += sin8( slowcycle16);
  slowcycle16 =  (slowcycle16 * 2053) + 1384;
  uint8_t slowcycle8 = (slowcycle16 & 0xFF) + (slowcycle16 >> 8);
  
  uint8_t bright = 0;
  //want len 1 == 1
  //want len 32 == 8 
  uint8_t len = (length/4) + 1;
  if( ((slowcycle8 & 0x0E)/2) < (len)) {//TWINKLE_DENSITY) {
    bright = attackDecayWave8( fastcycle8);
  }

  uint8_t hue = slowcycle8 - salt;
  CRGB c;
  if( bright > 0) {
    c = ColorFromPalette( gCurrentPalette, hue, bright, NOBLEND);
    if( COOL_LIKE_INCANDESCENT == 1 ) {
      coolLikeIncandescent( c, fastcycle8);
    }
  } else {
    c = CRGB::Black;
  }
  return c;
}


// This function is like 'triwave8', which produces a 
// symmetrical up-and-down triangle sawtooth waveform, except that this
// function produces a triangle wave with a faster attack and a slower decay:
//
//     / \ 
//    /     \ 
//   /         \ 
//  /             \ 
//

uint8_t attackDecayWave8( uint8_t i)
{
  if( i < 86) {
    return i * 3;
  } else {
    i -= 86;
    return 255 - (i + (i/2));
  }
}

// This function takes a pixel, and if its in the 'fading down'
// part of the cycle, it adjusts the color a little bit like the 
// way that incandescent bulbs fade toward 'red' as they dim.
void coolLikeIncandescent( CRGB& c, uint8_t phase)
{
  if( phase < 128) return;

  uint8_t cooling = (phase - 128) >> 4;
  c.g = qsub8( c.g, cooling);
  c.b = qsub8( c.b, cooling * 2);
}

// A mostly red palette with green accents and white trim.
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 RedGreenWhite_p FL_PROGMEM =
{  CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
   CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
   CRGB::Red, CRGB::Red, CRGB::Gray, CRGB::Gray, 
   CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green };

// A mostly (dark) green palette with red berries.
#define Holly_Green 0x00580c
#define Holly_Red   0xB00402
const TProgmemRGBPalette16 Holly_p FL_PROGMEM =
{  Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
   Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
   Holly_Green, Holly_Green, Holly_Green, Holly_Green, 
   Holly_Green, Holly_Green, Holly_Green, Holly_Red 
};

// A red and white striped palette
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 RedWhite_p FL_PROGMEM =
{  CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red, 
   CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray,
   CRGB::Red,  CRGB::Red,  CRGB::Red,  CRGB::Red, 
   CRGB::Gray, CRGB::Gray, CRGB::Gray, CRGB::Gray };

// A mostly blue palette with white accents.
// "CRGB::Gray" is used as white to keep the brightness more uniform.
const TProgmemRGBPalette16 BlueWhite_p FL_PROGMEM =
{  CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
   CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
   CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
   CRGB::Blue, CRGB::Gray, CRGB::Gray, CRGB::Gray };

// A pure "fairy light" palette with some brightness variations
#define HALFFAIRY ((CRGB::FairyLight & 0xFEFEFE) / 2)
#define QUARTERFAIRY ((CRGB::FairyLight & 0xFCFCFC) / 4)
const TProgmemRGBPalette16 FairyLight_p FL_PROGMEM =
{  CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, 
   HALFFAIRY,        HALFFAIRY,        CRGB::FairyLight, CRGB::FairyLight, 
   QUARTERFAIRY,     QUARTERFAIRY,     CRGB::FairyLight, CRGB::FairyLight, 
   CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight, CRGB::FairyLight };

// A palette of soft snowflakes with the occasional bright one
const TProgmemRGBPalette16 Snow_p FL_PROGMEM =
{  0x304048, 0x304048, 0x304048, 0x304048,
   0x304048, 0x304048, 0x304048, 0x304048,
   0x304048, 0x304048, 0x304048, 0x304048,
   0x304048, 0x304048, 0x304048, 0xE0F0FF };

// A palette reminiscent of large 'old-school' C9-size tree lights
// in the five classic colors: red, orange, green, blue, and white.
#define C9_Red    0xB80400
#define C9_Orange 0x902C02
#define C9_Green  0x046002
#define C9_Blue   0x070758
#define C9_White  0x606820
const TProgmemRGBPalette16 RetroC9_p FL_PROGMEM =
{  C9_Red,    C9_Orange, C9_Red,    C9_Orange,
   C9_Orange, C9_Red,    C9_Orange, C9_Red,
   C9_Green,  C9_Green,  C9_Green,  C9_Green,
   C9_Blue,   C9_Blue,   C9_Blue,
   C9_White
};

// A cold, icy pale blue palette
#define Ice_Blue1 0x0C1040
#define Ice_Blue2 0x182080
#define Ice_Blue3 0x5080C0
const TProgmemRGBPalette16 Ice_p FL_PROGMEM =
{
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue1, Ice_Blue1, Ice_Blue1, Ice_Blue1,
  Ice_Blue2, Ice_Blue2, Ice_Blue2, Ice_Blue3
};

const TProgmemRGBPalette16 HappyLights_p FL_PROGMEM =
{  CRGB::DarkViolet, CRGB::Cyan, CRGB::Lime, CRGB::Cyan, 
   CRGB::DarkViolet, CRGB::Cyan, CRGB::Lime, CRGB::DarkViolet, 
   CRGB::Red, CRGB::Cyan, CRGB::Lime, CRGB::DarkViolet, 
   CRGB::Lime, CRGB::DarkViolet, CRGB::Cyan, CRGB::Red };

const TProgmemRGBPalette16 FireLights_p FL_PROGMEM =
{  C9_Red, CRGB::Red, CRGB::Red, CRGB::Orange, 
   CRGB::DarkRed, C9_Orange, GOOD_ORANGE, C9_Orange, 
   CRGB::Red, C9_Red, CRGB::Red, CRGB::Orange, 
   CRGB::DarkRed, GOOD_ORANGE, C9_Red, C9_Orange };

const TProgmemRGBPalette16 Night_p FL_PROGMEM =
{  CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
   CRGB::Red, CRGB::Red, CRGB::Red, CRGB::DarkViolet, 
   CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
   CRGB::Red, CRGB::Red, CRGB::Red, GOOD_ORANGE };

/* --- BEGIN SOLID COLOR "PALLETTES" --- */
//This is a crap way to do this, but it keeps it simple
const TProgmemRGBPalette16 Red_p FL_PROGMEM =
{  CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
   CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
   CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 
   CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red };

const TProgmemRGBPalette16 Orange_p FL_PROGMEM =
{
  GOOD_ORANGE, GOOD_ORANGE, GOOD_ORANGE, GOOD_ORANGE,
  GOOD_ORANGE, GOOD_ORANGE, GOOD_ORANGE, GOOD_ORANGE,
  GOOD_ORANGE, GOOD_ORANGE, GOOD_ORANGE, GOOD_ORANGE,
  GOOD_ORANGE, GOOD_ORANGE, GOOD_ORANGE, GOOD_ORANGE
};

const TProgmemRGBPalette16 Yellow_p FL_PROGMEM =
{  CRGB::Yellow, CRGB::Yellow, CRGB::Yellow, CRGB::Yellow, 
   CRGB::Yellow, CRGB::Yellow, CRGB::Yellow, CRGB::Yellow, 
   CRGB::Yellow, CRGB::Yellow, CRGB::Yellow, CRGB::Yellow, 
   CRGB::Yellow, CRGB::Yellow, CRGB::Yellow, CRGB::Yellow };

const TProgmemRGBPalette16 YellowGreen_p FL_PROGMEM =
{
  0x56D500, 0x56D500, 0x56D500, 0x56D500,
  0x56D500, 0x56D500, 0x56D500, 0x56D500,
  0x56D500, 0x56D500, 0x56D500, 0x56D500,
  0x56D500, 0x56D500, 0x56D500, 0x56D500};

const TProgmemRGBPalette16 Green_p FL_PROGMEM =
{  CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green, 
   CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green, 
   CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green, 
   CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green };

const TProgmemRGBPalette16 Aqua_p FL_PROGMEM =
{
  0x00AB55, 0x00AB55, 0x00AB55, 0x00AB55,
  0x00AB55, 0x00AB55, 0x00AB55, 0x00AB55,
  0x00AB55, 0x00AB55, 0x00AB55, 0x00AB55,
  0x00AB55, 0x00AB55, 0x00AB55, 0x00AB55};

const TProgmemRGBPalette16 Blue_p FL_PROGMEM =
{  CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
   CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
   CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue, 
   CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue };

const TProgmemRGBPalette16 Indigo_p FL_PROGMEM =
{
  0x5500AB, 0x5500AB, 0x5500AB, 0x5500AB,
  0x5500AB, 0x5500AB, 0x5500AB, 0x5500AB,
  0x5500AB, 0x5500AB, 0x5500AB, 0x5500AB,
  0x5500AB, 0x5500AB, 0x5500AB, 0x5500AB};

const TProgmemRGBPalette16 Violet_p FL_PROGMEM =
{
  0x800080, 0x800080, 0x800080, 0x800080,
  0x800080, 0x800080, 0x800080, 0x800080,
  0x800080, 0x800080, 0x800080, 0x800080,
  0x800080, 0x800080, 0x800080, 0x800080};

const TProgmemRGBPalette16 Pink_p FL_PROGMEM =
{
  0xD5002B, 0xD5002B, 0xD5002B, 0xD5002B,
  0xD5002B, 0xD5002B, 0xD5002B, 0xD5002B,
  0xD5002B, 0xD5002B, 0xD5002B, 0xD5002B,
  0xD5002B, 0xD5002B, 0xD5002B, 0xD5002B};

const TProgmemRGBPalette16 White_p FL_PROGMEM =
{  CRGB::White, CRGB::White, CRGB::White, CRGB::White, 
   CRGB::White, CRGB::White, CRGB::White, CRGB::White, 
   CRGB::White, CRGB::White, CRGB::White, CRGB::White, 
   CRGB::White, CRGB::White, CRGB::White, CRGB::White };


// Add or remove palette names from this list to control which color
// palettes are used, and in what order.

extern const TProgmemRGBPalette16* ActivePaletteList[] = {
  &Red_p,             //0
  &Orange_p,          //1
  &Yellow_p,          //2
  &YellowGreen_p,     //3 Could be replaced
  &Green_p,
  &Aqua_p,            //5
  &Blue_p,
  // &Indigo_p,
  &Violet_p,          //7 Could be replaced
  &Pink_p,            //8
  &White_p,           //9

  //Good stuff!
  &RainbowColors_p,   //10
  &HappyLights_p,
  &PartyColors_p,
  &RetroC9_p,
  &ForestColors_p,    //14 - Green and white

  //Whites/ blues
  &FairyLight_p,      //15 - Creamy white twinkle
  &Snow_p,            //xx - Cool white
  &OceanColors_p,     //xx - Blue white with more teal
  &Ice_p,             //xx - Chill blue
  &BlueWhite_p,

  //Reds
  &FireLights_p,      //20
  &LavaColors_p,      //xx Red orange and white for some reason (CAN RP)

  //Other
  
  //XMAS
  &RedGreenWhite_p,   //22 - very XMas
  &Holly_p,           //23 - Green with a hint of red
  &RedWhite_p,        //24
  &Night_p            //25
};

void changePalette(void) {
  gTargetPalette = *(ActivePaletteList[pattern]);
  // currentPalette = *ActivePaletteList[pattern];
}


// Advance to the next color palette in the list (above).
void chooseNextColorPalette( CRGBPalette16& pal)
{
  const uint8_t numberOfPalettes = sizeof(ActivePaletteList) / sizeof(ActivePaletteList[0]);
  static uint8_t whichPalette = -1; 
  whichPalette = addmod8( whichPalette, 1, numberOfPalettes);

  pal = *(ActivePaletteList[whichPalette]);
}