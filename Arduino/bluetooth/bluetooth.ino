#include <SoftwareSerial.h>

SoftwareSerial btSerial(7,8);

void setup() 
{
  Serial.begin(9600);
  btSerial.begin(9600);
}

void loop()
{
  if(Serial.available()) 
    btSerial.write(Serial.read());
  
  if(btSerial.available())
    Serial.write(btSerial.read());
}
