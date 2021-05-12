/***************************************************
 2021-05-27
 EC631 ESP32 Final Project
 Board : "NodeMCU-32s"
 ****************************************************/
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#define LEDBLUE 2
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif


// Wifi Setup -------------------------------------
const char* ssid     = "ece631Lab";
const char* password = "esp32IOT!";
//const char* ssid     = "NETGEAR74";
//const char* password = "greenlotus261";

// Time Variables
unsigned long LEDMillis;
unsigned long LEDMillis2;
int oldTime = 0;

// JSON Payload Variables
String distPayload = "";
DynamicJsonDocument tempHall(1024);
DynamicJsonDocument tempUltra(1024);
DynamicJsonDocument tempFlashRate(1024);
DynamicJsonDocument docAccess(1024);

// Topics
const char *AccessTopic = "/ece631/FinalProject/Access";

// Hall Sensor Variables
int samplesHall;
int avgNumHall;
int hallValue = 0;
int HEvalue = 0;
const int NUMELEMENTSHALL = 15;
const int OFFSETELEMENTS = 5000;
int offsetReading = 0;
int movingArrayHall[NUMELEMENTSHALL];

// UltraSonic Sensor Variables
const int NUMELEMENTSULTRA = 5;
int PWM_FREQUENCY = 16;
int PWM_CHANNEL = 0;
int PWM_RESOLUTION = 8;
int dutyCycle = 1/256;
double sendDist = 0;
double movingArrayUltra[NUMELEMENTSULTRA];
int samplesUltra;
double avgNumUltra = 0;
double distance = 0;
double highTime = 0;
double lowTime = 0;
double width = 0;
int rate = 0;
int ultraTime = 0;
int hallTime = 0;
bool openState = false;
int stateTime = 0;

// Garage Door Variables
bool isOpen = false;
bool ThreeFactor = false;

// Basic Setup Variables
bool LEDState;
char serial[50];
IPAddress server(192,168,1,120);
WiFiClient wificlient;
PubSubClient client(wificlient);

void reconnect() {
  // Loop until we're reconnected
  client.setServer(server, 1883);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("codymorse/jackjohnson-Final")) {
      Serial.println("connected");
      client.subscribe("/ece631/FinalProject/Access");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void isr(){
 //Serial.println("test");
if(digitalRead(22) == HIGH){
  highTime = micros();
  }
else if(digitalRead(22) == LOW){
  lowTime = micros();
  }

 width = lowTime - highTime;
 
 distance =  ((width / 1000000) * 13503.9) / 2;
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.println();
  if(*topic == *AccessTopic)
  {
    deserializeJson(docAccess, payload);
    if(docAccess["Access"] = "Pass")
    {
      ThreeFactor = true;
    }
    else
    {
      ThreeFactor = false;
    }
  }
}

void setup() {
 Serial.begin(115200); 
 
 pinMode(23, INPUT_PULLUP);
 int PWM_FREQUENCY = 16;
 int PWM_CHANNEL = 0;
 int PWM_RESOLUTION = 8;
 int dutyCycle = 1;
 
 Serial.println("Awaiting 3FA On Other ESP32.");
 Serial.println("1. Bluetooth Verification");
 Serial.println("2. NFC Verification");
 Serial.println("3. Capacitive Touch Verification");
 Serial.println("");
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) 
 {
    delay(500);
    Serial.print(".");
 }
 Serial.println("");
 Serial.println("WiFi connected");
 Serial.println("IP address: ");
 Serial.println(WiFi.localIP());
 client.setCallback(callback);
 client.setServer(server,1883);
 reconnect();
 client.loop();
 pinMode(LEDBLUE, OUTPUT);
 digitalWrite(LEDBLUE, HIGH);
 delay(100);
 digitalWrite(LEDBLUE, LOW);
 delay(100);
 Serial.setTimeout(100);//0.1 second timeout
 delay(2500);
 
 Serial.println("ESP32 Final Project");
 for(int i = 0; i < OFFSETELEMENTS; i++)
 {
    offsetReading = offsetReading + hallRead();
 }
 offsetReading = offsetReading / OFFSETELEMENTS;
 Serial.print("offset: ");
 Serial.println(offsetReading);
 LEDMillis = millis();
 oldTime = millis();
 ultraTime = millis();
 stateTime = millis();
 
 ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
 ledcAttachPin(23, PWM_CHANNEL);
 attachInterrupt(digitalPinToInterrupt(22), isr, CHANGE);
 ledcWrite(PWM_CHANNEL, dutyCycle);
 delay(2000);
 sendHallMessage(2);
 sendZeroDistance();
}

void getLEDRate(double tempDistance)
{
    rate = tempDistance * 5;
}

void sendHallMessage(int choice)
{
    if(choice == 1)
    {
       tempHall["Switch"] = "OPEN";
       stateTime = millis();
       sendZeroDistance();
       getLEDRate(sendDist);
    }
    else if(choice == 2)
    {
       tempHall["Switch"] = "CLOSED";
       stateTime = millis();
       sendZeroDistance();
       
    }
    else if(choice == 3)
    {
       tempHall["Switch"] = "OPENING";
       sendZeroDistance();
    }
    else if(choice == 4)
    {
       tempHall["Switch"] = "CLOSING";
       sendZeroDistance();
    }
    tempHall["Value"] = HEvalue;
    serializeJson(tempHall, distPayload);
    distPayload.toCharArray(serial, 50);
    client.publish("/ece631/FinalProject/Hall", serial);
    tempHall.clear();
    distPayload = "";
    
}

void sendZeroDistance()
{
     tempUltra["Distance"] = 0;
     tempUltra["Units"] = "Inches";
     serializeJson(tempUltra, distPayload);
     distPayload.toCharArray(serial, 50);
     client.publish("/ece631/FinalProject/Distance", serial);
     distPayload = "";
     delay(20);
     tempFlashRate["FlashRate"] = 0;
     serializeJson(tempFlashRate, distPayload);
     distPayload.toCharArray(serial, 50);
     client.publish("/ece631/FinalProject/FlashRate", serial);
     distPayload = "";
     tempFlashRate.clear();
     tempUltra.clear();
}

void sendDistance()
{
   tempUltra["Distance"] = sendDist;
   tempUltra["Units"] = "Inches";
   tempFlashRate["FlashRate"] = rate;
   serializeJson(tempUltra, distPayload);
   distPayload.toCharArray(serial, 50);
   client.publish("/ece631/FinalProject/Distance", serial);
   tempUltra.clear();
   distPayload = "";
   delay(20);
   serializeJson(tempFlashRate, distPayload);
   distPayload.toCharArray(serial, 50);
   client.publish("/ece631/FinalProject/FlashRate", serial);
   tempFlashRate.clear();
   distPayload = "";
}
void loop() 
{
  client.loop(); 
  if(ThreeFactor == true)
  {     
     hallValue = hallRead() - offsetReading;
     movingArrayHall[samplesHall] = hallValue;
     samplesHall++;
    
     if(avgNumHall < NUMELEMENTSHALL)
       avgNumHall++;
     
     if(samplesHall >= NUMELEMENTSHALL)
       samplesHall = 0;
    
     for(int i = 0; i < avgNumHall; i++)
     {
       HEvalue = HEvalue + movingArrayHall[i];
     }
     HEvalue = HEvalue / avgNumHall;
            
     movingArrayUltra[samplesUltra] = distance;
     samplesUltra++;
  
     if(avgNumUltra < NUMELEMENTSULTRA)
       avgNumUltra++;
   
     if(samplesUltra >= NUMELEMENTSULTRA)
       samplesUltra = 0;
  
     for(int i = 0; i < avgNumUltra; i++)
     {
       sendDist = sendDist + movingArrayUltra[i];
     }
     sendDist = sendDist / avgNumUltra;


     if(millis() - ultraTime >= 1000)
     {
       if(openState == true)
       {
         ultraTime = millis();
         getLEDRate(sendDist);
         if(sendDist < 0)
         {
            sendZeroDistance();
            
         }
         else
         {
            sendDistance();           
         }        
       }
     }

     if(millis() - LEDMillis2 >= rate)
     {
        if(openState == true)
        {
            LEDMillis2 = millis();
            LEDState = LEDState ^ HIGH;
            digitalWrite(LEDBLUE, LEDState);
        }
        else
        {
            digitalWrite(LEDBLUE, LOW);
        }

     }
     
     if(millis() - LEDMillis >= 1000)
     {
      LEDMillis = millis();
      hallTime = millis();
      Serial.println(HEvalue);
      Serial.println(sendDist);
      tempHall["Value"] = HEvalue;
      serializeJson(tempHall, distPayload);
      distPayload.toCharArray(serial, 50);
      client.publish("/ece631/FinalProject/Hall", serial);
      tempHall.clear();
      distPayload = "";
      
      if(HEvalue > 5)
      {
        if(openState == false)
        {
           if(millis()-stateTime >= 5000)
           {
             sendHallMessage(3);
             while(openState == false)
             {
                if(millis() - hallTime >= 15000)
                {
                    openState = true;
                    sendHallMessage(1);
                }
                
             }
           }
        }
      }
      else if(HEvalue < -5)
      {
        if(openState == true)
        {
           if(millis()-stateTime >= 5000)
           {
             sendHallMessage(4);
             while(openState == true)
             {
                if(millis() - hallTime >= 15000)
                {
                    openState = false;
                    sendHallMessage(2);
                }
                
             }
           }
        }
      }      
     }   
  }  
}
