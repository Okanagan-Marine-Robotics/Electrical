#include <ESP32Servo.h>
// Define throttle signal values
const int minThrottle = 1100; // Minimum throttle in microseconds (1ms)
const int maxThrottle = 1900; // Maximum throttle in microseconds (2ms)
bool armed = false;

// PWM configuration
int pins[] = {15, 2, 4, 16, 17, 5, 18, 19};
int killSwitchPin = 13;
int ledPin1 = 12;
int ledPin2 = 14;
int ledPin3 = 27;
int max_throttle = 45;

bool lastPressed = false;
long pressTime = 0;

Servo motors[8];

void killSwitch() {
  if (digitalRead(killSwitchPin) == HIGH) {
    if (lastPressed == false ) {
      pressTime = millis();
    }
    lastPressed = true;
  }
  else {
    if (lastPressed == true) {
      if (millis() - pressTime > 3000 && millis() - pressTime < 7000) {
        armed = true;
        Serial.println("Armed!");
      }
      else if (millis() - pressTime > 5 && millis() - pressTime < 300) {
        armed = false;
        Serial.println("Disarmed!");
        for (int i = 0; i < 8; i++)
          motors[i].writeMicroseconds(throttle2pwm(0));
      }
    }
    lastPressed = false;
  }
}

float throttle2pwm(float throttle) {
  return map(throttle, -100, 100, minThrottle, maxThrottle);
}
void escArm() {
  delay(500);
  for (int i = 0; i < 8; i++) {
    motors[i].writeMicroseconds(1500);
  }
  delay(3000);
  for (int i = 0; i < 8; i++) {
    motors[i].writeMicroseconds(1500);
  }
}
void setup() {
  // Initialize PWM for the ESC
  for (int i = 0; i < 8; i++) {
    motors[i] = Servo();
    motors[i].attach(pins[i]);
  }

  Serial.begin(115200);
  Serial.println("Arming ESC...");
  pinMode(killSwitchPin, INPUT_PULLDOWN);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  escArm();
  Serial.println("ESCs Armed.");
  while(Serial.available() > 0){
    Serial.read();
  }
  
}
const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data

boolean newData = false;
void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;
    
    if (Serial.available() > 0) {
        rc = Serial.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

float throttle[8];
int currMotor = 0;
void parseNewData(){
    char * strtokIndx; // this is used by strtok() as an index

    strtokIndx = strtok(receivedChars,"<");      // get the first part - the string
    currMotor=atoi(strtokIndx);

    strtokIndx = strtok(NULL, "<");
    throttle[currMotor] = atof(strtokIndx);     // convert this part to a float
    newData = false;
}
void loop() {
  // Check for serial input
  killSwitch();
  if (armed) {
    analogWrite(ledPin1, 100);
    analogWrite(ledPin2, 100);
    analogWrite(ledPin3, 100);
  }
  else {
    analogWrite(ledPin1, 0);
    analogWrite(ledPin2, 0);
    analogWrite(ledPin3, 0);
  }
  recvWithEndMarker();
  if(newData)parseNewData();
  if (armed) {
    for (int i = 0; i < 8; i++) {
      motors[i].writeMicroseconds(throttle2pwm(throttle[i]));
      /*
        Serial.print("sending ");
        Serial.print(throttle2pwm(throttle[i]));
        Serial.print(" to motor ");
        Serial.println(i);
      */
    }
  }
  else {
    for (int i = 0; i < 8; i++) {
      motors[i].writeMicroseconds(1500);
    }
  }
}
