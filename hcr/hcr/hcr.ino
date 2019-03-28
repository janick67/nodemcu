#define PIR_PIN D6
 
void setup(){
  Serial.begin(9600);
  pinMode(PIR_PIN, INPUT);
}
 
void alarm(){
  //add alarm routines here
  //...
  Serial.println("ALARM!");
}
 
void loop(){
  int pirState = digitalRead( PIR_PIN );
  //If PIR state is HIGH
  if( pirState > 0 ){
    alarm();
  }
  else {
    Serial.println(".");
  }
  delay(100); 
}
