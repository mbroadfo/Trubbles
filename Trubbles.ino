#include <Servo.h> 
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

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
    Serial.print("Kickoff: ");
    Serial.print(currMs);
    Serial.print(" : ");
    Serial.println(previousMillis);
 
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
    Serial.print("Turnoff: ");
    Serial.print(previousMillis);
    Serial.print(" : ");
    Serial.println(totalMillis);
 
    for (int i = 0; numPixels(); i++) {
      setPixelColor(x,0); // Initialize Pixels
    }
    show();               // Clear Display
    runMode = false;      // Turn Off NeoPix Object
  }

  void Update(unsigned long currentMillis) {
    Serial.print("Update: Runmode=");
    Serial.print(runMode);
    Serial.print(" Current=");
    Serial.print(currentMillis);
    Serial.print(" Previous=");
    Serial.print(previousMillis);
    Serial.print(" CheckTime=");
    Serial.println(CheckTime);
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

NeoPix strip1(35, 55, NEO_GRB + NEO_KHZ800);

// Setup
void setup() { 
  Serial.begin(9600);

  // Timer0 is already used for millis() - we'll just interrupt somewhere in the middle and call the "Compare A" function below
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  
  // Setup Button
  pinMode(2, INPUT_PULLUP);

  // Attach the Servos
  sweeper1.Attach(8);
  sweeper2.Attach(9);

  // Initialize the Strip
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
  Serial.print("Current time: ");
  Serial.println(currentMillis);

  if(digitalRead(2) != HIGH) {
    strip1.kickOff(currentMillis, 50, 5, "rainbowCycle");
  }
  
//  sweeper1.Update(currentMillis);
//  sweeper2.Update(currentMillis);
//  led1.Update(currentMillis);
//  led2.Update(currentMillis);
//  strip1.Update(currentMillis);
} 

void loop() {
}
