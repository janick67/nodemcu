/*
.  To upload through terminal you can use: curl. -F "image=@firmware.bin" esp8266-webupdate.local/update.
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN_SWITCH      0
#define PIN_LAMP        14

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            12

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      60

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

 
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
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form> <h1>Simple NodeMCU Web Server test</h1>";

int red = 0, green = 0, blue = 0;

void setup(void){
  pinMode(PIN_SWITCH, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_SWITCH), handleInterrupt, CHANGE);
  pinMode(PIN_LAMP, INPUT);
  digitalWrite( PIN_LAMP, LOW );
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
  client.subscribe("/lamp/ster/lampa");
  client.subscribe("/lamp/led/#");
  pixels.begin(); // This initializes the NeoPixel library.

}
 
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  

  if(!strcmp(topic,"/lamp/ster/lampa"))
  {
    if(message == "1"){
      pinMode( PIN_LAMP, OUTPUT ); 
    }
    if(message == "0"){
      pinMode( PIN_LAMP, INPUT );
    }
  }

  if(!strcmp(topic,"/lamp/led/red"))
  {
    red = atoi(message.c_str());
  }

    if(!strcmp(topic,"/lamp/led/green"))
  {
    green = atoi(message.c_str());
  }

    if(!strcmp(topic,"/lamp/led/blue"))
  {
    blue = atoi(message.c_str());
    neopixelall(red,green,blue);
  }
 
 
//  Serial.print("Message arrived in topic: ");
//  Serial.println(topic);
// 
//  Serial.print("Message:");
//  for (int i = 0; i < length; i++) {
//    Serial.print((char)payload[i]);
//  }
// 
//  Serial.println();
//  Serial.println("-----------------------");
} 

void loop(void){
  server.handleClient();
  client.loop();
  delayMicroseconds(1000);
}


void handleInterrupt() {
  int suma;
  for (int i = 0; i < 20; i++){
  Serial.print(i);
  Serial.print("    Digittal: ");
  int dig = digitalRead(PIN_SWITCH);
  Serial.println(dig);
  suma += dig;
  }
  Serial.println("");
  Serial.println("");
  Serial.println("");
  if (suma > 20/3) {
    pinMode( PIN_LAMP, INPUT ); 
    client.publish("/lamp/state/lampa","0");
//    Serial.println("Swiatlo off przerwanie");
  }else{
    pinMode( PIN_LAMP, OUTPUT ); 
    client.publish("/lamp/state/lampa","1");
//    Serial.println("Swiatlo on przerwanie");
  }
}


void neopixelall(int red, int green, int blue){
  Serial.print("zmieniam kolor na: ");
  Serial.println(red);
  Serial.println(green);
  Serial.println(blue);
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, red,green,blue);
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
}

void neopixelallhex(String kolor){
  Serial.print("zmieniam kolor na: ");
  Serial.println(kolor);
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, strtoul (kolor.c_str(), NULL, 0));
  }
  pixels.show(); // This sends the updated pixel color to the hardware.
}

