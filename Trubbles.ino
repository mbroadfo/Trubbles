#include <Servo.h> 
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

class NeoPix
{
  // Class Member Variables - initialized at startup
  int ledPin;     // the DIN pin of the NeoPixels
  long CheckTime; // ms to wait to update the display
  long OnTime     // number of seconds to run the display
  long neoDisp;   // the NeoPixel Display function
  
  // Current State Variables
  volatile unsigned long previousMillis;  // last time we checked
  volatile unsigned long totalMillis;    //  total ms display has been running
  volatile bool runMode;                 //  whether Neo Display still running
  
  // Constructor - Creates a NeoPix & Initialized Variables & State
  public:
  NeoPix(int pin, long check, long on, string neo) {
  ledPin = pin;
  pinMode(ledPin, OUTPUT);
  CheckTime = check;
  OnTime = on;
  neoDisp = neo; 
  previousMillis = 0;
  totalMillis = 0;
  runMode = false;
  }

  void Update(unsigned long currentMillis) {
    if(runMode == true and (currentMillis - previousMillis >= CheckTime)) {
      totalMillis += (currentMillis - previousMillis);
      if (totalMillis > OnTime * 1000) {  // if OnTime Exceeded
        self.begin();                     // clear buffer
        self.show();                      // clear display
        runMode = false;                  // stop updating
        previousMillis = 0;
        totalMillis = 0;
      }
      else {
        switch(neoDisp) {
          case 'colorWipe':
            colorWipe(strip.Color(255, 0, 0), 50); // Red
  //          colorWipe(strip.Color(0, 255, 0), 50); // Green
  //          colorWipe(strip.Color(0, 0, 255), 50); // Blue
            break;
          case 'theaterChase':
              theaterChase(strip.Color(127, 127, 127), 50); // White
  //            theaterChase(strip.Color(127, 0, 0), 50); // Red
  //            theaterChase(strip.Color(0, 0, 127), 50); // Blue
            break;
          case 'rainbow':
            rainbow(20);
  //          rainbowCycle(20);
  //          theaterChaseRainbow(50);
            break;
        }
      
    else {
      
    }
    }

    
    if((currentMillis - previousMillis >= OnTime))
    {
      strip.begin();
      strip.show(); // Set all pixels to 'off'
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

  void colorWipe(uint32_t c, uint8_t wait) {
  // Fill the dots one after the other with a color
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
    }
  }
  
  void rainbow(uint8_t wait) {
    uint16_t i, j;
  
    for(j=0; j<256; j++) {
      for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel((i+j) & 255));
      }
      strip.show();
      delay(wait);
    }
  }
  
  // Slightly different, this makes the rainbow equally distributed throughout
  void rainbowCycle(uint8_t wait) {
    uint16_t i, j;
  
    for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
      for(i=0; i< strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
      }
      strip.show();
      delay(wait);
    }
  }
  
  //Theatre-style crawling lights.
  void theaterChase(uint32_t c, uint8_t wait) {
    for (int j=0; j<10; j++) {  //do 10 cycles of chasing
      for (int q=0; q < 3; q++) {
        for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, c);    //turn every third pixel on
        }
        strip.show();
  
        delay(wait);
  
        for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
      }
    }
  }
  
  //Theatre-style crawling lights with rainbow effect
  void theaterChaseRainbow(uint8_t wait) {
    for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
      for (int q=0; q < 3; q++) {
        for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
  
        delay(wait);
  
        for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
      }
    }
  }
  
  // Input a value 0 to 255 to get a color value.
  // The colours are a transition r - g - b - back to r.
  uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
      return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if(WheelPos < 170) {
      WheelPos -= 85;
      return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
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

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(35, 5, NEO_GRB + NEO_KHZ800);

// Setup
void setup() 
{ 
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
 
void Reset()
{
  sweeper1.reset();
  sweeper2.reset();
}

// Interrupt is called once a millisecond, 
SIGNAL(TIMER0_COMPA_vect) 
{
  unsigned long currentMillis = millis();
  sweeper1.Update(currentMillis);
  sweeper2.Update(currentMillis);
  led1.Update(currentMillis);
  led2.Update(currentMillis);
  strip1.Update(currentMillis);
} 

void loop()
{
}
