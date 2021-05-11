#include <Wire.h>

byte cmd[32];
unsigned short x,addr;

void setup() {
  Wire.begin();
  Wire.setWireTimeout(1000000, true);
  Wire.clearWireTimeoutFlag();
  Serial.begin(115200);
}

void loop() {
  Wire.beginTransmission(24);
  Wire.write(highByte(addr));
  Wire.write(lowByte(addr));
  Wire.endTransmission();
  Wire.requestFrom(24, 22);

  x=0;
  delay(1);
  while(Wire.available()){
    cmd[x++]=Wire.read();
  }
  
  for(int i=0;i<11;i++){
    unsigned short c=(cmd[i*2]<<8)+cmd[i*2+1];
    Serial.print(addr*100,DEC);
    Serial.print(',');
    Serial.println(c,DEC);
  }
  if(addr<473){
    addr+=11;
  }
  else{
    addr=0;
    delay(1000);
  }
}
