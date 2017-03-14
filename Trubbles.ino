#include <Servo.h> 
#include <Adafruit_NeoPixel.h>

// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE , PATRIOT};
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

// ################################################################
// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel {
    public:
    // Member Variables:  
    pattern  ActivePattern;               // which pattern is running
    direction Direction;                  // direction to run the pattern
    uint16_t Duration;                    // duration to run the pattern    
    unsigned long Interval;               // milliseconds between updates
    volatile unsigned long lastUpdate;    // last update of position
    volatile unsigned long currentMillis; // current ms counter
    volatile unsigned long totalMillis;   // total ms display has been running
    bool runMode;                         // Whether we are running the pattern or not
    uint32_t Color1, Color2;              // What colors are in use
    int DispLength;                       // Length of Display Pixels in PATRIOT
    uint16_t TotalSteps;                  // total number of steps in the pattern
    uint16_t Index;                       // current step within the pattern    
    void (*OnComplete)();                 // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type) {
        runMode = false;
        OnComplete = callback;
    }
    
    // Update the pattern
    void Update() {
        currentMillis = millis();
        // Is it time to update yet?
        if(runMode == true and (currentMillis - lastUpdate) > Interval) {    // time to update
            totalMillis += (currentMillis - lastUpdate);
            if (totalMillis > (Duration * 1000)) {
              endDisp();
            }
            else {
                lastUpdate = currentMillis;
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
                    case PATRIOT:
                        PatriotUpdate();
                        break;
                    default:
                        break;
                }
            }
        }
    }
  
    // Start the Display
    void startDisp(uint8_t dur) {
      Duration = dur;
      runMode = true;
      lastUpdate = millis();
      totalMillis = 0;
//      Serial.println("Start Display at "+String(lastUpdate)+" for "+String(Duration)+" secs");
    }
    
    // End the Display
    void endDisp() {
        runMode = false;
        for(int i=0; i< numPixels(); i++) {
            setPixelColor(i, Color(0,0,0));
        }
        show();
//      Serial.println("Ended Display");
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

    // Initialize for a Patriot Display
    void Patriot(uint8_t dlen, uint8_t interval, direction dir = FORWARD) {
        ActivePattern = PATRIOT;
        DispLength = dlen;        // Length of the Display
        Interval = interval;      // ms to check for updates
        TotalSteps = numPixels(); 
        Index = 0;
        Direction = dir;
   }
    
    // Update the Theater Chase Pattern
    void PatriotUpdate() {
        for(int i=0; i< numPixels(); i++) {
            int idx = (i+Index) / DispLength;
            if ((idx) % 3 == 0) {
                setPixelColor(i, Color(255,0,0)); // Red
            }
            else if ((idx-1) % 3 == 0) {
                setPixelColor(i,Color(255,255,255));  // White
            }
            else {
                setPixelColor(i, Color(0,0,255)); // Blue
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
// ################################################################
class Flasher {
  public:
  int ledPin;      // the number of the LED pin
  long OnTime;     // milliseconds of on-time
  long OffTime;    // milliseconds of off-time
  int Duration;    // number of seconds to flash
  bool runMode;    // Whether the Flasher is running or not

  // These maintain the current state
  volatile int ledState;                     // ledState used to set the LED
  volatile unsigned long lastUpdate;    // last update of position
  volatile unsigned long currentMillis; // current ms counter
  volatile unsigned long totalMillis;   // total ms display has been running

  // Constructor - creates a Flasher and initializes the member variables and state
  public:
  Flasher(int pin, long on, long off) {
    ledPin = pin;
    pinMode(ledPin, OUTPUT);     
    OnTime = on;
    OffTime = off;
    ledState = LOW; 
    lastUpdate = 0;
    runMode = false;
  }

  void Update() {
      currentMillis = millis();
      
      if(runMode == true and ledState == HIGH and (currentMillis - lastUpdate) > OnTime) { 
          totalMillis += (currentMillis - lastUpdate);
          if (totalMillis > (Duration * 1000)) {
            endDisp();
          }
          else {
            lastUpdate = currentMillis;
            if(ledState == HIGH) {
              ledState = LOW;  // Turn it off
              lastUpdate = currentMillis;  // Remember the time
              digitalWrite(ledPin, ledState);  // Update the actual LED
            }
          }
      }
      else if(runMode == true and ledState == LOW and (currentMillis - lastUpdate) > OffTime) {  
          totalMillis += (currentMillis - lastUpdate);
          if (totalMillis > (Duration * 1000)) {
            endDisp();
          }
          else {
            lastUpdate = currentMillis;
            if (ledState == LOW) {
              ledState = HIGH;  // turn it on
              lastUpdate = currentMillis;   // Remember the time
              digitalWrite(ledPin, ledState);   // Update the actual LED
            }
          }
      }
  }

  // Start the Flasher
  void startDisp(uint8_t dur) {
    Duration = dur;
    runMode = true;
    lastUpdate = millis();
    totalMillis = 0;
    ledState = HIGH;    // turn it on
//    Serial.println("Start Flasher at "+String(lastUpdate)+" for "+String(Duration)+" secs");
  }
  
  // End the Flasher
  void endDisp() {
    runMode = false;
    ledState = LOW;  // turn it off
    digitalWrite(ledPin, ledState);   // Update the actual LED
//    Serial.println("Ended Flasher");
  }
};

// ################################################################
class Sweeper {
  public:
  Servo servo;              // the servo
  int  updateInterval;      // interval between updates
  int Duration;             // number of seconds to sweep
  bool runMode;             // Whether we are running the Sweeper or not
  
  volatile int pos;                       // current servo position 
  volatile unsigned long lastUpdate;      // last update of position
  volatile int increment;                 // increment to move for each interval
  volatile unsigned long currentMillis;   // will store the current ms
  volatile unsigned long totalMillis;     // will store the total ms run so far

public: 
  Sweeper(int interval) {
    updateInterval = interval;
    increment = 1;
    runMode = false;
  }
  
  void Attach(int pin) {
    servo.attach(pin);
  }
  
  void Detach() {
    servo.detach();
  }
  
  void reset() {
    pos = 0;
    servo.write(pos);
    increment = abs(increment);
  }
  
  void Update() {
      currentMillis = millis();
      // Is it time to update yet?
      if(runMode == true and (currentMillis - lastUpdate) > updateInterval) {  
          totalMillis += (currentMillis - lastUpdate);
          if (totalMillis > (Duration * 1000)) {
            endDisp();
          }
          else {
            lastUpdate = currentMillis;
            pos += increment;
            servo.write(pos);
            if ((pos >= 180) || (pos <= 0)) {   // end of sweep = reverse direction
              increment = -increment;
            }
          }
      }
  }
  
  // Start the Sweeper
  void startDisp(uint8_t dur) {
    Duration = dur;
    runMode = true;
    lastUpdate = millis();
    totalMillis = 0;
//    Serial.println("Start Sweeper at "+String(lastUpdate)+" for "+String(Duration)+" secs");
  }
  
  // End the Sweeper
  void endDisp() {
      runMode = false;
      reset();
//      Serial.println("Ended Sweeper");
  }
};

// ################################################################
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
NeoPatterns Thing1(35, 10, NEO_GRB + NEO_KHZ800, &Thing1Complete);
NeoPatterns Thing2(12, 3, NEO_GRB + NEO_KHZ800, &Thing2Complete);
NeoPatterns Thing3(8, 4, NEO_GRB + NEO_KHZ800, &Thing3Complete);
NeoPatterns Thing4(8, 5, NEO_GRB + NEO_KHZ800, &Thing4Complete);

int but1,but2;

// ################################################################
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

  // Set Up a pattern
  Thing1.TheaterChase(Thing1.Color(255,255,0), Thing1.Color(0,0,50), 100);
  Thing2.RainbowCycle(3);
  Thing2.Color1 = Thing1.Color1;
  Thing3.Scanner(Thing1.Color(255,0,0), 55);
  Thing4.Patriot(3,100);

  Thing1.endDisp();
  Thing2.endDisp();
  Thing3.endDisp();
  Thing4.endDisp();
} 
 
void Reset(){
  sweeper1.reset();
  sweeper2.reset();
}

void loop() {
    if (Serial.available()) {
      char rx = Serial.read();
      
      if      (rx == 'a') {Thing1.startDisp(5);}
      else if (rx == 'b') {Thing2.startDisp(8);}
      else if (rx == 'c') {Thing3.startDisp(9);}
      else if (rx == 'd') {Thing4.startDisp(7);}
      else if (rx == 'e') {sweeper1.startDisp(4);}
      else if (rx == 'f') {sweeper2.startDisp(6);}
      else if (rx == 'g') {led1.startDisp(8);}
      else if (rx == 'h') {led2.startDisp(14);}
    }

    if (digitalRead(but1) == LOW) {
      Thing1.startDisp(2);
      Thing2.startDisp(2);
      Thing3.startDisp(2);
      Thing4.startDisp(2);      
    }

    if (digitalRead(but2) == LOW) {
      sweeper1.startDisp(2);
      sweeper2.startDisp(2);
      led1.startDisp(2);
      led2.startDisp(2);      
    }

    // Update the things.
    Thing1.Update();
    Thing2.Update();
    Thing3.Update();
    Thing4.Update();
    sweeper1.Update();
    sweeper2.Update();
    led1.Update();
    led2.Update();
}

//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------

// Thing1 Completion Callback
void Thing1Complete()
{

      Thing1.Color1 = Thing1.Wheel(random(255));
      Thing1.Color2 = Thing1.Wheel(random(255));

//    if (digitalRead(9) == LOW)  // Button #2 pressed
//    {
//        // Alternate color-wipe patterns with Thing2
//        Thing2.Interval = 40;
//        Thing1.Color1 = Thing1.Wheel(random(255));
//        Thing1.Interval = 20000;
//    }
//    else  // Retrn to normal
//    {
      Thing1.Reverse();
//    }
}

// Ring 2 Completion Callback
void Thing2Complete()
{
//    if (digitalRead(9) == LOW)  // Button #2 pressed
//    {
//        // Alternate color-wipe patterns with Thing1
//        Thing1.Interval = 20;
//        Thing2.Color1 = Thing2.Wheel(random(255));
//        Thing2.Interval = 20000;
//    }
//    else  // Retrn to normal
//    {
        Thing2.Reverse();
//    }
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
//    Thing4.Reverse();
}


