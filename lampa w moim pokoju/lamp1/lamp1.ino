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

#define PIN_SWITCH      0
#define PIN_LAMP        14
int lampa_switch = 0;
int lampa_real = 0;

int kierunek = 1; // 1 lub -1



// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            12

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      119

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
const char* serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form> <h1>Simple NodeMCU Web Server test2</h1>";

int red = 0, green = 0, blue = 0;

void setup(void){
  pinMode(PIN_SWITCH, INPUT);
  //attachInterrupt(digitalPinToInterrupt(PIN_SWITCH), handleInterrupt, CHANGE);
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
 
    if (client.connect("Lamp1")) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
 
  client.publish("esp/test", "Hello from Lamp1");
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
      pinMode( PIN_LAMP, OUTPUT ); // włączone
      client.publish("/nodemcu/lampa","Włączyłem");
      lampa_real = 1;
    }
    if(message == "0"){
      pinMode( PIN_LAMP, INPUT ); // wyłączone
      client.publish("/nodemcu/lampa","Wyłączyłem");
      lampa_real = 0;
    }
  }

  if(!strcmp(topic,"/lamp/led/color"))
  {
    neopixelallhex(message);
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
    client.publish("/nodemcu/kolor","Zmieniam kolor");
    Serial.print("Zmieniam kolor na: ");
    Serial.print(red);
    Serial.print(" ");
    Serial.print(green);
    Serial.print(" ");
    Serial.println(blue);
  }
  client.publish("/nodemcu/kolor",message.c_str());
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);
  
} 

void loop(void){
  server.handleClient();
  client.loop();
  handleInterrupt();
  delay(1);
}


void handleInterrupt() {
  int suma;
  for (int i = 0; i < 10; i++){
  //Serial.print(i);
  //Serial.print("    Digittal: ");
  int dig = digitalRead(PIN_SWITCH);
 // Serial.println(dig);
  suma += dig;
  }
  int stan = 0;
  if (suma < 10/3) stan = 1;
  if (stan == lampa_switch){
    return;
  }else{
    lampa_switch = stan;
    lampa_real = stan;
    Serial.println("Zmieniam stan");
  }

  if (lampa_real) {
    pinMode( PIN_LAMP, INPUT ); // wylączone
    client.publish("/lamp/state/lampa","0");
    neopixelall(0,0,0);
    Serial.println("Swiatlo off przerwanie");
  }else{
   // neopixelall(30,255,0);
    pinMode( PIN_LAMP, OUTPUT ); // włączone
    client.publish("/lamp/state/lampa","1");
//    Serial.println("Swiatlo on przerwanie");
  }
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

