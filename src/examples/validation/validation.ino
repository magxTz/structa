#include <Arduino.h>
#include "structa.h"

#include "dataModel.h"


User user;

void populate(){
  user.address.city = "Dar es salaam";
  user.address.zip = 12345;
  user.username ="Alex Malisa Gabriel CHARLES";
  user.age = 29;
  user.role = "admin";
  user.note = "welcome Home";
  auto res = user.serializeWithResult();
  if(res.success)
    Serial.println(user.serialize());
  else
    Serial.println(res.error.toString());
}

void setup() {
  Serial.begin(115200);
  populate();
  
}

void loop() {}
