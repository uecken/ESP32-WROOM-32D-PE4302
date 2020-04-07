#include <WiFi.h>
#include <WiFiUdp.h>
#include "time.h"

//WiFi
WiFiUDP udp;
const char* ssid       = "elecom2g-6c8919";
const char* password   = "lsnwple4aqr8";
const char * udpAddress = "192.168.2.145";
const int udpPort = 6543;
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

//Variable
int i = 0;
int j = 0;
int tpi = 0;
int hoi = 0;
String msgs[6] = {"\0"};
int interval,iteration;

void setup() {
    Serial.begin(115200);

//Pin Set to Digital
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
  }
  Serial.println(" CONNECTED");
  
//NTP sync
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

void loop() {
  if(Serial.available() > 0){
    String msg =  Serial.readString();
    Serial.print("Serial Input: " + msg);  //gain,phase
    int len = split(msg,',',msgs);
     
    if(msgs[0] == "ho" || msgs[0] == "handover"){
      //Input example: ho,2[s],999
      interval = msgs[1].toInt()*1000;
      iteration = msgs[2].toInt();
      for(j=0;j<iteration;j++){
        HandOver4dBstep(interval,"p18-12_and_p26-p14");

        udp.beginPacket(udpAddress,udpPort);
        udp.printf("Handover Iteration: %d", tpi++);
        udp.endPacket();
        Serial.println("UDP:Handover Iteration:" + String(tpi));
      }
    }
    else if(msgs[0] == "tp" || msgs[0] == "throughput"){
      //Input example: tp,30[s],1
      interval = msgs[1].toInt()*1000;
      //iteration = msgs[2].toInt();
      Throughput4dBstep(interval,"p18-12_and_p26-p14");

      udp.beginPacket(udpAddress,udpPort);
      udp.printf("Throughputr Iteration: %d", hoi++);
      udp.endPacket();
      Serial.println("UDP:Throughput Iteration:" + String(i));
    }
  }
  
  udp.beginPacket(udpAddress,udpPort);
  udp.printf("No: %d", i++);
  udp.endPacket();
  Serial.println("No: " + String(i));
  
  delay(3000);
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
  else if(pinArrange =="p18-12_and_p26-p14"){ 
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
}
  
void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
  
// Thanks to https://algorithm.joho.info/arduino/string-split-delimiter/
int split(String data, char delimiter, String *dst){
    //文字列配列の初期化
    for (int j=0; j< sizeof(dst); j++){
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
