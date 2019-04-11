/*
.  To upload through terminal you can use: curl. -F "image=@firmware.bin" esp8266-webupdate.local/update.
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <IRrecv.h>
#include "DHTesp.h"
#include <Wire.h>
#include <BH1750.h>

String ESPName = "Irda";

BH1750 lightMeter;

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif


DHTesp dht;

int RECV_PIN = 14;        // the pin where you connect the output pin of IR sensor 
IRrecv irrecv(RECV_PIN);
decode_results results;

int i =0 ;
 
const char* host = "esp8266-webupdate";
//const char* ssid = "Blachotrapez";
//const char* password = "blachotrapez2016";
//const char* mqttServer = "rdzawka67.ddns.net";
const char* ssid = "D-Link1658";
const char* password = "RdzawkaPaSs67";
const char* mqttServer = "192.168.1.100";
const int mqttPort = 1883;

ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form> <h1>Simple NodeMCU Web Server IRDA</h1> <button id='reset' onclick=\"window.location.href='/reset'\">Reset</button> ";

void setup(void){
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");
  Serial.print("Połącz z: ");
  Serial.println(mqttServer);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if(WiFi.waitForConnectResult() == WL_CONNECTED){
    
    server.on("/", HTTP_GET, [](){
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", serverIndex);
    });   

    server.on("/reset", HTTP_GET, [](){
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", "ok");
      system_restart();
    });   
      
    server.on("/update", HTTP_POST, [](){
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
      ESP.restart();
    },[](){
      HTTPUpload& upload = server.upload();
      if(upload.status == UPLOAD_FILE_START){
        Serial.setDebugOutput(true);
        Serial.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)){//start with max available size
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_WRITE){
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
          Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){ //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      }
      yield();
    });
    server.begin(); 
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi Failed");
  }

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 


    while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect(ESPName.c_str())) {

      
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
 
    mqttSay("START", "Hello from "+ESPName);
    mqttSay("ip", WiFi.localIP().toString());
  irrecv.enableIRIn();
  dht.setup(12, DHTesp::DHT11); // Connect DHT sensor to GPIO 17
  Wire.begin(5,4);
  lightMeter.begin();

  Serial.println(F("BH1750 Test begin"));
}
 
void callback(char* topic, byte* payload, unsigned int length) {

}

void irloop(){
  if (irrecv.decode(&results)) 
    {
    unsigned int ircode = results.value;
    Serial.print("ircode: ");
    //Serial.println(ircode);
    char* str = "";
    sprintf(str,"%d",ircode);
    Serial.println("");
    checkMqtt();
    client.publish("/irda/ir", str);
    irrecv.resume();  // Receive the next value
    }
}

void dhtloop()
{
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();

  if(dht.getStatusString() == "OK"){
  char* str = "";
  sprintf(str,"%.0f",humidity);
  mqttSay("hum", str);
  str = "";
  sprintf(str,"%3.2f",temperature);
  mqttSay("temp", str);
  }else
  {
    Serial.print(dht.getStatusString()); 
  }
}

void lightloop(){
  float lux = lightMeter.readLightLevel();
  char*  str = "";
  sprintf(str,"%.2f",lux);
  mqttSay("light", str);
}

void checkMqtt(){
  if(!(WiFi.waitForConnectResult() == WL_CONNECTED) ) WIFI_Connect();
     while (!client.connected()) {
      Serial.println("Connecting to MQTT...");
 
     if (client.connect(ESPName.c_str())) {
 
      Serial.println("connected");  
      mqttSay("START", "Hello from "+ESPName);
      mqttSay("ip", WiFi.localIP().toString());
      return;
 
     } else {
      
      Serial.print("failed with state");
      Serial.print(client.state());
      
      delay(500);
      return;
 
    }
 }
}

void WIFI_Connect()
{
  digitalWrite(2,1);
  WiFi.disconnect();
  Serial.println("Booting Sketch...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
    // Wait for connection
  for (int i = 0; i < 25; i++)
  {
    if ( WiFi.status() != WL_CONNECTED ) {
      delay ( 250 );
      digitalWrite(2,0);
      Serial.print ( "." );
      delay ( 250 );
      digitalWrite(2,1);
    }
  }
  digitalWrite(2,0);
}

void mqttSay(String topic, String message)
  {
    client.publish(("/"+ESPName+"/"+topic).c_str(), message.c_str());
  }
 

void loop(void){
  server.handleClient();
  client.loop();
  irloop();
  if (i > 60000)
  {
    checkMqtt();
    dhtloop();
    lightloop();
    i = 0;
  }
  i++;
  delay(1);
}

