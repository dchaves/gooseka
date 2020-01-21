#include "commons.h"

HardwareSerial MySerial(1);


void setup() {

     MySerial.begin(115200, SERIAL_8N1, 16, 17);

     // Configure motor frequency
     ledcSetup(LEFT_MOTOR_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
}

void loop() {

  // Stop the motors
  ledcWrite(LEFT_MOTOR_CHANNEL, 0);

  delay(2000); 

  while(true) {
     // Doing stuff here
  }

}
