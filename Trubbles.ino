#include <Servo.h> 
#include <Adafruit_NeoPixel.h>

// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel {
    public:
    // Member Variables:  
    pattern  ActivePattern;   // which pattern is running
    direction Direction;      // direction to run the pattern
    uint16_t duration;        // duration to run the pattern    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    volatile unsigned long totalMillis;     //  total ms display has been running
    bool runMode;             // Whether we are running the pattern or not
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;      // total number of steps in the pattern
    uint16_t Index;           // current step within the pattern    
    void (*OnComplete)();     // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)(), uint8_t dur)
    :Adafruit_NeoPixel(pixels, pin, type) {
        OnComplete = callback;
        dur = duration;
    }
    
    // Update the pattern
    void Update() {

        if(runMode = true and (millis() - lastUpdate) > Interval) {    // time to update
            totalMillis += (millis() - lastUpdate);
            if ((totalMillis > duration * 1000) and duration == -1) {
              runMode = false;
            }
        }
        else {
            lastUpdate = millis();
            switch(ActivePattern) {
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
    void Increment() {
        if (Direction == FORWARD) {
           Index++;
           if (Index >= TotalSteps) {
                Index = 0;
                if (OnComplete != NULL) {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
        else {    // Direction == REVERSE
            --Index;
            if (Index <= 0) {
                Index = TotalSteps-1;
                if (OnComplete != NULL) {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }
    
    // Reverse pattern direction
    void Reverse() {
        if (Direction == FORWARD) {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else {
            Direction = FORWARD;
            Index = 0;
        }
    }
    
    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD) {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate() {
        for(int i=0; i< numPixels(); i++) {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
        Increment();
    }

    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD) {
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
   }
    
    // Update the Theater Chase Pattern
    void TheaterChaseUpdate() {
        for(int i=0; i< numPixels(); i++) {
            if ((i + Index) % 3 == 0) {
                setPixelColor(i, Color1);
            }
            else {
                setPixelColor(i, Color2);
            }
        }
        show();
        Increment();
    }

    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD) {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Color Wipe Pattern
    void ColorWipeUpdate() {
        setPixelColor(Index, Color1);
        show();
        Increment();
    }
    
    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval) {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = (numPixels() - 1) * 2;
        Color1 = color1;
        Index = 0;
    }

    // Update the Scanner Pattern
    void ScannerUpdate() { 
        for (int i = 0; i < numPixels(); i++) {
            if (i == Index) {      // Scan Pixel to the right
                 setPixelColor(i, Color1);
            }
            else if (i == TotalSteps - Index)  {    // Scan Pixel to the left
                 setPixelColor(i, Color1);
            }
            else {      // Fading tail
                 setPixelColor(i, DimColor(getPixelColor(i)));
            }
        }
        show();
        Increment();
    }
    
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD) {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Fade Pattern
    void FadeUpdate() {
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
    uint32_t DimColor(uint32_t color) {
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color) {
        for (int i = 0; i < numPixels(); i++) {
            setPixelColor(i, color);
        }
        show();
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color) {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color) {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color) {
        return color & 0xFF;
    }
    
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos) {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85) {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170) {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
};
// ####################################################################################
// ####################################################################################
class NeoPix : public Adafruit_NeoPixel {
  // Class Member Variables - initialized at startup
  long CheckTime;  // ms between updates
  long OnTime;     // Total Display Time
  String neoDisp;   // Display Type  
  // Current State Variables
  volatile unsigned long previousMillis;  // last time we checked
  volatile unsigned long totalMillis;     //  total ms display has been running
  volatile bool runMode;                  //  whether Neo Display still running
  volatile int x,y;                       // generic counters within NeoPix
  
  // Constructor - Creates a NeoPix & Initialized Variables & State
  public:
  NeoPix(uint16_t pix, uint8_t pin, uint8_t nflag) : Adafruit_NeoPixel(pix, pin, nflag) {
    pinMode(pin, OUTPUT);
   runMode = false;   // start running when created
  }

  void kickOff(long currMs, long ctime, long otime, String ndisp) {
    previousMillis = currMs; // Current ms Timer
    CheckTime = ctime;       // ms between updates
    OnTime = otime;          // Total Display Time in sec
    neoDisp = ndisp;         // Display Type
    totalMillis = 0;         // Clear Total Display Timer
    x = 0;                   // Generic Counter
    y = 0;                   // 2nd Generic Counter
    runMode = true;          // Turn It On
  }

  void turnOff() {
    for (int i = 0; i<numPixels(); i++) {
      setPixelColor(i,Color(0,0,0)); // Initialize Pixels
    }
    show();               // Clear Display
    runMode = false;      // Turn Off NeoPix Object
  }

  void Update(unsigned long currentMillis) {

    if( runMode == true and (currentMillis - previousMillis >= CheckTime) ) {
      totalMillis += (currentMillis - previousMillis);
      if ( totalMillis > (OnTime * 1000) ) {  // if OnTime Exceeded
        turnOff();                        // turn off display
      }
      else {
        if(neoDisp == "colorWipe") {
          colorWipe(Color(255, 0, 0)); // Red
//          colorWipe(Color(0, 255, 0)); // Green
//          colorWipe(Color(0, 0, 255)); // Blue
        }
        else if(neoDisp == "rainbow") {
          rainbow();
        }
        else if(neoDisp == "rainbowCycle") {
          rainbowCycle();
        }
        previousMillis = currentMillis;  // Remember the time
      }
    }
  }

  void colorWipe(uint32_t c) {
  // Fill the dots one after the other with a color    
    if (x <= numPixels()) {
      setPixelColor(x, c);
      show();
      x++;          
    }
  }
  
  void rainbow() {
    uint16_t i;
    if (x < 256) {
      for(i=0; i<numPixels(); i++) {
        setPixelColor(i, Wheel((i+x) & 255));
      }
      show();
      x++;    
    }
  }
  
  // Slightly different, this makes the rainbow equally distributed throughout
  void rainbowCycle() {
    uint16_t i;
    if (x < 256*5) {
      for(i=0; i< numPixels(); i++) {
        setPixelColor(i, Wheel(((i * 256 / numPixels()) + x) & 255));
      }
      show();
      x++;
    }
  }
  
  // Input a value 0 to 255 to get a color value.
  // The colours are a transition r - g - b - back to r.
  uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
      return Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if(WheelPos < 170) {
      WheelPos -= 85;
      return Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
};

class Flasher
{
  // Class Member Variables
  // These are initialized at startup
  int ledPin;      // the number of the LED pin
  long OnTime;     // milliseconds of on-time
  long OffTime;    // milliseconds of off-time

  // These maintain the current state
  volatile int ledState;                // ledState used to set the LED
  volatile unsigned long previousMillis;    // will store last time LED was updated

  // Constructor - creates a Flasher 
  // and initializes the member variables and state
  public:
  Flasher(int pin, long on, long off)
  {
  ledPin = pin;
  pinMode(ledPin, OUTPUT);     
    
  OnTime = on;
  OffTime = off;
  
  ledState = LOW; 
  previousMillis = 0;
  }

  void Update(unsigned long currentMillis)
  {
    if((ledState == HIGH) && (currentMillis - previousMillis >= OnTime))
    {
      ledState = LOW;  // Turn it off
      previousMillis = currentMillis;  // Remember the time
      digitalWrite(ledPin, ledState);  // Update the actual LED
    }
    else if ((ledState == LOW) && (currentMillis - previousMillis >= OffTime))
    {
      ledState = HIGH;  // turn it on
      previousMillis = currentMillis;   // Remember the time
      digitalWrite(ledPin, ledState);   // Update the actual LED
    }
  }
};

class Sweeper
{
  Servo servo;              // the servo
  int  updateInterval;      // interval between updates
  
  volatile int pos;                  // current servo position 
  volatile unsigned long lastUpdate; // last update of position
  volatile int increment;            // increment to move for each interval

public: 
  Sweeper(int interval)
  {
    updateInterval = interval;
    increment = 1;
  }
  
  void Attach(int pin)
  {
    servo.attach(pin);
  }
  
  void Detach()
  {
    servo.detach();
  }
  
  void reset()
  {
    pos = 0;
    servo.write(pos);
    increment = abs(increment);
  }
  
  void Update(unsigned long currentMillis)
  {
    if((currentMillis - lastUpdate) > updateInterval)  // time to update
    {
      lastUpdate = currentMillis;
      pos += increment;
      servo.write(pos);
      if ((pos >= 180) || (pos <= 0)) // end of sweep
      {
        // reverse direction
        increment = -increment;
      }
    }
  }
};

// Create Action Objects 
Flasher led1(6, 123, 400);
Flasher led2(7, 350, 350);

Sweeper sweeper1(4);
Sweeper sweeper2(6);

// Setup NeoPixels
void Thing1Complete();
void Thing2Complete();
void Thing3Complete();
void Thing4Complete();
NeoPatterns Thing1(35, 10, NEO_GRB + NEO_KHZ800, &Thing1Complete,10);
NeoPatterns Thing2(12, 3, NEO_GRB + NEO_KHZ800, &Thing2Complete,10);
NeoPatterns Thing3(8, 4, NEO_GRB + NEO_KHZ800, &Thing3Complete,10);
NeoPatterns Thing4(8, 5, NEO_GRB + NEO_KHZ800, &Thing4Complete,10);
//NeoPix strip1(35, 10, NEO_GRB + NEO_KHZ800);
//NeoPix strip2(12, 3, NEO_GRB + NEO_KHZ800);
//NeoPix strip3(8, 4, NEO_GRB + NEO_KHZ800);
//NeoPix strip4(8, 5, NEO_GRB + NEO_KHZ800);

int but1,but2;

// Setup
void setup() { 
  Serial.begin(115200);
  // Setup Buttons
  but1 = 2;
  but2 = 11;
  pinMode(but1, INPUT_PULLUP);
  pinMode(but2, INPUT_PULLUP);

  // Attach the Servos
  sweeper1.Attach(8);
  sweeper2.Attach(9);

  // Initialize all the Things
  Thing1.begin();
  Thing2.begin();
  Thing3.begin();
  Thing4.begin();

  // Kick off a pattern
  Thing1.TheaterChase(Thing1.Color(255,255,0), Thing1.Color(0,0,50), 100);
  Thing2.RainbowCycle(3);
  Thing2.Color1 = Thing1.Color1;
  Thing3.Scanner(Thing1.Color(255,0,0), 55);
  Thing4.ColorWipe(Thing2.Color(0,150,150), 25);
  // Initialize the Strips
  //strip1.begin();
  //strip1.show();
  //strip2.begin();
  //strip2.show();
  //strip3.begin();
  //strip3.show();
  //strip4.begin();
  //strip4.show();
} 
 
void Reset(){
  sweeper1.reset();
  sweeper2.reset();
}

void loop() {
  unsigned long currentMillis = millis();
  sweeper1.Update(currentMillis);
  sweeper2.Update(currentMillis);
  led1.Update(currentMillis);
  led2.Update(currentMillis);

//  if(digitalRead(2) != HIGH) {
//    strip1.kickOff(currentMillis, 4, 5, "rainbowCycle");
//    strip2.kickOff(currentMillis, 3, 10, "rainbowCycle");
//    strip3.kickOff(currentMillis, 2, 15, "rainbowCycle");
//    strip4.kickOff(currentMillis, 1, 20, "rainbowCycle");
//  }  
//  strip1.Update(currentMillis);
//  strip2.Update(currentMillis);
//  strip3.Update(currentMillis);
//  strip4.Update(currentMillis);

    // Switch patterns on a button press:
    if (digitalRead(but1) == LOW) // Button #1 pressed
    {
        // Switch Thing1 to FADE pattern
        Thing1.ActivePattern = FADE;
        Thing1.Interval = 20;
        // Speed up the rainbow on Thing2
        Thing2.Interval = 0;
        // Set Thing3 to all red
        Thing3.ColorSet(Thing3.Color(255, 0, 0));
    }
    else if (digitalRead(but2) == LOW) // Button #2 pressed
    {
        // Switch to alternating color wipes on Rings1 and 2
        Thing1.ActivePattern = COLOR_WIPE;
        Thing2.ActivePattern = COLOR_WIPE;
        Thing2.TotalSteps = Thing2.numPixels();
    }
    else // Back to normal operation
    {
        // Restore all pattern parameters to normal values
        Thing1.ActivePattern = THEATER_CHASE;
        Thing1.Interval = 100;
        Thing2.ActivePattern = RAINBOW_CYCLE;
        Thing2.TotalSteps = 255;
        Thing2.Interval = min(10, Thing2.Interval);
    }    
    // Update the things.
    Thing1.Update();
    Thing2.Update();    
    Thing3.Update();    
    Thing4.Update();    
    

}

//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------

// Thing1 Completion Callback
void Thing1Complete()
{
    if (digitalRead(9) == LOW)  // Button #2 pressed
    {
        // Alternate color-wipe patterns with Thing2
        Thing2.Interval = 40;
        Thing1.Color1 = Thing1.Wheel(random(255));
        Thing1.Interval = 20000;
    }
    else  // Retrn to normal
    {
      Thing1.Reverse();
    }
}

// Ring 2 Completion Callback
void Thing2Complete()
{
    if (digitalRead(9) == LOW)  // Button #2 pressed
    {
        // Alternate color-wipe patterns with Thing1
        Thing1.Interval = 20;
        Thing2.Color1 = Thing2.Wheel(random(255));
        Thing2.Interval = 20000;
    }
    else  // Retrn to normal
    {
        Thing2.RainbowCycle(random(0,10));
    }
}

// Thing3 Completion Callback
void Thing3Complete()
{
    // Random color change for next scan
    Thing3.Color1 = Thing3.Wheel(random(255));
}

// Thing4 Completion Callback
void Thing4Complete()
{
    // Random color change for next scan
    Thing4.Color1 = Thing4.Wheel(random(255));
}


