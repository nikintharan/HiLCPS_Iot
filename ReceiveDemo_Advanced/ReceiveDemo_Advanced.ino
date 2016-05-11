/*
  Example for receiving

  Taken from:
  http://code.google.com/p/rc-switch/
  
  If you want to visualize a telegram copy the raw data and 
  paste it into http://test.sui.li/oszi/
*/

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();

//Runs once upon startup
void setup() {
  //Set up serial console (rate in baud)
  Serial.begin(9600);
  
  //Use digital pin 2 as an interrupt
  mySwitch.enableReceive(digitalPinToInterrupt(2));
  Serial.println("Setup completed");
}

//Continuously runs while Huzzah remains on
void loop() {
  if (mySwitch.available()) {
    output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(),mySwitch.getReceivedProtocol());
    mySwitch.resetAvailable();
  }
}
