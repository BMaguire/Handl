#include "../include/FPS_GT511C3.h"

/*int counter = 0;*/
int Serial1BaudRate = 9600;
/*int incomingByte = 0; // for incoming serial data*/

FPS_GT511C3 fps(2, 1); // (Arduino SS_RX = pin 4, Arduino SS_TX = pin 5)

void Enroll()
{
  // Enroll test

  // find open enroll id
  int enrollid = 0;
  bool usedid = true;
  while (usedid == true)
  {
    usedid = fps.CheckEnrolled(enrollid);
    if (usedid==true) enrollid++;
  }
  fps.EnrollStart(enrollid);

  // enroll
  Serial.print("Press finger to Enroll #");
  Serial.println(enrollid);
  while(fps.IsPressFinger() == false) delay(100);
  bool bret = fps.CaptureFinger(true);
  int iret = 0;
  if (bret != false)
  {
    Serial.println("Remove finger");
    fps.Enroll1();
    while(fps.IsPressFinger() == true) delay(100);
    Serial.println("Press same finger again");
    while(fps.IsPressFinger() == false) delay(100);
    bret = fps.CaptureFinger(true);
    if (bret != false)
    {
      Serial.println("Remove finger");
      fps.Enroll2();
      while(fps.IsPressFinger() == true) delay(100);
      Serial.println("Press same finger yet again");
      while(fps.IsPressFinger() == false) delay(100);
      bret = fps.CaptureFinger(true);
      if (bret != false)
      {
        Serial.println("Remove finger");
        iret = fps.Enroll3();
        if (iret == 0)
        {
          Serial.println("Enrolling Successful");
        }
        else
        {
          Serial.print("Enrolling Failed with error code:");
          Serial.println(iret);
        }
      }
      else Serial.println("Failed to capture third finger");
    }
    else Serial.println("Failed to capture second finger");
  }
  else Serial.println("Failed to capture first finger");
}

void setup() {
    /*Serial1.listen();*/
    Serial.begin(115200);
    Serial1.begin(Serial1BaudRate);
    /*Serial1.begin(Serial1BaudRate); //set up Arduino's hardware serial UART*/
  	fps.UseSerialDebug = false; // so you can see the messages in the serial debug screen
  	fps.Open(); //send serial command to initialize fps
    fps.SetLED(true);
    Enroll();
}

void loop() {
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
