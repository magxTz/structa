const char* roles[] = {"admin", "user", "guest"};
#define USER_ADDRESS(f)          \
  f(String,city,META_NONE()) \
  f(int,zip,META_NONE())    
DEFINE_STRUCTA(Address,USER_ADDRESS)

#define USER_FIELDS(field) \
    field(String, username, META_STRLEN(3, 15)) \
    field(String, role, META_ENUM(roles)) \
    field(int, age, META_RANGE(18, 100)) \
    field(String, note, META_OPTIONAL()) \
    field(Address, address, META_OPTIONAL())

DEFINE_STRUCTA(User, USER_FIELDS)