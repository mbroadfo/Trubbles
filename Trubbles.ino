#include <Servo.h> 
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

class NeoPix : Adafruit_NeoPixel {
  // Class Member Variables - initialized at startup
  int ledPin;     // the DIN pin of the NeoPixels
  long CheckTime; // ms to wait to update the display
  long OnTime;     // number of seconds to run the display
  long neoDisp;   // the NeoPixel Display function
  
  // Current State Variables
  volatile unsigned long previousMillis;  // last time we checked
  volatile unsigned long totalMillis;    //  total ms display has been running
  volatile bool runMode;                 //  whether Neo Display still running
  
  // Constructor - Creates a NeoPix & Initialized Variables & State
  public:
  NeoPix(int pix, int pin, char flags, long check, long on, char neo) {
  nbrPixels = pix;
  ledPin = pin;
  pixFlags = flags;
  pinMode(ledPin, OUTPUT);
  CheckTime = check;
  OnTime = on;
  neoDisp = neo; 
  previousMillis = -1;  // check on creation
  totalMillis = 0;
  runMode = true;   // start running when created
  Pos = 0;
  }

  void Update(unsigned long currentMillis) {
    if(runMode == true and ((currentMillis - previousMillis >= CheckTime) or (previousMillis == -1)) {
      totalMillis += (currentMillis - previousMillis);
      if (totalMillis > OnTime * 1000) {  // if OnTime Exceeded
        self.begin();                     // clear buffer
        self.show();                      // clear display
        runMode = false;                  // stop updating
        previousMillis = 0;
        totalMillis = 0;
        Pos = 0;
        // need to kill instance when time exceeded
      } 
      else {
        switch(neoDisp) {
          case 'colorWipe':
            colorWipe(strip.Color(255, 0, 0)); // Red
  //          colorWipe(strip.Color(0, 255, 0)); // Green
  //          colorWipe(strip.Color(0, 0, 255)); // Blue
            break;
          case 'rainbow':
            rainbow();
            break;
          case 'rainbowCycle':
            rainbowCycle();
            break;
          case theaterChaseRainbow
            theaterChaseRainbow();
            break;
        }
    
      previousMillis = currentMillis;  // Remember the time
    }
  }

  void colorWipe(uint32_t c) {
  // Fill the dots one after the other with a color    
    if Pos <= self.numPixels() {
      self.setPixelColor(Pos, c);
      self.show();
      Pos++;          
    }
  }
  
  void rainbow() {
    uint16_t i;
    if Pos < 256 {
      for(i=0; i<self.numPixels(); i++) {
        self.setPixelColor(i, Wheel((i+Pos) & 255));
      }
      self.show();
      Pos++;    
    }
  }
  
  // Slightly different, this makes the rainbow equally distributed throughout
  void rainbowCycle() {
    uint16_t i;
    if Pos < 256*5 {
      Pos++;
      for(i=0; i< self.numPixels(); i++) {
        self.setPixelColor(i, Wheel(((i * 256 / self.numPixels()) + Pos) & 255));
      }
      self.show();
    }
  }
  
  //Theatre-style crawling lights with rainbow effect
  void theaterChaseRainbow() {
    for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
      for (int q=0; q < 3; q++) {
        for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
          self.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        self.show();
  
//        delay(wait);
  
        for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
          self.setPixelColor(i+q, 0);        //turn every third pixel off
        }
      }
    }
  }
  
  // Input a value 0 to 255 to get a color value.
  // The colours are a transition r - g - b - back to r.
  uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
      return self.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if(WheelPos < 170) {
      WheelPos -= 85;
      return self.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return self.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

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

NeoPix strip1(35, 5, NEO_GRB + NEO_KHZ800, 50, 5,"colorWipe");

// Setup
void setup() { 
  sweeper1.Attach(8);
  sweeper2.Attach(9);
  
  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(0, Reset, FALLING);

  strip1.begin();
  strip1.show();
} 
 
void Reset(){
  sweeper1.reset();
  sweeper2.reset();
}

// Interrupt is called once a millisecond, 
SIGNAL(TIMER0_COMPA_vect) {
  unsigned long currentMillis = millis();
  sweeper1.Update(currentMillis);
  sweeper2.Update(currentMillis);
  led1.Update(currentMillis);
  led2.Update(currentMillis);
  strip1.Update(currentMillis);
} 

void loop() {
}
