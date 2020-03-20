/*
.  To upload through terminal you can use: curl. -F "image=@firmware.bin" esp8266-webupdate.local/update.
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__/
  #include <avr/power.h>
#endif

String ESPName = "lamp";


#define PIN_SWITCH      4
#define PIN_LAMP        12

int lampa_switch = 0;
int lampa_real = 0;

int kierunek = 1; // 1 lub -1
int i = 0;
int przer = 0;

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            5

const char* host = "esp8266-webupdate";
//const char* ssid = "Blachotrapez";
//const char* password = "blachotrapez2016";
//const char* mqttServer = "rdzawka67.ddns.net";
const char* ssid = "MikroTik";
const char* password = "RdzawkaPaSs67";
const char* mqttServer = "192.168.88.14";
const int mqttPort = 1883;

ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);


int red = 0, green = 0, blue = 0;

void setup(void){
  Serial.begin(115200);
  pinMode(PIN_SWITCH, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_SWITCH), handleInterrupt, CHANGE);
  pinMode(PIN_LAMP, INPUT);
  digitalWrite( PIN_LAMP, LOW );
  Serial.println();
  Serial.println("Booting Sketch...");
  Serial.print("Połącz z: ");
  Serial.println(mqttServer);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if(WiFi.waitForConnectResult() == WL_CONNECTED){
    
    server.on("/", HTTP_GET, [](){
      server.sendHeader("Connection", "close");
      String mqtt_status ;
      if (client.connected()) mqtt_status= "Jest ok";
      else mqtt_status = "coś jest nie tak";
      
      String serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form> <h1>Simple NodeMCU Web Server lamp test</h1> MQTT: " + mqtt_status + " <br />lamp-switch: " + lampa_switch + " <br />lamp-real: " + lampa_real; 
      server.send(200, "text/html", serverIndex.c_str());
    });   

      server.on("/connect_mqtt", HTTP_GET, [](){
      Serial.println("Connecting to MQTT...");
 
     if (client.connect(ESPName.c_str())) {
 
      Serial.println("connected");  
      server.send(200, "text/html", "connected");
        mqttSay("START", "Hello from "+ESPName);
        mqttSay("ip", WiFi.localIP().toString());
        client.subscribe("/lamp/ster/lampa");
        client.subscribe("/lamp/led/#");
 
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
 
    } else {
 
      Serial.print("failed with state");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
  
    mqttSay("START", "Hello from "+ESPName);
    mqttSay("ip", WiFi.localIP().toString());
  client.subscribe("/lamp/ster/lampa");
}

  void mqttSay(String topic, String message)
  {
    client.publish(("/"+ESPName+"/"+topic).c_str(), message.c_str());
  }
 
 
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  

  if(!strcmp(topic,"/lamp/ster/lampa"))
  {
    if(message == "1"){
      pinMode( PIN_LAMP, OUTPUT ); // włączone
      mqttSay("ON","1");
      mqttSay("state/lampa","1");
      lampa_real = 1;
    }
    if(message == "0"){
      pinMode( PIN_LAMP, INPUT ); // wyłączone
      mqttSay("OFF","0");
      mqttSay("state/lampa","0");
      lampa_real = 0;
    }
  }

} 

void loop(void){
    if (i % 50 == 0 && przer == 1)  przerwanie();
    if (i > 60000)
    {
    checkMqtt();
    mqttSay("stan","OK");
    i = 0;
    }
   i++;
  server.handleClient();
  client.loop();
//  handleInterrupt();
  delay(10);
}

ICACHE_RAM_ATTR  void handleInterrupt() {
  przer = 1;
}

void przerwanie(){
  przer = 0;
  int suma = 0;
  for (int i = 0; i < 20; i++){
  //Serial.print(i);
  //Serial.print("    Digittal: ");
  int dig = digitalRead(PIN_SWITCH);
  suma += dig;
  }
  //Serial.println(suma);
  int stan = 0;
  if (suma < 20/3) stan = 1;
  if (stan == lampa_switch){
    return;
  }else{
    lampa_switch = stan;
    lampa_real = stan;
    Serial.println("Zmieniam stan");
  }

  if (lampa_real) {
    pinMode( PIN_LAMP, INPUT ); // wylączone
    mqttSay("state/lampa","0");
//    neopixelall(0,0,0);
   // Serial.println("Swiatlo off przerwanie");
  }else{
    pinMode( PIN_LAMP, OUTPUT ); // włączone
    mqttSay("state/lampa","1");
//    Serial.println("Swiatlo on przerwanie");
  }
}

void checkMqtt(){
  if(!(WiFi.waitForConnectResult() == WL_CONNECTED) ) WIFI_Connect();
     while (!client.connected()) {
      Serial.println("Connecting to MQTT...");
 
     if (client.connect(ESPName.c_str())) {
 
      Serial.println("connected");  
        mqttSay("START", "Hello from "+ESPName);
        mqttSay("ip", WiFi.localIP().toString());
        client.subscribe("/lamp/ster/lampa");
        client.subscribe("/lamp/led/#");
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
