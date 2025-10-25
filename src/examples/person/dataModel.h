
#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include "structa.h"

# define fields(field)\
field(String,id)\
field(String,name)\
field(int,age)\ 
field(float,weight)\

DEFINE_STRUCTA(person,fields)

#endif // DATA_MODEL_H
