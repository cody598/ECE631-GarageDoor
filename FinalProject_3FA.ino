 /***************************************************
 2021-04-28
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

// JSON Payload Variables
String distPayload = "";
DynamicJsonDocument docAccess(1024);
DynamicJsonDocument docNFC(1024);
DynamicJsonDocument docPassword(1024);

// Topics
const char *NFCTopic = "/ece631/FinalProject/UID";

// Capacitive Touch Variables
char code[] = "";
const char tempPassCode[] = "4452";
int threshold = 40;
int codeCount = 0;
bool correct = false;
bool touch0detect = false;
bool touch1detect = false;
bool touch2detect = false;
bool touch3detect = false;
bool touch4detect = false;
bool touch5detect = false;
bool touch6detect = false;
bool touch7detect = false;


// NFC Variables
String UID = "";
String temp = "";
bool UIDAccess = false;
int verifyCount = 0;
bool NFCAccess = false;
int NFCcount = 0;
bool NFCReceived = false;
bool NFCfail = false;
int touchCount = 0;
bool touchAccess = false;

// Bluetooth Setup
BluetoothSerial SerialBT;
int deviceConnected = 0;
const char PassCode[] = "4452";
bool BluetoothAccess = false;
int charCount = 0;
bool correctCode = false;
int passwordAccess = true;
bool passcodeReceived = false;
int receivedPassCount = 0;

// Garage Door Variables
bool ThreeFactor = false;

// Basic Setup Variables
char serial[50];
IPAddress server(192,168,1,120);
WiFiClient wificlient;
PubSubClient client(wificlient);
int timer = 0;
int mainPassSends = 0;
int timeBetweenAccess = 0;
int timeSend = 0;

void reconnect() {
  // Loop until we're reconnected
  client.setServer(server, 1883);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("codymorse/jackjohnson3FA")) {
      Serial.println("connected");
      client.subscribe("/ece631/FinalProject/UID");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  if(*topic == *NFCTopic)
  {
    deserializeJson(docNFC, payload);
    NFCcount = 0;
    NFCReceived = true;
  }
}

void wifiSetup()
{
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
}

void bluetoothVerification()
{
 Serial.println("1. Bluetooth Verification");
 Serial.println("2. NFC Verification");
 Serial.println("3. Capacitive Touch Verification");
 Serial.println("");
 Serial.println("Awaiting Bluetooth Verification, please enter 4-digit code in bluetooth terminal! Assume if no serial output, then the access code is wrong. Must be reentered.");
 client.loop();

 while(deviceConnected == 0)
 {  
    if(SerialBT.available()) 
    {   
        for(int i = 0; i < 4; i++)
        {
          if(SerialBT.read() == PassCode[i])
          {
             if(i == 3)
             {
               Serial.println();
               Serial.println("Bluetooth Access Granted");
               BluetoothAccess = true;
               deviceConnected += 1;
             }
          }
        } 
    }
 }
}

void setup() {
 timer = millis();
 timeBetweenAccess = millis();
 timeSend = millis();
 Serial.begin(115200);
 Serial.println("Press the button to select the next mode");
 pinMode(LEDBLUE, OUTPUT);
 Serial.println("Login Process Started, 3FA required to proceed. Onboard LED will turn on if access is granted. Below are the listed requirements in order:");
 client.loop();
 SerialBT.begin("Cody/Jack ESP32"); //Bluetooth device name
 bluetoothVerification();
 wifiSetup();
 client.loop();
 pinMode(LEDBLUE, OUTPUT);
 digitalWrite(LEDBLUE, HIGH);
 delay(100);
 digitalWrite(LEDBLUE, LOW);
 delay(100);
 Serial.setTimeout(100);//0.1 second timeout
 delay(2500);
 Serial.println("ESP32 Final Project");
 touchAttachInterrupt(T0, gotTouch0, threshold);
 touchAttachInterrupt(T3, gotTouch1, threshold);
 touchAttachInterrupt(T4, gotTouch2, threshold);
 touchAttachInterrupt(T5, gotTouch3, threshold);
 touchAttachInterrupt(T6, gotTouch4, threshold);
 touchAttachInterrupt(T7, gotTouch5, threshold);
 touchAttachInterrupt(T8, gotTouch6, threshold);
 touchAttachInterrupt(T9, gotTouch7, threshold);
}

void gotTouch0()
{
   touch0detect = true;
}
void gotTouch1()
{
   touch1detect = true;
}
void gotTouch2()
{
   touch2detect = true;
}
void gotTouch3()
{
   touch3detect = true;
}
void gotTouch4()
{
   touch4detect = true;
}
void gotTouch5()
{
   touch5detect = true;
}
void gotTouch6()
{
   touch6detect = true;
}
void gotTouch7()
{
   touch7detect = true;
}

void remainingVerification()
{
  if(verifyCount == 0 && BluetoothAccess == true)
  {
    delay(30);
    client.publish("/ece631/FinalProject/BluetoothAccess", "{\"Access\":\"Pass\"}");
    client.publish("/ece631/FinalProject/NFCAccess", "{\"Access\":\"Denied\"}");  
    
    Serial.println("Awaiting NFC Verification, use NFC card to verify UID");
    while(verifyCount == 0)
    {
      client.loop();
      if(NFCReceived == true)
      {
            if(docNFC["ID"] == "Pass")          
            {
               NFCAccess = true;
               NFCReceived = false;
               Serial.println("NFC Access Granted");
               delay(30);
               client.publish("/ece631/FinalProject/NFCAccess", "{\"Access\":\"Pass\"}");
               Serial.println("");
               verifyCount += 1; 
               break; 
            }
            else if(docNFC["UID"] != "Pass")
            {
               Serial.println("NFC Access Denied");  
               Serial.println("");
               NFCcount += 1;  
               NFCReceived = false;       
            }
                
      }
    }
  }
  if(touchCount == 0 && NFCAccess == true)
  {
    Serial.println("Awaiting Touch Capacitive Verification, please enter 4-digit code using the four wires! Numbers are 0-7");
    Serial.println("Wait 10 seconds before starting");
    client.publish("/ece631/FinalProject/TouchAccess", "{\"Access\":\"Denied\"}");  
    delay(1000);
    while(touchCount == 0)
    {
        if(touch0detect)
        {
            code[codeCount] = '0';
            touch0detect = false;
            Serial.println("Entered: 0");
            codeCount++;
        }
        else if(touch1detect)
        {
            code[codeCount] = '1';
            touch1detect = false;
            Serial.println("Entered: 1");
            codeCount++;
 
        }
        else if(touch2detect)
        {
            code[codeCount] = '2';
            touch2detect = false;
            Serial.println("Entered: 2");
            codeCount++;
        }   
        else if(touch3detect)
        {
            code[codeCount] = '3';
            touch3detect = false;
            Serial.println("Entered: 3");
            codeCount++;   
        }
        else if(touch4detect)
        {
            code[codeCount] = '4';
            touch4detect = false;
            Serial.println("Entered: 4");
            codeCount++;
        }
        else if(touch5detect)
        {
            code[codeCount] = '5';
            touch5detect = false;
            Serial.println("Entered: 5");
            codeCount++;       
        }
        else if(touch6detect)
        {
            code[codeCount] = '6';
            touch6detect = false;
            Serial.println("Entered: 6");
            codeCount++;   
        }
        else if(touch7detect)
        {    
            code[codeCount] = '7';
            touch7detect = false;
            Serial.println("Entered: 7");
            codeCount++;      
        }
        delay(2000);  
        if(codeCount == 4)
        {
          correct = true;
          for(int i = 0; i < 4; i++)
          {
            if(code[i] != tempPassCode[i])
            {
              correct = false;
            }
          }
          if(correct == true)
          {
             ThreeFactor = true;
             timeBetweenAccess = millis();
             delay(1000);
             client.publish("/ece631/FinalProject/TouchAccess", "{\"Access\":\"Pass\"}"); 
             delay(1000);
             client.publish("/ece631/FinalProject/Access", "{\"Access\":\"Pass\"}");
             touchCount++;  
             Serial.println("Touch Password Entered Correctly");
          }
          else
          {
              Serial.println("Touch Password Entered Incorrectly. Please Restart");
              delay(30);
              client.publish("/ece631/FinalProject/TouchAccess", "{\"Access\":\"Denied\"}"); 
              codeCount = 0; 
          }         
        }
      }  
    }
}

void loop() 
{
  client.loop();
  if(ThreeFactor != true)
  {
     remainingVerification();
  }
  else
  {
     if(millis() - timeSend >= 10000)
     {
        timeSend = millis();
        delay(30);
        client.publish("/ece631/FinalProject/Access", "{\"Access\":\"Pass\"}");
        digitalWrite(LEDBLUE, HIGH);
        Serial.println("User Gained Access");
        Serial.println();
        docAccess.clear();
        distPayload = "";
     }
     if(millis() - timeBetweenAccess >= 900000)
     {
       delay(30);
       client.publish("/ece631/FinalProject/Access", "{\"Access\":\"Denied\"}");
       client.publish("/ece631/FinalProject/NFCAccess", "{\"Access\":\"Denied\"}");
       client.publish("/ece631/FinalProject/TouchAccess", "{\"Access\":\"Denied\"}");
       digitalWrite(LEDBLUE, LOW);
       Serial.println("User Needs to Re-verify. NFC and Touch-Capacitive Password Will be required.");
       verifyCount = 0;
       touchCount = 0;
       codeCount = 0;
       ThreeFactor = false;   
       NFCAccess = false;
       remainingVerification();
     }
  }
}
