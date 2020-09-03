#include "Arduino.h"
#include "MPU6050_tockn.h"
#include "Wire.h"
#include "I2Cdev.h"
#include "TimeLib.h"
#include "math.h"

#include <WiFi.h>
#include <WiFiUdp.h>

#define BUFFERSIZE 420

const char * ssid = "hugodocpu";
const char * password = "DV9SrOap";

const char * udpAddress =  "10.42.0.1";
const int udpPort = 11111;

static int iSS=1,paStatic=200,nsStatic=1;


MPU6050 mpu6050(Wire);
WiFiUDP udp;

char buffer[BUFFERSIZE];

struct Parameter{
    int ts;
    int pm;
    int pa;
    int ns;
};

struct Parameter prmS;
bool dataT=false,errorT=false;


void setup() {
  Serial.begin(115200);
  Wire.begin();
  while (!Serial);
  
  while(WiFi.status()!= WL_CONNECTED){
      Serial.println("Trying to connect.");
      WiFi.begin(ssid, password);
      Serial.println("Please wait...");
      delay(10000);
    }
  Serial.print("WiFi Connected.");  
  udp.begin(udpPort);
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  //estabelecer rota
  udp.beginPacket(udpAddress,udpPort);
  bit32Input(0,4,4);
  bit32Input(4,8,iSS);
  udp.write((uint8_t *)buffer,sizeof(buffer));
  udp.endPacket();
  //fim de envio de data
}

void loop() {
  int tp;
  memset(buffer,'\0',BUFFERSIZE);
  int packetSize=udp.parsePacket();
  if(packetSize){
     while((udp.read((uint8_t *)buffer, BUFFERSIZE))>0){}
     tp=bit32Read(0);
     switch (tp) {
        case 0:
        Serial.println("Start");
          if (start()){
            dataT=true;
          }else{
            Serial.println("Error");
            errorT=true;
          }
          break;
        case 2:
          Serial.println("Stop");
          
          dataT=false;
          break;
        default:
          Serial.println("Error");
          error(5);
          errorT=true;
     }
  }
  if(dataT){
    Serial.println("Data");
    memset(buffer,'\0',BUFFERSIZE);
    if (dataPacket()){}else{
      Serial.println("Error");
      dataT=false;
      errorT=true;
    }
  }
  if(dataT || errorT){
    udp.beginPacket(udpAddress,udpPort);
    udp.write((uint8_t *) buffer,sizeof(buffer));
    udp.endPacket();
  }
  errorT=false;
}
void bit32Input(int begin,int end,int var){
  int n=24;
  for (int i = begin; i < end; i++) {
    buffer[i]=var >> n;
    n=n-8;
  }
}
int bit32Read(int i){
  int a;
  a = (int)buffer[i]*pow(2,24)+(int)buffer[i+1]*pow(2,16)+(int)buffer[i+2]*pow(2,8)+(int)buffer[i+3]*1;
  return a;
}
bool start(){
  prmS.ts=bit32Read(4);
  prmS.pm=bit32Read(8);
  prmS.pa=bit32Read(12);
  prmS.ns=bit32Read(16);
  setTime(prmS.ts);

  if (prmS.ns==0){
    prmS.ns=nsStatic;
  }else if (prmS.ns>8){
    //ns elevado
    error(0);
    return false;
  }
  if (prmS.pa==0){
    prmS.pa=paStatic;
  }else if(prmS.pa<25){
    //pa muito baixo 
    error(1);
    return false;
  }
  if(prmS.pm==0){
    prmS.pm=500;
  }else if(prmS.pm<200){
    //pm muito baixo
    error(2);
    return false;
  }
  return true;
}
bool dataPacket(){
  int timerPA,n;
  int timerPM=millis();
  bit32Input(0,4,1);//tp
  bit32Input(4,8,iSS);
  bit32Input(8,12,now());//ts
  bit32Input(12,16,prmS.pm);
  bit32Input(16,20,prmS.pa);
  bit32Input(20,24,prmS.ns);
  
  n=24;
  for (int i = 0; i < prmS.ns; i++) {
    timerPA=millis();
    mpu6050.update();
    float a=constrain(mpu6050.getAccX(),-1,1);
    Serial.print("accX: ");Serial.print(a);
    bit32Input(n,n+4,negativeAdjust(a));
    
    n=n+4;
    a=constrain(mpu6050.getAccY(),-1,1);
    Serial.print("  accY: ");Serial.print(a);
    bit32Input(n,n+4,negativeAdjust(a));
    
    n=n+4;
    a=constrain(mpu6050.getAccZ(),-1,1);
    Serial.print("  accZ: ");Serial.println(a);
    bit32Input(n,n+4,negativeAdjust(a));
    
    n=n+4;
    a=mpu6050.getGyroX();
    //Serial.print("gX: ");Serial.print(a);
    bit32Input(n,n+4,negativeAdjust(a));
    
    n=n+4;
    a=mpu6050.getGyroY();
    //Serial.print("  gY: ");Serial.print(a);
    bit32Input(n,n+4,negativeAdjust(a));
    
    n=n+4;
    a=mpu6050.getGyroZ();  
    //Serial.print("  gZ: ");Serial.println(a);
    bit32Input(n,n+4,negativeAdjust(a));
    
    n=n+4;
    a=mpu6050.getTemp();
    //Serial.print("Temp: ");Serial.println(a);
    bit32Input(n,n+4,negativeAdjust(a));
    n=n+4;
    
    while((timerPA+prmS.pa)!=millis()){
      if((timerPA+prmS.pa)<millis()){
        //error timeout em pa
        error(3);
        return false;
      }
    }
  }

  while((timerPM+prmS.pm)!=millis()){
    if((timerPA+prmS.pm)<millis()){
        //error timeout em pm
        error(4);
        return false;
    }
  }
  return true;
}
void error(int tipo){
  memset(buffer,'\0',BUFFERSIZE);
  bit32Input(0,4,3);//tp
  bit32Input(4,8,iSS);
  bit32Input(8,12,now());
  bit32Input(12,16,tipo);
}


int negativeAdjust(float a){
  a=a*1000;
    if(a<0){
      a=a+1200000;
    }
  return (int)a;
}
