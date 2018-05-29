// This #include statement was automatically added by the Particle IDE.
#include <neopixel.h>

// #include "application.h"
// #include "neopixel/neopixel.h"


bool d1 = HIGH;
bool d2 = LOW;
//for LED
int led1 = D4;
int led2 = D7;
// for push button
// Our button wired to D0
int buttonPin = D0;
// for number of invalid authorisations
int count = 0;
int alert = 0;
int locked = 1;
String user;

int waitTime = 25;
void spin(int R, int G, int B);


// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 16
#define PIXEL_PIN D6
#define PIXEL_TYPE WS2812

#define GREEN 10,180,10
#define RED 255,0,0
#define AMBER 255,191,0
#define CYAN 10,150,70
#define PURPLE 180,3,180
#define BLUE 5,5,190

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

void spin(int R, int G, int B);



Timer timer(500, stop_motor);
// These need to be changed later. These are just examples for demonstration.
Timer unauthAccess(10000, unauthAccessTimer);
Timer lockout_period(5000, time_out);
Timer alert_tracker(120000, period_complete);

void stop_motor()
{
  analogWrite(3, 0);
  timer.stop();
}

void time_out()
{
  // once a specified amount of time has passed unlock the system and reset count.
  count = 0;
  // Sets the RGB LED to cyan
  RGB.color(0, 255, 255);
  unauthAccess.stop();
  lockout_period.stop();

}

void unauthAccessTimer()
{
    // if there are three failed attempts then send SMS via IFTTT
    if (count == 3 || count > 3)
    {
      //lockout_period.start();
      // Notify for an SMS to be sent
      alert = 1;
      Serial.println("ALERT - to many unauthorised attempts");
      // take control of the LED
      RGB.control(true);

      // Sets the RGB LED to white
      RGB.color(255, 255, 255);

      alert_tracker.start();
    }
    else{

      count = 0;
      // take control of the LED
      RGB.control(true);
      // Sets the RGB LED to cyan
      RGB.color(0, 255, 255);

    }
}

void period_complete()
{
  alert = 0;
  alert_tracker.stop();
}


void unLockDoor (){

  digitalWrite(led2,HIGH);

  if (locked == 1 )
  {
    Serial.println("Authorised  - unlocking Door");

    spin(AMBER);
    spin(AMBER);
    spin(AMBER);

    // strip.begin();
    // strip.setPixelColor(255,0,0);  //set RED- FOR unLOCKED
    // strip.show();

    show(RED);

    user = "Teahneal";
    Particle.publish("log", user);
    Serial.println(user);
    digitalWrite(1,d1);
    digitalWrite(2,d2);
    analogWrite(3, 255);
  }
  else if (locked == 0 ){
    Serial.println("Door is unlocked");
    // strip.begin();
    // strip.setPixelColor(255,0,0);  //set RED- FOR unLOCKED
    // strip.show();

    show(RED);


  }

  locked = 0;
  timer.start();
}

void unauthorisedUser(){

  //log unauthorised User
  user = "Unauthorised Access";
  Particle.publish("log", user);

  //SPIN RED FOR UNAUTHORISED USER
  spin(RED);
  spin(RED);
  spin(RED);

//   strip.begin();
//   strip.setPixelColor(10,180,10);  //Initially set GREEN - FOR LOCKED
//   strip.show();

    show(GREEN);



  count++;

  if (count == 1)
  {
    unauthAccess.start();
  }
}

void motorSetup(){

  pinMode(1, OUTPUT);  /* Motor control pin 1 */
  pinMode(2, OUTPUT);  /* Motor control pin 2 */
  pinMode(3, OUTPUT);  /* PWM pin for Speed Control */

}


void lightSetup(){

  pinMode(led2, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  pinMode( buttonPin , INPUT_PULLUP); // sets pin as input

  strip.begin();
  strip.show();
  show(GREEN);

}


void setup()
{
  motorSetup();

  lightSetup();

   Serial.begin(9600);
   Particle.variable("alert", &alert, INT);

}


// PUT THIS IN THE COMMAND LINE: screen /dev/tty.usbmodem1421
void serialEvent()
{
    char c = Serial.read();

    if (c == 'a' ){  //&& count < 3
      unLockDoor();
    }
    else if (c == 'u'  ){    //&& count < 3
      Serial.println("Unauthorised");
      unauthorisedUser();
    }
}

void spin(int R, int G, int B) {
    for(int i=0; i < PIXEL_COUNT; i++) {
        strip.setPixelColor(i, R,G,B);
        strip.show();
        delay(waitTime);
    }
    for(int i=0; i < PIXEL_COUNT; i++) {
        strip.setPixelColor(i, 0,0,0);
        strip.show();
        delay(waitTime);
    }
}


void show(int R, int G, int B) {
    for(int i=0; i < PIXEL_COUNT; i++) {
        strip.setPixelColor(i, R,G,B);
        strip.show();
    }
}

void loop()
{

  //show(GREEN);
  int buttonState = digitalRead( buttonPin );
  if( buttonState == LOW ) //door shut
  {

    //show(PURPLE);

    if(locked == 0 && count < 3)
    {
      Serial.println("Lock Door");
      digitalWrite(1,!d1);
      digitalWrite(2,!d2);
      analogWrite(3, 255);
      digitalWrite( led2, HIGH);

    //   strip.begin();
    //   strip.setPixelColor(10,180,10);  //Initially set GREEN - FOR LOCKED
    //   strip.show();

    show(GREEN);


    }

    locked = 1;
    timer.start();


  }else //door open
  {
    //show(CYAN);
  }

  if (locked == 1 ){
    //Serial.println("Door is unlocked");


  }
}
