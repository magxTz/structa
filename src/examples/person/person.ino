#include "dataModel.h"
person p;
configs cfg;
settings st;
void setup(){
  Serial.begin(115200);
  // Create and populate a person struct
  
  
  
  // Basic serialization
  Serial.println("1. person Data:");
  Serial.println(populatePerson());
  Serial.println("2. config Data:");
  Serial.println(setConfigs());
  Serial.println("2. settings Data:");
  Serial.println(setting());
  Serial.println();
  
}

String populatePerson(){
  p.id = "magx-01";
  p.name ="Alex Malisa";
  p.weight = 92.31;
  p.age = 29;

  return p.serialize();
}
String setConfigs(){
  cfg.deviceName = "uno";
  cfg.apiKey = "12er43cdx120*";
  cfg.ssid = "Mallisa@12ag";
  cfg.debug = true;
  return cfg.serialize();
}
String setting(){
  st.deviceName = "esp";
  st.apiKey = "12er43caaaax20*";
  st.ssid = "Mallisa@ag12";
  st.debug = false;
  return st.serialize();
}
void loop(){
  // Nothing in loop for this demo
}
