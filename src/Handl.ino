#include "Fingerprint_FPS_GT511C3.h"
#include "HttpClient.h"
#include "WebDuino.h"

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


void formCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  Serial.println("Test");
  if (type == WebServer::POST)
  {
    Serial.println("Test2");
    bool repeat;
    char name[16], value[16];
    do
    {
      Serial.println("Test3");
      repeat = server.readPOSTparam(name, 16, value, 16);
      Serial.println("Test3");
      if (name[0] == 'i')
      {
        int pin = strtoul(name + 1, NULL, 10);
        int val = strtoul(value, NULL, 10);
        fps.DeleteID(val-'0');
        Serial.println("Deleted" + String((val-'0')));
      }
    } while (repeat);

    server.httpSeeOther(PREFIX "/form");
  }
  Serial.println("Test4");
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
  pinMode(button, INPUT_PULLDOWN);
  webserver.begin();
  webserver.addCommand("form", &formCmd);

  initAuth();
}

void loop() {
    webserver.processConnection();
    Serial.println(WiFi.localIP());

    if (digitalRead(button) == LOW) {
      Serial.print("Button Pressed: Begin Enrollment");
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
    /*fps.SetLED(true); // turn on the LED inside the fps*/
  	/*delay(1000);*/
  	/*fps.SetLED(false);// turn off the LED inside the fps*/

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
      Serial.println("Please press finger");
    }
    delay(100);

  }
