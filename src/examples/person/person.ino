#include "dataModel.h"

void setup(){
  Serial.begin(115200);
  
  // Create and populate a person struct
  person p;
  p.id = "magx-01";
  p.name ="Alex Malisa";
  p.weight = 92.31;
  p.age = 29;
  
  // Basic serialization
  Serial.println("1. Basic Serialization:");
  Serial.println(p.serialize());
  Serial.println();
  
}

void loop(){
  // Nothing in loop for this demo
}