//##CUSTOM_INCLUDES##

//##CUSTOM_INCLUDES_END##

//##CUSTOM_VARIABLE##
#define PIN_DOOR 14
#define PIN_SENSOR 16

int lock = 0;
int sensor = 0;
//musi istnieć w każdym, tylko wartosc można zmieniać
String ESPName = "door";
//##CUSTOM_VARIABLE_END##


//##CUSTOM_FUNCTIONS##
String c_getCurrentState(){
  sprintf (json, "{\"lock\":\"%d\",\"sensor\":\"%d\"}", lock, sensor);
  return (String) json;
}

void c_setup(){
  c_turnOff();
  pinMode(PIN_SENSOR, INPUT_PULLDOWN_16 );
}

void c_loop(){
  if (i % 1000){
    int sens = digitalRead(PIN_SENSOR);
    if (sens != sensor){
      sensor = sens;
      mqttSay("sensorState",(String)sensor);
    }
  }
}

void c_callback(char* topic, String message){
  if(!strcmp(topic,"/door/ster"))
  {
    int mi = millis();
     if(message == "1"){
        c_turnOn();
      }
      if(message == "0"){
        c_turnOff();
      }
  }
}

void c_turnOn(){
  pinMode( PIN_DOOR, OUTPUT ); // włączone
  lock = 1;
  mqttSay("state","1");
  Serial.print("on ");
}

void c_turnOff(){
  pinMode( PIN_DOOR, INPUT ); // wyłączone
  lock = 0;
  mqttSay("state","0");
  Serial.print("off");
}
//##CUSTOM_FUNCTIONS_END##
