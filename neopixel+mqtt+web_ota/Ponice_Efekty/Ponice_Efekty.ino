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

String effect = "";

#define PIN            5

#define NUM_LEDS      120

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

 
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
      serverIndex += "<br /> <h1>Effects</h1> <br /> <a href='/fire'>Fire</a><br /> <a href='/rainbow'>rainbow</a><br /> <a href='/runing'>runing</a><br /> <a href='/theaterChase'>theaterChase</a><br /> <a href='/meteor'>meteor</a><br /> <a href='/bouncingBall'>bouncingBall</a><br /> <a href='/noeffect'>Stop</a>";
      server.send(200, "text/html", serverIndex.c_str());
    });   

    server.on("/fire", HTTP_GET, [](){
      effect = "fire";
      server.send(200, "text/html", effect.c_str());
      server.sendHeader("Connection", "close");
    });  

    server.on("/rainbow", HTTP_GET, [](){
      effect = "rainbow";
      server.send(200, "text/html", effect.c_str());
      server.sendHeader("Connection", "close");
    });  

    server.on("/runing", HTTP_GET, [](){
      effect = "runing";
      server.send(200, "text/html", effect.c_str());
      server.sendHeader("Connection", "close");
    });  
    
    server.on("/theaterChase", HTTP_GET, [](){
      effect = "theaterChase";
      server.send(200, "text/html", effect.c_str());
      server.sendHeader("Connection", "close");
    }); 

     server.on("/meteor", HTTP_GET, [](){
      effect = "meteor";
      server.send(200, "text/html", effect.c_str());
      server.sendHeader("Connection", "close");
    }); 
    
     server.on("/bouncingBall", HTTP_GET, [](){
      effect = "bouncingBall";
      server.send(200, "text/html", effect.c_str());
      server.sendHeader("Connection", "close");
    }); 
    
    server.on("/noeffect", HTTP_GET, [](){
      effect = "";
      neopixelall(0,0,0);
      String text = "no Effect";
      server.send(200, "text/html", text.c_str());
      server.sendHeader("Connection", "close");
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
  if (i > 10000){
    checkMqtt();
    mqttSay("stan","OK");
    i = 0;
  }
  if (effect.length() > 0){
    doEffect();
  }
  
  i++;
  server.handleClient();
  client.loop();
  delay(1);
}

void doEffect(){
  checkMqtt();
  mqttSay("stan","OK");
  if (effect.equals("fire")){
    Fire(55,120,15);
  }else if (effect.equals("rainbow")){
    rainbowCycle(1);
  }else if (effect.equals("runing")){
    RunningLights(red,green,blue, 10);
  }else if (effect.equals("theaterChase")){
    theaterChase(red,green,blue, 10);
  }else if (effect.equals("meteor")){
    meteorRain(red,green,blue, 10, 64, true, 30);
  }else if (effect.equals("bouncingBall")){
     BouncingBalls(red,green,blue, 4);
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
  for(int i=NUM_LEDS-1;i>=0;i--){
    pixels.setPixelColor(i, red,green,blue);        // zgodnie ze wskazówkami zegara
    delay(5);
      pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

void wprawo(int red, int green, int blue){
  for(int i=0;i<NUM_LEDS;i++){
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
  for(int i=NUM_LEDS-1;i>=0;i--){
    pixels.setPixelColor(i, strtoul (kolor.c_str(), NULL, 0));       // zgodnie ze wskazówkami zegara
    delay(5);
      pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

void wprawohex(String kolor){
  for(int i=0;i<NUM_LEDS;i++){
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


void Fire(int Cooling, int Sparking, int SpeedDelay) {
  static byte heat[NUM_LEDS];
  int cooldown;
 
  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_LEDS; i++) {
    cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);
   
    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }
 
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
   
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < Sparking ) {
    int y = random(7);
    heat[y] = heat[y] + random(160,255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < NUM_LEDS; j++) {
    setPixelHeatColor(j, heat[j] );
  }

  showStrip();
  delay(SpeedDelay);
}

void setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);
 
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    setPixel(Pixel, 255, 255, heatramp);
  } else if( t192 > 0x40 ) {             // middle
    setPixel(Pixel, 255, heatramp, 0);
  } else {                               // coolest
    setPixel(Pixel, heatramp, 0, 0);
  }
}



void rainbowCycle(int SpeedDelay) {
  byte *c;
  uint16_t i, j;

  for(j=0; j<256; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< NUM_LEDS; i++) {
      c=Wheel(((i * 256 / NUM_LEDS) + j) & 255);
      setPixel(i, *c, *(c+1), *(c+2));
    }
    showStrip();
    delay(SpeedDelay);
  }
}

byte * Wheel(byte WheelPos) {
  static byte c[3];
 
  if(WheelPos < 85) {
   c[0]=WheelPos * 3;
   c[1]=255 - WheelPos * 3;
   c[2]=0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   c[0]=255 - WheelPos * 3;
   c[1]=0;
   c[2]=WheelPos * 3;
  } else {
   WheelPos -= 170;
   c[0]=0;
   c[1]=WheelPos * 3;
   c[2]=255 - WheelPos * 3;
  }

  return c;
}


void RunningLights(byte red, byte green, byte blue, int WaveDelay) {
  int Position=0;
 
  for(int j=0; j<NUM_LEDS; j++)
  {
      Position++; // = 0; //Position + Rate;
      for(int i=0; i<NUM_LEDS; i++) {
        // sine wave, 3 offset waves make a rainbow!
        //float level = sin(i+Position) * 127 + 128;
        //setPixel(i,level,0,0);
        //float level = sin(i+Position) * 127 + 128;
        setPixel(i,((sin(i+Position) * 127 + 128)/255)*red,
                   ((sin(i+Position) * 127 + 128)/255)*green,
                   ((sin(i+Position) * 127 + 128)/255)*blue);
      }
     
      showStrip();
      delay(WaveDelay);
  }
}



void theaterChase(byte red, byte green, byte blue, int SpeedDelay) {
  for (int j=0; j<1; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < NUM_LEDS; i=i+3) {
        setPixel(i+q, red, green, blue);    //turn every third pixel on
      }
      showStrip();
     
      delay(SpeedDelay);
     
      for (int i=0; i < NUM_LEDS; i=i+3) {
        setPixel(i+q, 0,0,0);        //turn every third pixel off
      }
    }
  }
}



void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  setAll(0,0,0);
 
  for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {
   
   
    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(j, meteorTrailDecay );        
      }
    }
   
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        setPixel(i-j, red, green, blue);
      }
    }
   
    showStrip();
    delay(SpeedDelay);
  }
}

void fadeToBlack(int ledNo, byte fadeValue) {
    // NeoPixel
    uint32_t oldColor;
    uint8_t r, g, b;
    int value;
   
    oldColor = pixels.getPixelColor(ledNo);
    r = (oldColor & 0x00ff0000UL) >> 16;
    g = (oldColor & 0x0000ff00UL) >> 8;
    b = (oldColor & 0x000000ffUL);

    r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
    g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
    b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
   
    pixels.setPixelColor(ledNo, r,g,b);
}




void BouncingBalls(byte red, byte green, byte blue, int BallCount) {
  float Gravity = -9.81;
  int StartHeight = 1;
 
  float Height[BallCount];
  float ImpactVelocityStart = sqrt( -2 * Gravity * StartHeight );
  float ImpactVelocity[BallCount];
  float TimeSinceLastBounce[BallCount];
  int   Position[BallCount];
  long  ClockTimeSinceLastBounce[BallCount];
  float Dampening[BallCount];
 
  for (int i = 0 ; i < BallCount ; i++) {  
    ClockTimeSinceLastBounce[i] = millis();
    Height[i] = StartHeight;
    Position[i] = 0;
    ImpactVelocity[i] = ImpactVelocityStart;
    TimeSinceLastBounce[i] = 0;
    Dampening[i] = 0.90 - float(i)/pow(BallCount,2);
  }

  while (true) {
    for (int i = 0 ; i < BallCount ; i++) {
      TimeSinceLastBounce[i] =  millis() - ClockTimeSinceLastBounce[i];
      Height[i] = 0.5 * Gravity * pow( TimeSinceLastBounce[i]/1000 , 2.0 ) + ImpactVelocity[i] * TimeSinceLastBounce[i]/1000;
 
      if ( Height[i] < 0 ) {                      
        Height[i] = 0;
        ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
        ClockTimeSinceLastBounce[i] = millis();
 
        if ( ImpactVelocity[i] < 0.01 ) {
          ImpactVelocity[i] = ImpactVelocityStart;
        }
      }
      Position[i] = round( Height[i] * (NUM_LEDS/2 - 1) / StartHeight);
    }
 
    for (int i = 0 ; i < BallCount ; i++) {
      setPixel(Position[i],red,green,blue);
    }
   
    showStrip();
    setAll(0,0,0);
  }
}




void showStrip() {
   pixels.show();
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
   pixels.setPixelColor(Pixel, pixels.Color(red, green, blue));
}

void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue); 
  }
  showStrip();
}
