/*
Read from a private channel on Adafruit IO

//http://core-electronics.com.au/tutorials/using-neopixels-with-particle.html
*/

/* ======================= INCLUDES ================================= */
#include <application.h>
#include "Adafruit_IO_Client.h"
#include "neopixel.h"
#include "elapsedMillis.h"

TCPClient client;


/* ======================= IO SETUP ================================= */
#define AIO_SERVER      "io.adafruit.com"       // server
#define AIO_SERVERPORT  8883                   // use 8883 for SSL / 1883 not
#define AIO_USERNAME    "you"              // username
#define AIO_KEY         "key"    // adafruit key

Adafruit_IO_Client aio = Adafruit_IO_Client(client, AIO_KEY);
Adafruit_IO_Feed gasVal = aio.getFeed("gasReading"); //gasReading
/* ======================= NEOPATTERNS CLASS ================================= */


/* This class extends Nexpixels to be non-blocking.
 * https://learn.adafruit.com/multi-tasking-the-arduino-part-3/overview
 *
 * DON'T DECLARE A TYPE: this bungs it up for some weird ass reason
 * To reduce NeoPixel burnout risk, add 1000 uF capacitor across pixel power leads
 * Add 300 - 500 Ohm resistor on first pixel's data input
*/

// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
    public:

    // Member Variables:
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern

    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position

    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern

    void (*OnComplete)();  // Callback on completion of pattern

    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin)
    {
        OnComplete = callback;
    }

    // Update the pattern
    void Update()
    {
        if((millis() - lastUpdate) > Interval) // time to update
        {
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case THEATER_CHASE:
                    TheaterChaseUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case SCANNER:
                    ScannerUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                default:
                    break;
            }
        }
    }

    // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index >= TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }

    // Reverse pattern direction
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }

    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        Index = 0;
        Direction = dir;
    }

    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
        Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
   }

    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            if ((i + Index) % 3 == 0)
            {
                setPixelColor(i, Color1);
            }
            else
            {
                setPixelColor(i, Color2);
            }
        }
        show();
        Increment();
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color;
        Index = 0;
        Direction = dir;
    }

    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
        setPixelColor(Index, Color1);
        show();
        Increment();
    }

    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval)
    {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = (numPixels() - 1) * 2;
        Color1 = color1;
        Index = 0;
    }

    // Update the Scanner Pattern
    void ScannerUpdate()
    {
        for (int i = 0; i < numPixels(); i++)
        {
            if (i == Index)  // Scan Pixel to the right
            {
                 setPixelColor(i, Color1);
            }
            else if (i == TotalSteps - Index) // Scan Pixel to the left
            {
                 setPixelColor(i, Color1);
            }
            else // Fading tail
            {
                 setPixelColor(i, DimColor(getPixelColor(i)));
            }
        }
        show();
        Increment();
    }

    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }

    // Update the Fade Pattern
    void FadeUpdate()
    {
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

        ColorSet(Color(red, green, blue));
        show();
        Increment();
    }

    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }

    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
};


/* ======================= NEOPATTERNS ================================= */

NeoPatterns Strip1(6, 2, &Strip1Complete);


/* ======================= VARIABLES ================================= */

int getRate = 10000;         // every 10 seconds
int lastGet;                // last time you retrieved


SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);


int BASE_THRESHOLD = 80;            // base THRESHOLD
int currentReading;
int speed = 2000;   // make a new interval for animation

/* ======================= POOP ARRAY ================================= */

void setup(){
  Serial.begin(115200);
  delay(10);
  Serial.println(); Serial.println();
  Serial.println(F("Adafruit IO Particle test!"));
  aio.begin();
  Serial.println(F("Ready!"));
  Strip1.begin();
  //Strip1.ColorWipe(Strip1.Color(0,255,0), 50);
  Strip1.Fade(Strip1.Color(0,0,0), Strip1.Color(0,255,0), speed, 1);


}

void loop(){

  Strip1.Update();
  int newReading;


  if(millis()-lastGet>=getRate){
    FeedData latest = gasVal.receive();
    lastGet = millis();
    if (latest.isValid()){
      if (latest.intValue(&newReading)){
        if(newReading > BASE_THRESHOLD){
          Serial.print(F("newReading: ")); Serial.println(newReading, DEC);
          if(newReading != currentReading){
            if(newReading < 100){
              speed = 2000;
            } else if(newReading > 100 && newReading < 120){
              speed = 500;
            } else if(newReading > 120 && newReading < 250){
              speed = 200;
            } else if(newReading >250){
              speed = 80;
            }
            //Strip1.ActivePattern = FADE;
            Strip1.Fade(Strip1.Color(0,0,0), Strip1.Color(0,255,0), speed, 1);
            //Strip1.Interval = speed;
            Serial.print(F("!Doing the thing - interval: ")); Serial.println(speed, DEC);
            // reset current reading
            currentReading = newReading;

          } else {
            Serial.println(F("nothing changed. do nothing."));
          }
        }
      }
    }else {
      Serial.println(F("Failed to receive the latest feed value!"));
    }
    Serial.println(F("Waiting 10 seconds"));
  }

}

/* ======================= CALLBACKS ================================= */

void Strip1Complete()
{

  Strip1.Reverse();

}
