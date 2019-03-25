/*
.  To upload through terminal you can use: curl. -F "image=@firmware.bin" esp8266-webupdate.local/update.
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
 
const char* host = "esp8266-webupdate";
const char* ssid = "Blachotrapez";
const char* password = "blachotrapez2016";
//const char* ssid = "D-Link1658";
//const char* password = "RdzawkaPaSs67";
const char* mqttServer = "rdzawka67.ddns.net";
const int mqttPort = 1883;

ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form> <h1>Simple NodeMCU Web Server test</h1>";

void setup(void){
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");
  Serial.print("Połącz z: ");
  Serial.println(mqttServer);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  if(WiFi.waitForConnectResult() == WL_CONNECTED){
    
    server.on("/", HTTP_GET, [](){
      server.sendHeader("Connection", "close");
      server.send(200, "text/html", serverIndex);
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
 
    if (client.connect("ESP8266Client")) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
 
  client.publish("esp/test", "Hello from ESP8266");
  client.subscribe("#");
 
}
 
void callback(char* topic, byte* payload, unsigned int length) {


  if(!strcmp(topic,"nodemcu/test"))
  {
    if((char)payload[0] == '1'){
      digitalWrite(LED_BUILTIN, LOW);
    }
    if((char)payload[0] == '0'){
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------");
 
}
 

void loop(void){
  server.handleClient();
  client.loop();
  delay(1);
}

