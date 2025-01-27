#include <WiFi.h>
#include <WiFiUdp.h>
#include "time.h"
#include<Wire.h>

//WiFi
WiFiUDP udp;
//const char* ssid       = "elecom2g-6c8919";
//const char* password   = "lsnwple4aqr8";
const char* ssid       = "HUAWEI P10 lite";
const char* password   = "inoueinoue";
const char * udpAddress = "192.168.43.105";
const int udpPort = 6105;
boolean connected = false;

//NTP
const char* ntpServer = "ntp.nict.jp";
const long  gmtOffset_sec = 3600*9;
const int   daylightOffset_sec = 0;

//DAC
//double maxVoltage = 3.3;
//double voltResolution = 255;
//double High_octet =  0.7 * 255 * (3.3/5.0);
//double Low_octet = 0.3 * 255 * (3.3/5);
int vinPin = 39; //if wifi is on pin 2,4 cannot be used
float voltage = 3.3;
float ADCoffsetdB = +11;

//Variable
int i = 0;
int j = 0;
int k =0;
int tpi = 0;
int hoi = 0;
String msg;
String msgs[30] = {"\0"};
int interval,iteration,att,offtime,ontime;

void setup() {
    Serial.begin(115200);

//Pin Set to Digital GPIO
     pinMode(18, OUTPUT);
     pinMode(19, OUTPUT);
     pinMode(23, OUTPUT);
     pinMode(5, OUTPUT);
     pinMode(13, OUTPUT);
     pinMode(12, OUTPUT);
     
     pinMode(14, OUTPUT);
     pinMode(27, OUTPUT);
     pinMode(16, OUTPUT);
     pinMode(17, OUTPUT);
     pinMode(25, OUTPUT);
     pinMode(26, OUTPUT);
    
//Connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      i++;
      if(i>10) Serial.println("NG...");break;
  }
  Serial.println(" CONNECTED");
  
//NTP sync
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

//AD setting
 /*
 Serial.print(analogRead(2));
 Serial.print(analogRead(4)); 
 Serial.print(analogRead(36));
 */
 Serial.print(analogRead(39)); 

 //analogReadResolution(12); //12 bits
 Serial.print((float(analogRead(vinPin))*voltage/4095.0)/0.022);
 Serial.println(" dBm");
}

void loop() {
  if(Serial.available() > 0){
    msg =  Serial.readString();
    Serial.print("Serial Input: " + msg);  //gain,phase
    int len = split(msg,',',msgs);

    if(msgs[0] == "h" || msgs[0] == "help"){
      Serial.println("Command list;");
      Serial.println("  ho,2,999 //4dB change every 2second, iterate 999times");
      Serial.println("  tp,30,1 //+2dB every 30second-interval, iterate 1time");
      Serial.println("  set,28 //set to 28dB");
      Serial.println("  onoff,32,10,1000,5 //switch attenuator, 10msOFF 1000msON , iterate 5times");
    }
    else if(msgs[0] == "ho" || msgs[0] == "handover"){
      //Input example: ho,2,999 //2second-interval 999iteration
      interval = msgs[1].toInt()*1000;
      iteration = msgs[2].toInt();
      for(j=0;j<iteration;j++){
        HandOver4dBstep(interval,"ho_p18-p12_and_p26-p14");

        udp.beginPacket(udpAddress,udpPort);
        udpLocalTime();
        udp.printf("Handover Iteration: %d", tpi++);
        udp.endPacket();
        Serial.println("UDP:Handover Iteration:" + String(tpi));
      }
    }
    else if(msgs[0] == "tp" || msgs[0] == "throughput"){
      //Input example: tp,30[s],1
      interval = msgs[1].toInt()*1000;
      //iteration = msgs[2].toInt();
      Throughput4dBstep(interval,"tp_p18-p12_and_p26-p14");

      udp.beginPacket(udpAddress,udpPort);
      udpLocalTime();
      udp.printf("Throughputr Iteration: %d", tpi++);
      udp.endPacket();
      printLocalTime();
      Serial.println("UDP:Throughput Iteration:" + String(tpi));
    }
    else if(msgs[0] == "set" || msgs[0] == "setatt"){
      //Input example:set,28
      att = msgs[1].toInt();
      setxdB(att,"p18-p12"); //32dB
      setxdB(att,"p26-p14"); //32dB
/*
      udp.beginPacket(udpAddress,udpPort);
      udpLocalTime();
      udp.printf("Set %d dB.", att);
      udp.endPacket();
      printLocalTime();
*/
Serial.println("Set " + String(att) + " dB");
    }
    else if(msgs[0] == "onoff"){
      //Input example:onoff,32,10,1000,5 // 32dB 10msOFF 1000msATT 5timeIteration
      att = msgs[1].toInt();
      offtime = msgs[2].toInt();
      interval = msgs[3].toInt();
      iteration = msgs[4].toInt();
      Serial.println(msgs[1]+","+msgs[2]+","+msgs[3]+","+msgs[4]);
      Serial.println(String(att)+","+String(offtime)+","+String(interval)+","+String(iteration));

      for(k=0;k<iteration;k++){
        setxdB(att,"p18-p12"); //32dB
        setxdB(att,"p26-p14"); //32dB
        delay(offtime) ;     
        setxdB(0,"p18-p12"); //0dB
        setxdB(0,"p26-p14"); //32dB
        delay(interval-offtime);
      }

      udp.beginPacket(udpAddress,udpPort);
      udpLocalTime();
      udp.printf("onoff %d dB.", att);
      udp.endPacket();
      printLocalTime();
      Serial.println("onoff " + String(att) + " dB");
    }
  }

  /*
  printLocalTime();
  udp.beginPacket(udpAddress,udpPort);
  udpLocalTime();
  udp.printf("No: %d, ", i);
  udp.print((analogRead(0)*3.3/4095.0)/0.022);
  udp.print(" dBm");
  udp.endPacket();
  */
  Serial.print("No: " + String(i) + ", ");
  Serial.print(-1*(float(analogRead(vinPin))*voltage/4095.0)/0.022 + ADCoffsetdB);
  Serial.println(" dBm");
  delay(1000);
  i = i + 1;
}

void setPin(String pinArrange, unsigned char v1_HL,unsigned char v2_HL,unsigned char v3_HL,unsigned char v4_HL,unsigned char v5_HL,unsigned char v6_HL){
  if(pinArrange =="p18-p12"){
    digitalWrite(18,v1_HL); // V1 0.5dB
    digitalWrite(19,v2_HL); // V2 1dB
    digitalWrite(23,v3_HL); // V3 2dB 
    digitalWrite(5,v4_HL); // V4 4dB
    digitalWrite(13,v5_HL); // V5 8dB
    digitalWrite(12,v6_HL); // V6 16dB
  }
  else if(pinArrange =="p26-p14"){
    digitalWrite(26,v1_HL); // V1 0.5dB
    digitalWrite(25,v2_HL); // V2 1dB
    digitalWrite(17,v3_HL); // V3 2dB 
    digitalWrite(16,v4_HL); // V4 4dB
    digitalWrite(27,v5_HL); // V5 8dB
    digitalWrite(14,v6_HL); // V6 16dB
  }
  else if(pinArrange =="ho_p18-p12_and_p26-p14"){ 
    //for handover
    digitalWrite(18,v1_HL); // V1 0.5dB
    digitalWrite(19,v2_HL); // V2 1dB
    digitalWrite(23,v3_HL); // V3 2dB 
    digitalWrite(5,v4_HL); // V4 4dB
    digitalWrite(13,v5_HL); // V5 8dB
    digitalWrite(12,v6_HL); // V6 16dB
    
    digitalWrite(26,1-v1_HL); // V1 0.5dB
    digitalWrite(25,1-v2_HL); // V2 1dB
    digitalWrite(17,1-v3_HL); // V3 2dB 
    digitalWrite(16,1-v4_HL); // V4 4dB
    digitalWrite(27,1-v5_HL); // V5 8dB
    digitalWrite(14,1-v6_HL); // V6 16dB
  }
  else if(pinArrange =="tp_p18-p12_and_p26-p14" || pinArrange =="default"){ 
    //for throughput or default-use
    digitalWrite(18,v1_HL); // V1 0.5dB
    digitalWrite(19,v2_HL); // V2 1dB
    digitalWrite(23,v3_HL); // V3 2dB 
    digitalWrite(5,v4_HL); // V4 4dB
    digitalWrite(13,v5_HL); // V5 8dB
    digitalWrite(12,v6_HL); // V6 16dB
    
    digitalWrite(26,v1_HL); // V1 0.5dB
    digitalWrite(25,v2_HL); // V2 1dB
    digitalWrite(17,v3_HL); // V3 2dB 
    digitalWrite(16,v4_HL); // V4 4dB
    digitalWrite(27,v5_HL); // V5 8dB
    digitalWrite(14,v6_HL); // V6 16dB
  }
}


void HandOver4dBstep(int sleepTime,String controlPin){
  //up down
  setPin(controlPin,0,0,0,0,0,0); // 0,32dB 
  delay(sleepTime);  
  setPin(controlPin,0,0,0,1,0,0); //4,28dB
  delay(sleepTime);
  setPin(controlPin,0,0,0,0,1,0); //8,24dB
  delay(sleepTime);
  setPin(controlPin,0,0,0,1,1,0); //12,20dB
  delay(sleepTime);
  setPin(controlPin,0,0,0,0,0,1); //16,16dB
  delay(sleepTime);
  setPin(controlPin,0,0,0,1,0,1); //20,12dB 
  delay(sleepTime);
  setPin(controlPin,0,0,0,0,1,1); //24,8dB  
  delay(sleepTime);
  setPin(controlPin,0,0,0,1,1,1); //28,4dB
  delay(sleepTime);
  setPin(controlPin,1,1,1,1,1,1); //32,0dB  
  delay(sleepTime);

  //down up
  setPin(controlPin,0,0,0,1,1,1); //28,4dB
  delay(sleepTime);
  setPin(controlPin,0,0,0,0,1,1); //24,8dB  
  delay(sleepTime);
  setPin(controlPin,0,0,0,1,0,1); //20,12dB 
  delay(sleepTime);
  setPin(controlPin,0,0,0,0,0,1); //16,16dB
  delay(sleepTime);
  setPin(controlPin,0,0,0,1,1,0); //12,20dB
  delay(sleepTime);
  setPin(controlPin,0,0,0,0,1,0); //8,24dB
  delay(sleepTime);
  setPin(controlPin,0,0,0,1,0,0); //4,28dB
  delay(sleepTime);
  setPin(controlPin,0,0,0,0,0,0); // 0,32dB 
  delay(sleepTime);  
}

void Throughput4dBstep(int sleepTime,String controlPin){
  //up down
  setPin(controlPin,0,0,0,0,0,0); // 0
  delay(sleepTime);  
  setPin(controlPin,0,0,0,1,0,0); //4
  delay(sleepTime);
  setPin(controlPin,0,0,0,0,1,0); //8
  delay(sleepTime);
  setPin(controlPin,0,0,0,1,1,0); //12
  delay(sleepTime);
  setPin(controlPin,0,0,0,0,0,1); //16
  delay(sleepTime);
  setPin(controlPin,0,0,0,1,0,1); //20
  delay(sleepTime);
  setPin(controlPin,0,0,0,0,1,1); //24
  delay(sleepTime);
  setPin(controlPin,0,0,0,1,1,1); //28
  delay(sleepTime);
  setPin(controlPin,1,1,1,1,1,1); //32 
  delay(sleepTime);
}

void setxdB(int att,String controlPin){
  switch(att){
    case 0: setPin(controlPin,0,0,0,0,0,0);break; //0
    case 4: setPin(controlPin,0,0,0,1,0,0);break; //4
    case 8: setPin(controlPin,0,0,0,0,1,0);break; //8
    case 12: setPin(controlPin,0,0,0,1,1,0);break; //12
    case 16: setPin(controlPin,0,0,0,0,0,1);break; //16
    case 20: setPin(controlPin,0,0,0,1,0,1);break; //20
    case 24: setPin(controlPin,0,0,0,0,1,1);break; //24
    case 28: setPin(controlPin,0,0,0,1,1,1);break; //28
    case 32: setPin(controlPin,1,1,1,1,1,1);break; //32
    default: Serial.print("cannot set loss");
  }
}
  
void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.print(&timeinfo, "%A, %B %d %Y %H:%M:%S,");
}

void udpLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time udp");
    return;
  }
  udp.print(&timeinfo, "%A, %B %d %Y %H:%M:%S,");
}
  
// Thanks to https://algorithm.joho.info/arduino/string-split-delimiter/
int split(String data, char delimiter, String *dst){
    //文字列配列の初期化
    for (int j=0; j<= sizeof(dst); j++){
      //Serial.println(dst(j));
      dst[j] = {""}; 
    }
    //msgs[3] = {"\0"};
  
    int index = 0;
    int arraySize = (sizeof(data)/sizeof((data)[0]));  
    int datalength = data.length();
    for (int i = 0; i < datalength; i++) {
        char tmp = data.charAt(i);
        if ( tmp == delimiter ) {
            index++;
            if ( index > (arraySize - 1)) return -1;
        }
        else dst[index] += tmp; //区切り文字が来るまで1Byteづつ連結
        //Serial.print("dbg: "+dst[index]);
    }
    return (index + 1);
}
