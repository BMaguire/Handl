// This #include statement was automatically added by the Particle IDE.
#include <neopixel.h>
#include "Fingerprint_FPS_GT511C3.h"
#include "HttpClient.h"
#include "WebDuino.h"

// #include "application.h"
// #include "neopixel/neopixel.h"

/* WEB SERVER */
template<class T>
inline Print &operator <<(Print &obj, T arg)
{ obj.print(arg); return obj; }

#define PREFIX ""
WebServer webserver(PREFIX, 80);

bool d1 = HIGH;
bool d2 = LOW;
//for LED
//int led1 = D4;
int doorBtn = D4;
// for push button
// Our button wired to D0
int buttonPin = D0;
int motorBtn = D7;

int enrollButton = D5;
// for number of invalid authorisations
int count = 0;
int alert = 0;
int locked = 1;
String user;

int Serial1BaudRate = 9600;

HttpClient http;
// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    { "Content-Type", "application/json" },
    //  { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;
FPS_GT511C3 fps;
char* url = "172.20.10.8";//"192.168.43.47"; //ip address of computer when connected to phone
int port = 3000;


enum states {
  LOCKED,
  ENROLL,
  AUTHORISE,
  UNLOCKED
};



enum states state = LOCKED;

int waitTime = 25;
void spin(int R, int G, int B, int PIXEL_COUNT);





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


void checkAccess(int id){

  //digitalWrite(led2,HIGH);

  if (locked == 1 )
  {
    Serial.println("Authorised  - unlocking Door");
    unlockingTransition();
    postActivityData("{\"activity\":{\"print_id\":"+String(id)+"}}");
  }
  else if (locked == 0 ){
    Serial.println("Door is unlocked");
    lockingTransition();
    // show(RED);
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

  // pinMode(led2, OUTPUT);
  // digitalWrite(led1, LOW);
  // digitalWrite(led2, LOW);
  pinMode( buttonPin , INPUT_PULLUP); // sets pin as input

  strip.begin();
  strip.show();
  show(GREEN);

}

// PUT THIS IN THE COMMAND LINE: screen /dev/tty.usbmodem1421
// void serialEvent()
// {
//     char c = Serial.read();
//
//     if (c == 'a' ){  //&& count < 3
//       unLockDoor();
//     }
//     else if (c == 'u'  ){    //&& count < 3
//       Serial.println("Unauthorised");
//       unauthorisedUser();
//     }
// }

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

void spinEnroll(int R, int G, int B, int num, int count) {
    for(int i=num; i < count; i++) {
        strip.setPixelColor(i, R,G,B);
        strip.show();
        delay(40);
    }
}

void spinUnlock(int R, int G, int B) {
  int top_start = 0;
  int bottom_start = 8;
  for( int i= 0; i <= 4; i++) {
      show(GREEN);
      strip.setPixelColor(top_start + i, 0,0,250);
      strip.setPixelColor(bottom_start + i, 0,0,250);

      strip.show();
      delay(150);
  }
}


void ledOff() {
    for(int i=0; i < PIXEL_COUNT; i++) {
        strip.setPixelColor(i, 0,0,0);
        strip.show();
        // delay(waitTime);
    }
}


void show(int R, int G, int B) {
    for(int i=0; i < PIXEL_COUNT; i++) {
        strip.setPixelColor(i, R,G,B);
        strip.show();
    }
}




void lockDoor()
{
  digitalWrite(1,!d1);
  digitalWrite(2,!d2);
  analogWrite(3, 255);
}

void unlockDoor()
{
  digitalWrite(1,d1);
  digitalWrite(2,d2);
  analogWrite(3, 255);
}

void initAuth() {
  initFingerPrintScanner();
}

void initFingerPrintScanner() {
  Serial.begin(115200);
  Serial1.begin(Serial1BaudRate);
  fps.UseSerialDebug = false; // so you can see the messages in the serial debug screen
  fps.Open(); //send serial command to initialize fps
  fps.SetLED(true);
}


bool waitForRelease() {
  while(fps.IsPressFinger()) delay(100);
}

bool waitForPress() {
  while(!fps.IsPressFinger()) delay(100);
}

void postProfileData(String body) {
  request.hostname = url;
  request.port = port;
  request.path = "/api/profiles";
  request.body = body;
  http.post(request, response, headers);
  Serial.print("Application>\tResponse status: ");
  Serial.println(response.status);

  Serial.print("Application>\tHTTP Response Body: ");
  Serial.println(response.body);
}

void postActivityData(String body) {
  request.hostname = url;
  request.port = port;
  request.path = "/api/activity";
  request.body = body;
  http.post(request, response, headers);
  Serial.print("Application>\tResponse status: ");
  Serial.println(response.status);

  Serial.print("Application>\tHTTP Response Body: ");
  Serial.println(response.body);
}
void Enroll() {
  ledOff();
  fps.SetLED(true);
  // find open enroll id
  int enrollid = 0;
  bool usedid = true;
  while (usedid) {
    usedid = fps.CheckEnrolled(enrollid);
    if (usedid) enrollid++;
  }

  fps.EnrollStart(enrollid);

  // enroll
  Serial.print("Press finger to Enroll #");
  Serial.println(enrollid);
  int iret = 0;

  for (int i = 1; i <= 3; i++) {
    // wait for a finger press
    waitForPress();
    // attempt to capture finger
    // if we fail then  report an error over serial and return the function
    if (!fps.CaptureFinger(true)) {
      Serial.println("Failed to capture finger "+ String(i));
      spin(RED);
      spin(RED);
      state = LOCKED;
      return;
    }
    // prompt for finger removal
    Serial.println("Remove finger");
    // enroll correct template
    switch(i) {
      case 1:
        fps.Enroll1();
        spinEnroll(BLUE,0,5);
      break;
      case 2:
        fps.Enroll2();
        spinEnroll(BLUE,5,10);
      break;
      case 3:
        iret = fps.Enroll3();
        spinEnroll(BLUE,10,16);
      break;
    }
    // wait for release
    waitForRelease();
    // if it's not the last enroll attempt prompt for another attempt
    if (i < 3) Serial.println("Press same finger again");
  }

  if (iret == 0) {
    postProfileData("{\"profile\":{\"print_id\":"+String(enrollid)+"}}");
    spin(GREEN);
    spin(GREEN);
    state = LOCKED;
    Serial.println("Enrolling Successful");
  } else {
    Serial.print("Enrolling Failed with error code:");
    Serial.println(iret);
    spin(RED);
    spin(RED);
  }
}

void fingerRec(){
  if (fps.IsPressFinger())
  {
    fps.CaptureFinger(false);
    int id = fps.Identify1_N();

       /*Note:  GT-521F52 can hold 3000 fingerprint templates
                GT-521F32 can hold 200 fingerprint templates
                 GT-511C3 can hold 200 fingerprint templates.
                GT-511C1R can hold 20 fingerprint templates.
       Make sure to change the id depending on what
       model you are using */
    if (id <20) //<- change id value depending model you are using
    {//if the fingerprint matches, provide the matching template ID
      Serial.print("Verified ID:");
      Serial.println(id);
      checkAccess(id);
    }
    else
    {//if unable to recognize
      Serial.println("Finger not found");
      unauthorisedUser();
    }
  }
  else
  {
    /*Serial.println("Please press finger");*/
  }
  delay(100);
}

void deleteCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{

  if (type == WebServer::POST)
  {
    bool repeat;
    char name[16], value[16];
    do
    {
      repeat = server.readPOSTparam(name, 16, value, 16);
      Serial.println(name);
      Serial.println(value);

      if (name[0] == 'i')
      {
        Serial.println("HIT");
        int val = strtoul(value, NULL, 10);
        if (fps.DeleteID(val)) {
          Serial.println("Deleted profile: " + String(val));
        } else {
          Serial.println("Error occured while deleting: " + String(val));
        }
      }
    } while (repeat);

    /*server.httpSeeOther(PREFIX "/form");*/
    server.httpSuccess();
  }
}



void setup()
{
  motorSetup();
  lightSetup();

  strip.begin();
  strip.show();
  spin(GREEN);

  // Serial.begin(9600);
  Particle.variable("alert", &alert, INT);
  pinMode(enrollButton, INPUT_PULLUP);
  pinMode(doorBtn, INPUT_PULLUP);
  pinMode(motorBtn, INPUT_PULLUP);

  webserver.begin();
  webserver.addCommand("delete", &deleteCmd);

  initAuth();

}


void unlockingTransition()
{
  while(digitalRead( motorBtn ) != HIGH)  //while not white
  {
    spin(AMBER);
    unlockDoor();
  }
  if( digitalRead( motorBtn ) == HIGH ) //- locked
  {
    //REVERSE motorBtn
    lockDoor();
    delay(100);
    stop_motor();

    spin(GREEN);
    spin(GREEN);
    state = UNLOCKED;
  }
}


void lockingTransition()
{
  while(digitalRead( motorBtn ) != HIGH)  //while not white
  {
    spin(AMBER);
    lockDoor();
  }

  if( digitalRead( motorBtn ) == HIGH ) //- locked
  {
    //REVERSE motorBtn
    unlockDoor();
    delay(100);
    stop_motor();

    spin(RED);
    spin(RED);
    state = LOCKED;
  }
}




void loop()
{


  webserver.processConnection();
    /*Serial.println(WiFi.localIP());*/


  if( digitalRead( buttonPin ) == LOW ) //- locked
  {
    state = UNLOCKED;
    // lockDoor();
    // delay(300);
  }
  else{
    state = LOCKED;
  }

  if (digitalRead(doorBtn) == LOW){
    lockingTransition();
  }

  if (digitalRead(enrollButton) == LOW)
  {
    Serial.println("Button Pressed: Begin Enrollment");
    state = ENROLL;
  }

  switch(state) {
    case LOCKED:
        Serial.println("LOCKED");
        // Serial.println(WiFi.localIP());
        stop_motor();

      locked = 1;
      timer.start();
      break;
    case ENROLL:
      Serial.println("ENROLL");
      Enroll();
      break;
    case AUTHORISE:
      break;
    case UNLOCKED:
      Serial.println("UNLOCKED");
      break;
  }

  fingerRec();

  if (locked == 1 ){
    //Serial.println("Door is unlocked");


  }
}
