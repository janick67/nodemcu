//Stałe biblioteki
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <EEPROM.h>
//koniec stałe biblioteki

//stałe definicje
#define EEPROM_SIZE 1
#define EEPROM_MQTTSYSTEM 0

#ifdef __AVR__/
  #include <avr/power.h>
#endif
//konic stałe definicje


//dodatkowe biblioteki

//##CUSTOM_INCLUDES##

//koniec dodatkowe biblioteki

//dodatkowe deklaracje

//##CUSTOM_VARIABLE##

//nie zmienialny fragment kodu
//STAŁY FRAGMENT KODU
int i = 0;
void c_setup();
void c_loop();
void c_callback(char* topic, String message);
String c_getCurrentState();
int miliConnectedWifi = 0;
int miliConnectedMqtt = 0;
String message = "";
char json [300];
boolean isMqttSystem = true;

String shost = "esp-" + ESPName;
const char* host = shost.c_str();
const char* ssid = "MikroTik";
const char* password = "RdzawkaPaSs67";
const char* mqttServer = "192.168.88.100";
const int mqttPort = 1883;

ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);

void setup(void){
  ESP.wdtEnable(5000);
  c_setup();
  EEPROM.begin(EEPROM_SIZE);
  isMqttSystem = EEPROM.read(EEPROM_MQTTSYSTEM);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");
  Serial.print("Połącz z: ");
  Serial.println(mqttServer);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if(WiFi.waitForConnectResult() == WL_CONNECTED){
    miliConnectedWifi = millis();
    server.on("/", HTTP_GET, [](){
      server.sendHeader("Connection", "close");
      String mqtt_status ;
      if (client.connected()) mqtt_status= "Jest ok";
      else mqtt_status = "coś jest nie tak";
      
      String serverIndex = "<h1>"+ESPName+"</h1><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form> <h1>Simple NodeMCU Web Server lamp</h1> MQTT: " + mqtt_status + "<h2>state:</h2>" + getCurrentState() + getCurrentSystemState() + "isMqttSystem: " + isMqttSystem; 
      server.send(200, "text/html", serverIndex.c_str());
    });   

    server.on("/reboot", HTTP_GET, [](){
       server.send(200, "text/html", "ok, rebooting...");
       ESP.restart();
    });

    server.on("/mqttSystem/on", HTTP_GET, [](){
       server.send(200, "text/html", "ok, isMqttSystem = 1");
       turnMqttSystem(true); 
    });

    server.on("/mqttSystem/off", HTTP_GET, [](){
       server.send(200, "text/html", "ok, isMqttSystem = 0");
       turnMqttSystem(false); 
    });

    server.on("/connect_mqtt", HTTP_GET, [](){
      Serial.println("Connecting to MQTT...");
 
     if (client.connect(ESPName.c_str())) {
 
      Serial.println("connected");  
      server.send(200, "text/html", "connected");
        mqttSay("START", "Hello from "+ESPName);
        mqttSay("ip", WiFi.localIP().toString());
        client.subscribe(("/"+ESPName+"/#").c_str());
 
     } else {
      server.send(500, "text/html", (char*)client.state());
      Serial.print("failed with state");
      Serial.print(client.state());
 
    }
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
      miliConnectedMqtt = millis();
 
    } else {
 
      Serial.print("failed with state");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
  
  mqttSay("START", "Connected "+ESPName);
  mqttSay("ip", WiFi.localIP().toString());
  mqttSubscribe();
}

  void mqttSay(String topic, String message)
  {
    client.publish(("/"+ESPName+"/"+topic).c_str(), message.c_str());
  }

 void mqttSubscribe(){
  client.subscribe(("/"+ESPName+"/ster/#").c_str());
  client.subscribe("/mqttsystem/set");
 }
 
void callback(char* topic, byte* payload, unsigned int length) {
  message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  if(!strcmp(topic,"/mqttsystem/set")) turnMqttSystem(message == "1");
  Serial.println(message);
  c_callback(topic, message);
} 

void turnMqttSystem(boolean isSystem){
  if (isMqttSystem != isSystem){
    isMqttSystem = isSystem;
    EEPROM.write(EEPROM_MQTTSYSTEM, isMqttSystem);
    EEPROM.commit();
  }
}

  void sendCurrentState(){
    mqttSay("state",getCurrentState());
    if(isMqttSystem) mqttSay("system",getCurrentSystemState());
  }

String getCurrentSystemState(){
  sprintf (json, "{\"system\":{\"name\":\"%s\",\"time\":\"%d\",\"connectedWifi\":\"%d\",\"connectedMqtt\":\"%d\"}}", ESPName.c_str(), millis()/1000,getSecFromWifiCon(),getSecFromMqttCon());//, (millis/1000 - miliConnectedMqtt/1000));
  return (String) json;
}

String getCurrentState(){
  sprintf (json, "{\"custom\":%s}", c_getCurrentState().c_str());
  return (String) json;
}

int getSecFromWifiCon(){
  return (millis() - miliConnectedWifi)/1000;
}

int getSecFromMqttCon(){
  return (millis() - miliConnectedMqtt)/1000;
}
  
void loop(void){
  if (i > 60000){
    checkMqtt();
    i = 0;
  } 
  if (i % 10000 == 0){
    sendCurrentState();
  }
  c_loop();
  i++;
  ESP.wdtFeed();
  server.handleClient();
  client.loop();
  delay(1);
}

void checkMqtt(){
  if(!(WiFi.status() == WL_CONNECTED)) WIFI_Connect();
      while (!client.connected()) {
      Serial.println("ReConnecting to MQTT...");
 
     if (client.connect(ESPName.c_str())) {
 
      Serial.println("reconnected");  
      miliConnectedMqtt = millis();
        mqttSay("START", "reconnected "+ESPName);
        mqttSay("ip", WiFi.localIP().toString());
        mqttSubscribe();
      return;
     } else {
      Serial.print("failed with state");
      Serial.print(client.state());
      ESP.wdtFeed();
      delay(500);
      return;
 
    }
 }
}

void WIFI_Connect()
{
  WiFi.disconnect();
  Serial.println("Try connect..");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
    // Wait for connection
  for (int i = 0; i < 25; i++)
  {
    if ( WiFi.status() != WL_CONNECTED ) {
      delay ( 500 );
      ESP.wdtFeed();
      Serial.print ( "." );
    }
  }
  miliConnectedWifi = millis();
}

//KONIEC STAŁEGO FRAGMENTU

// deklaracja funckji użytkownika
//

//##CUSTOM_FUNCTIONS##

