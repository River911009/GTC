#include <Wire.h>

#define type DEC

byte cmd[32];
unsigned short i=0,c=0,req=22;

void setup(){
  Wire.begin();
  Wire.setWireTimeout(1000000, true);
  Wire.clearWireTimeoutFlag();
  Serial.begin(115200);
}

void loop(){
  if (Wire.getWireTimeoutFlag())
  {
    Wire.clearWireTimeoutFlag();
  }

  Wire.beginTransmission(24);
  Wire.write(0);
  Wire.write(244);
  Wire.endTransmission();
  Wire.requestFrom(24, req);
  
  i=0;
  delay(1);
  while (Wire.available()){
    cmd[i++]=Wire.read();
  }
  if(i!=req && i!=31){
    Wire.endTransmission();
    Serial.print(i);
    Serial.println("data length error");
  }

  if(i==2){
    c=(cmd[0]<<8)+cmd[1];
    Serial.println(c,type);
  }
  else if(i==1){
    cmd[0];
    Serial.println(c,type);
  }
  else if(i>2){
    c=(cmd[0]<<8)+cmd[1];
    Serial.print(c,type);Serial.print('.');
    c=(cmd[2]<<8)+cmd[3];
    Serial.print(c,type);Serial.print('.');
    c=(cmd[4]<<8)+cmd[5];
    Serial.print(c,type);Serial.print('.');
    c=(cmd[6]<<8)+cmd[7];
    Serial.print(c,type);Serial.print('.');
    c=(cmd[8]<<8)+cmd[9];
    Serial.print(c,type);Serial.print('.');
    c=(cmd[10]<<8)+cmd[11];
    Serial.print(c,type);Serial.print('.');
    c=(cmd[12]<<8)+cmd[13];
    Serial.print(c,type);Serial.print('.');
    c=(cmd[14]<<8)+cmd[15];
    Serial.print(c,type);Serial.print('.');
    c=(cmd[16]<<8)+cmd[17];
    Serial.print(c,type);Serial.print('.');
    c=(cmd[18]<<8)+cmd[19];
    Serial.print(c,type);Serial.print('.');
    c=(cmd[20]<<8)+cmd[21];
    Serial.print(c,type);Serial.print('.');
    c=(cmd[22]<<8)+cmd[23];
    Serial.print(c,type);Serial.print('.');
    c=(cmd[24]<<8)+cmd[25];
    Serial.print(c,type);Serial.print('.');
    c=(cmd[26]<<8)+cmd[27];
    Serial.print(c,type);Serial.print('.');
    c=(cmd[28]<<8)+cmd[29];
    Serial.print(c,type);Serial.print('.');
    c=(cmd[30]<<8)+cmd[31];
    Serial.print(c,type);Serial.print('\n');
  }
  
  delay(100);
}
