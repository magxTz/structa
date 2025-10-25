#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include "structa.h"

#define fields(field)   \
  field(String,id)      \
  field(String,name)    \
  field(int,age)        \
  field(float,weight)

DEFINE_STRUCTA(person,fields)

#define configFields(f) \
  f(String, deviceName) \
  f(String, apiKey)     \
  f(String, ssid)       \
  f(bool, debug)

DEFINE_STRUCTA(configs,configFields)
#define Fields(f) \
  f(String, deviceName) \
  f(String, apiKey)     \
  f(String, ssid)       \
  f(bool, debug)

DEFINE_STRUCTA(settings,Fields)

#endif // DATA_MODEL_H
