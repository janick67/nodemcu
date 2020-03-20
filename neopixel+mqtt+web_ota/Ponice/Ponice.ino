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

String ESPName = "pokojLedy";

int kierunek = 1; // 1 lub -1
int i = 0;

#define PIN            5

#define NUMPIXELS      120

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

 
const char* host = "esp8266-webupdate";
//const char* ssid = "Blachotrapez";
//const char* password = "blachotrapez2016";
//const char* mqttServer = "rdzawka67.ddns.net";
const char* ssid = "MikroTik";
const char* password = "RdzawkaPaSs67";
const char* mqttServer = "192.168.88.100";
const int mqttPort = 1883;

ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);


int red = 0, green = 0, blue = 0;

void setup(void){
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
      String mqtt_status ;
      if (client.connected()) mqtt_status= "Jest ok";
      else mqtt_status = "coś jest nie tak";
      
      String serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form> <h1>Simple NodeMCU Web Server lamp</h1> MQTT: " + mqtt_status + "<br />red = " + red + "<br />green = " + green + "<br />blue = " + blue; 
      server.send(200, "text/html", serverIndex.c_str());
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
 
    } else {
 
      Serial.print("failed with state");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
  
  mqttSay("START", "Hello from "+ESPName);
  mqttSay("ip", WiFi.localIP().toString());
  client.subscribe(("/"+ESPName+"/#").c_str());
  pixels.begin(); // This initializes the NeoPixel library.
  neopixelall(0,0,0);

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

//  if(!strcmp(topic,"/lamp/led/color"))
//  {
//    neopixelallhex(message);
//  }

  if(!strcmp(topic,("/"+ESPName+"/red").c_str()))
  {
    red = atoi(message.c_str());
  }

    if(!strcmp(topic,("/"+ESPName+"/green").c_str()))
  {
    green = atoi(message.c_str());
  }

    if(!strcmp(topic,("/"+ESPName+"/blue").c_str()))
  {
    blue = atoi(message.c_str());
    neopixelall(red,green,blue);
    mqttSay("kolor","Zmieniam kolor");
  }
  //mqttSay("kolor",message.c_str());

} 

void loop(void){
  if (i > 60000){
    checkMqtt();
    mqttSay("stan","OK");
    i = 0;
  }
  i++;
  server.handleClient();
  client.loop();
  delay(1);
}




void neopixelall(int red, int green, int blue){
  if (kierunek > 0){
    wprawo(red,green,blue);
  }else{
    wlewo(red,green,blue);
  }
  kierunek *= -1;
}

void wlewo(int red, int green, int blue){
  for(int i=NUMPIXELS-1;i>=0;i--){
    pixels.setPixelColor(i, red,green,blue);        // zgodnie ze wskazówkami zegara
    delay(5);
      pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

void wprawo(int red, int green, int blue){
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, red,green,blue);                                // zgodnie ze wskazówkami zegara
    delay(5);
      pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

void neopixelallhex(String kolor){
  if (kierunek > 0){
    wprawohex(kolor);
  }else{
    wlewohex(kolor);
  }
  kierunek *= -1;
}

void wlewohex(String kolor){
  for(int i=NUMPIXELS-1;i>=0;i--){
    pixels.setPixelColor(i, strtoul (kolor.c_str(), NULL, 0));       // zgodnie ze wskazówkami zegara
    delay(5);
      pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

void wprawohex(String kolor){
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, strtoul (kolor.c_str(), NULL, 0));                              // zgodnie ze wskazówkami zegara
    delay(5);
      pixels.show(); // This sends the updated pixel color to the hardware.
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
        client.subscribe(("/"+ESPName+"/#").c_str());
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
