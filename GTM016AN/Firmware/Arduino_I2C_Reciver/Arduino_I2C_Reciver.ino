#include <Wire.h>

void setup(){
  Wire.begin();
  Serial.begin(115200);
}

void loop(){
  Wire.beginTransmission(24);
  Wire.write(5);
  Wire.endTransmission();
  Wire.requestFrom(24, 22);

  while (Wire.available()){
    unsigned short c = (Wire.read()<<8)|Wire.read();
    Serial.println(c);
  }

  delay(500);
}
