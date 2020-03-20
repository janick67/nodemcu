/*
.  To upload through terminal you can use: curl. -F "image=@firmware.bin" esp8266-webupdate.local/update.
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

String ESPName = "lampTest";

#define PIN_SWITCH      0
#define PIN_LAMP        14
int lampa_switch = 0;
int lampa_real = 0;

int i = 0;
int przer = 0;


const char* host = "esp8266-webupdate";
//const char* ssid = "Blachotrapez";
//const char* password = "blachotrapez2016";
//const char* mqttServer = "rdzawka67.ddns.net";
const char* ssid = "D-Link1658";
const char* password = "RdzawkaPaSs67";

ESP8266WebServer server(80);
WiFiClient espClient;

int red = 0, green = 0, blue = 0;

void setup(void){
  pinMode(PIN_SWITCH, INPUT);
  //pinMode(PIN_LAMP, INPUT);
  pinMode( PIN_LAMP, OUTPUT ); // włączone

  digitalWrite( PIN_LAMP, LOW );
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");
  Serial.print("Połącz z: ");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if(WiFi.waitForConnectResult() == WL_CONNECTED){
    
    server.on("/", HTTP_GET, [](){
      server.sendHeader("Connection", "close");
      
      String serverIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form> <h1>Simple NodeMCU Web Server lamp</h1>"; 
      server.send(200, "text/html", serverIndex.c_str());
     });   

    server.on("/on", HTTP_GET, [](){
      server.sendHeader("Connection", "close");      
      String serverIndex = "ok"; 
      server.send(200, "text/html", serverIndex.c_str());
      lampa_switch = 1;
      lampa_real = 1;
      pinMode( PIN_LAMP, OUTPUT ); // włączone
     });   

     server.on("/off", HTTP_GET, [](){
      server.sendHeader("Connection", "close");      
      String serverIndex = "ok"; 
      server.send(200, "text/html", serverIndex.c_str());
      lampa_switch = 0;
      lampa_real = 0;
      pinMode( PIN_LAMP, INPUT ); // włączone
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
} 

void loop(void){
    if (i % 50 == 0 && przer == 1)  przerwanie();

  server.handleClient();
  handleInterrupt();
  delay(1);
}


void handleInterrupt() {
  przer = 1;
}

void przerwanie(){
  przer = 0;
  int suma = 0;
  for (int i = 0; i < 20; i++){
  int dig = digitalRead(PIN_SWITCH);
  suma += dig;
  }
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
  }else{
    pinMode( PIN_LAMP, OUTPUT ); // włączone
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
