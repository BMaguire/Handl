
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


void setup()
{
  pinMode(1, OUTPUT);  /* Motor control pin 1 */
  pinMode(2, OUTPUT);  /* Motor control pin 2 */
  pinMode(3, OUTPUT);  /* PWM pin for Speed Control */

  pinMode(led2, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  pinMode( buttonPin , INPUT_PULLUP); // sets pin as input

   Serial.begin(9600);

   Particle.variable("alert", &alert, INT);
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

void loop()
{
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
}
