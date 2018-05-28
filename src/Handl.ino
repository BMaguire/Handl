#include "Fingerprint_FPS_GT511C3.h"
#include "HttpClient.h"
#include "WebDuino.h"

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
    // if there are three failed attempts then lock system and send SMS via IFTTT
    if (count == 3 || count > 3)
    {
      lockout_period.start();
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
    user = "Teahneal";
    Particle.publish("log", user);
    Serial.println(user);
    digitalWrite(1,d1);
    digitalWrite(2,d2);
    analogWrite(3, 255);
  }
  else if (locked == 0 ){
    Serial.println("Door is unlocked");
  }

  locked = 0;
  timer.start();
}

void unauthorisedUser(){

  count++;

  if (count == 1)
  {
    unauthAccess.start();
  }
}

// PUT THIS IN THE COMMAND LINE: screen /dev/tty.usbmodem1421
void serialEvent()
{
    char c = Serial.read();

    if (c == 'a' && count < 3){
      unLockDoor();
    }
    else if (c == 'u' && count < 3 ){
      Serial.println("Unauthorised");
      unauthorisedUser();
    }
    else {
      RGB.control(true);
      // Set the RGB LED to red
      RGB.color(255, 0, 0);
      Serial.println("System Locked Unauthorised Access");
      user = "Unauthorised Access";
      Particle.publish("log", user);

    }
}




/* WEB SERVER */
template<class T>
inline Print &operator <<(Print &obj, T arg)
{ obj.print(arg); return obj; }

#define PREFIX ""
WebServer webserver(PREFIX, 80);


/* WEB SERVER END*/



/*int counter = 0;*/
int Serial1BaudRate = 9600;
/*int incomingByte = 0; // for incoming serial data*/
int button = D0;

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
char* url = "192.168.43.47";
int port = 3000;


void deleteCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  if (type == WebServer::POST)
  {
    bool repeat;
    char name[16], value[16];
    do
    {
      repeat = server.readPOSTparam(name, 16, value, 16);
      if (name[0] == 'i')
      {
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


bool waitForRelease() {
  while(fps.IsPressFinger()) delay(100);
}

bool waitForPress() {
  while(!fps.IsPressFinger()) delay(100);
}

void Enroll() {
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
      return;
    }
    // prompt for finger removal
    Serial.println("Remove finger");
    // enroll correct template
    switch(i) {
      case 1:
        fps.Enroll1();
      break;
      case 2:
        fps.Enroll2();
      break;
      case 3:
        iret = fps.Enroll3();
      break;
    }
    // wait for release
    waitForRelease();
    // if it's not the last enroll attempt prompt for another attempt
    if (i < 3) Serial.println("Press same finger again");
  }

  if (iret == 0) {
    postData("{\"profile\":{\"print_id\":"+String(enrollid)+"}}");
    Serial.println("Enrolling Successful");
  } else {
    Serial.print("Enrolling Failed with error code:");
    Serial.println(iret);
  }
}

void initFingerPrintScanner() {
  Serial.begin(115200);
  Serial1.begin(Serial1BaudRate);
  fps.UseSerialDebug = false; // so you can see the messages in the serial debug screen
  fps.Open(); //send serial command to initialize fps
  fps.SetLED(true);
}

void initAuth() {
  initFingerPrintScanner();
}

void requestData() {
  request.hostname = url;
  request.port = port;
  request.path = "/api/profiles";
  request.body = "";
  http.get(request, response, headers);

  Serial.print("Application>\tResponse status: ");
  Serial.println(response.status);

  Serial.print("Application>\tHTTP Response Body: ");
  Serial.println(response.body);
}

void postData(String body) {
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

void setup() {
  pinMode(1, OUTPUT);  /* Motor control pin 1 */
  pinMode(2, OUTPUT);  /* Motor control pin 2 */
  pinMode(3, OUTPUT);  /* PWM pin for Speed Control */

  pinMode(led2, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  pinMode( buttonPin , INPUT_PULLUP); // sets pin as input

  Serial.begin(9600);

  Particle.variable("alert", &alert, INT);

  pinMode(button, INPUT_PULLDOWN);
  webserver.begin();
  webserver.addCommand("delete", &deleteCmd);

  initAuth();
}

void loop() {
    webserver.processConnection();
    /*Serial.println(WiFi.localIP());*/

    if (digitalRead(button) == LOW) {
      Serial.println("Button Pressed: Begin Enrollment");
      Enroll();
    }
    /*Serial.printlnf("testing %d", 1);*/
    /*delay(1000);*/
    /*
      GT-511C1R
      https://cdn.sparkfun.com/datasheets/Sensors/Biometric/GT-511C1R_datasheet_V1%205_20140312.pdf
      https://learn.sparkfun.com/tutorials/fingerprint-scanner-hookup-guide?_ga=2.202085589.539386202.1524630265-817810494.1521357116
      fingerprint initializes at 9600 baud rate
    */

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
      }
      else
      {//if unable to recognize
        Serial.println("Finger not found");
      }
    }
    else
    {
      /*Serial.println("Please press finger");*/
    }



    int buttonState = digitalRead( buttonPin );
    if( buttonState == LOW )
    {

      if(locked == 0 && count < 3)
      {
        Serial.println("Lock Door");
        digitalWrite(1,!d1);
        digitalWrite(2,!d2);
        analogWrite(3, 255);
        digitalWrite( led2, HIGH);
      }

      locked = 1;
      timer.start();


    }else
    {
    }


    delay(100);



  }
