#include <ESP32Servo.h>
// Define throttle signal values
const int minThrottle = 1100; // Minimum throttle in microseconds (1ms)
const int maxThrottle = 1900; // Maximum throttle in microseconds (2ms)
bool armed = false;

// PWM configuration

int pins[] = {15,2,4,16,17,5,18,19};
int killSwitchPin = 13;
int ledPin1 = 21;
int max_throttle = 15;

// Helper function to convert percentage to duty cycle
bool lastPressed = false;
long pressTime = 0;

Servo motors[8];
 
void killSwitch() {
  //Serial.println("Kill switch activated! Sending minimum throttle.");
  if(digitalRead(killSwitchPin) == HIGH) {
    if(lastPressed == false ){
      pressTime = millis();
    }
    lastPressed = true;
  }
  else{
    if(lastPressed == true){
      if(millis() - pressTime > 3000 && millis() - pressTime < 5000){
        armed = true;
        Serial.println("Armed!");
      }
      else if(millis() - pressTime > 25 && millis() - pressTime < 300){
        armed = false;
        Serial.println("Disarmed!");
        for(int i = 0;i < 8; i++)
          motors[i].writeMicroseconds(throttle2pwm(0));
      }
    }
    lastPressed = false;
  }
}

float throttle2pwm(int throttle){
  return map(throttle, -100, 100, minThrottle, maxThrottle);
}
void setup() {
  // Initialize PWM for the ESC

  for(int i = 0;i < 8; i++){
    motors[i] = Servo();
    motors[i].attach(pins[i]);
  }
  
  Serial.begin(9600);
  Serial.println("Arming ESC...");
  pinMode(killSwitchPin, INPUT_PULLDOWN);
  pinMode(ledPin1, OUTPUT);
   for(int i = 0;i < 8; i++){
    motors[i].writeMicroseconds(throttle2pwm(0));
   }
  delay(100);
  for(int i = 0;i < 8; i++){
    motors[i].writeMicroseconds(throttle2pwm(2));
   }
  delay(100);
  for(int i = 0;i < 8; i++){
    motors[i].writeMicroseconds(throttle2pwm(0));
   }
  Serial.println("ESCs Armed.");
}

void loop() {
  // Check for serial input
  killSwitch();
  if(armed)
    digitalWrite(ledPin1, HIGH);
  else 
    digitalWrite(ledPin1, LOW);
    
  if (Serial.available() > 0) {// if there are least 20 characters
    int throttle[8];
    for(int i = 0;i < 8; i++){
      throttle[i] = Serial.parseInt();
    }
    if(armed){
      for(int i = 0;i < 8; i++){
         motors[i].writeMicroseconds(throttle2pwm(throttle[i]));
      }
      Serial.println("sending 8 motor pwms");
    }
    while(Serial.available()){
     Serial.read();
    }
  }
}
