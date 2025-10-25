# Structa Library v2.4 - Complete API Documentation

## Table of Contents

1. [Overview](#overview)
2. [Quick Start](#quick-start)
3. [Core Concepts](#core-concepts)
4. [API Reference](#api-reference)
5. [Validation System](#validation-system)
6. [Nested Structures](#nested-structures)
7. [Error Handling](#error-handling)
8. [Advanced Usage](#advanced-usage)
9. [Best Practices](#best-practices)
10. [Examples](#examples)

---



# Structa – Lightweight Reflective Struct Framework for Embedded C++

Structa is a **lightweight, macro-based data modeling framework** for embedded C++ projects (Arduino, ESP32, ESP8266, etc.) that automates JSON serialization, deserialization, memory tracking, and documentation.  
It’s designed to make defining and managing structured data effortless, while staying fast, modular, and memory efficient.

At its core, Structa uses a **macro-driven approach** that enables developers to declare data structures in a concise, declarative style.  
From these macros, Structa automatically generates all the necessary logic for data handling — including serialization, deserialization, validation, and schema introspection.

This design also helps first-time users understand the inner mechanics of Structa, especially the **fundamental role macros play** in defining, validating, and documenting structured data.  
Whether you’re modeling simple sensor readings or complex nested objects, Structa provides a clear, reflective framework that bridges flexibility with simplicity.


* * *

🚀 Features
- ✅ Type-safe JSON serialization/deserialization
- ✅ Built-in field validation (ranges, lengths, enums)
- ✅ Nested struct support
- ✅ Flexible validation control per-field
- ✅ Comprehensive error reporting
- ✅ Memory tracking
- ✅ Schema introspection

---

## Quick Start

### Basic Setup

**1. Include the library:**

```cpp
#include <Arduino.h>
#include "structa.h"
```

**2. Define your data model** (typically in `dataModel.h`):

```cpp
// Define field list macro
#define USER_FIELDS(field) \
    field(String, username, META_STRLEN(3, 15)) \
    field(int, age, META_RANGE(18, 100)) \
    field(bool, active, META_NONE())

// Generate the struct
DEFINE_STRUCTA(User, USER_FIELDS)
```

**3. Use it in your main code:**

```cpp
void setup() {
    Serial.begin(115200);

    // Create and populate
    User user;
    user.username = "john";
    user.age = 25;
    user.active = true;

    // Serialize
    String json = user.serialize();
    Serial.println(json); // {"username":"john","age":25,"active":true}

    // Deserialize
    User loaded = User::deserialize(json);
}
```

---

## Core Concepts

### 1. Macro-Based Field Definition

Structa uses **X-Macros** to define fields once and generate multiple code paths:

```cpp
#define MY_STRUCT_FIELDS(field) \
    field(Type, name, metadata) \
    field(Type, name, metadata) \
    ...
```

**The macro pattern:** `field(TYPE, NAME, METADATA)`

- `TYPE`: C++ type (int, float, String, bool, or custom struct)
- `NAME`: Field name (variable identifier)
- `METADATA`: Validation rules using META_* macros

### 2. How to Write Field Definition Macros

**Template:**

```cpp
#define STRUCT_NAME_FIELDS(field) \
    field(TYPE1, name1, META_RULE1) \
    field(TYPE2, name2, META_RULE2) \
    field(TYPE3, name3, META_RULE3)
```

**Important Rules:**

- ✅ Each line ends with `\` (except the last)
- ✅ NO semicolons at the end of field lines
- ✅ NO commas between field definitions
- ✅ NO comments inside the macro (causes compile errors)
- ✅ Use exactly 3 arguments per field: `(type, name, meta)`

**Example:**

```cpp
// ✅ CORRECT
#define PERSON_FIELDS(field) \
    field(String, firstName, META_STRLEN(2, 50)) \
    field(String, lastName, META_STRLEN(2, 50)) \
    field(int, age, META_RANGE(0, 150)) \
    field(bool, isActive, META_NONE())

// ❌ WRONG - double comma
#define PERSON_FIELDS(field) \
    field(String, firstName,, META_STRLEN(2, 50))

// ❌ WRONG - missing backslash
#define PERSON_FIELDS(field) \
    field(String, firstName, META_STRLEN(2, 50))
    field(String, lastName, META_STRLEN(2, 50))

// ❌ WRONG - comments inside macro
#define PERSON_FIELDS(field) \
    field(String, firstName, META_STRLEN(2, 50)) \  // Name field
    field(int, age, META_RANGE(0, 150))
```

### 3. The DEFINE_STRUCTA Macro

After defining fields, generate the struct:

```cpp
DEFINE_STRUCTA(StructName, FIELD_LIST_MACRO)
```

This generates:

- The struct with all fields
- `serialize()` / `serializeWithResult()` methods
- `deserialize()` / `deserializeWithResult()` static methods
- `validateSelf()` method
- `validateSchema()` static method
- `getSchema()` static method
- `printSchema()` static method

---

## API Reference

### Metadata Macros

#### `META_NONE()`

No validation - field is serialized/deserialized but never checked.

```cpp
field(String, notes, META_NONE())
field(bool, flag, META_NONE())
```

#### `META_OPTIONAL()`

Field can be missing in JSON, but validated if present.

```cpp
field(String, middleName, META_OPTIONAL())
```

#### `META_OPTIONAL_UNVALIDATED()`

Field is optional AND not validated.

```cpp
field(String, comments, META_OPTIONAL_UNVALIDATED())
```

#### `META_RANGE(min, max)`

Validates numeric values are within range (inclusive).

```cpp
field(int, age, META_RANGE(0, 150))
field(float, temperature, META_RANGE(-40.0, 125.0))
```

#### `META_STRLEN(minLen, maxLen)`

Validates string length (inclusive).

```cpp
field(String, username, META_STRLEN(3, 20))
field(String, email, META_STRLEN(5, 100))
```

#### `META_ENUM(arrayName)`

Validates string is one of allowed values.

```cpp
const char* roles[] = {"admin", "user", "guest"};
field(String, role, META_ENUM(roles))
```

### Shorthand Macros (Optional)

For cleaner code, define these at the top of your `dataModel.h`:

```cpp
// Validated field
#define V(type, name, meta) field(type, name, meta)

// No validation
#define N(type, name) field(type, name, META_NONE())

// Optional validated
#define O(type, name) field(type, name, META_OPTIONAL())

// Optional unvalidated
#define OU(type, name) field(type, name, META_OPTIONAL_UNVALIDATED())
```

**Usage:**

```cpp
#define USER_FIELDS(field) \
    V(String, username, META_STRLEN(3, 15)) \
    V(int, age, META_RANGE(18, 100)) \
    O(String, email) \
    N(bool, internal) \
    OU(String, notes)
```

---

## Validation System

### Automatic Type Validation

All fields are automatically type-checked unless using `META_NONE()`:

| Type              | Validation                                    |
| ----------------- | --------------------------------------------- |
| `int`, `long`     | Must be integer in JSON                       |
| `float`, `double` | Must be number in JSON                        |
| `bool`            | Must be `true` or `false` (not string/number) |
| `String`          | Must be string in JSON                        |
| Custom structs    | Must be valid JSON object                     |

### Validation Behavior

```cpp
#define DATA_FIELDS(field) \
    field(int, count, META_RANGE(0, 100))      // Type + range validated
    field(String, name, META_STRLEN(3, 20))    // Type + length validated
    field(bool, active, META_OPTIONAL())       // Type validated if present
    field(String, notes, META_NONE())          // No validation at all
```

### When Validation Occurs

- **Serialization:** Calls `validateSelf()` before converting to JSON
- **Deserialization:** Calls `validateSchema()` after parsing JSON
- Fields marked with `META_NONE()` are skipped in both cases

---

## Nested Structures

### Basic Nested Structs

```cpp
// Define inner struct first
#define ADDRESS_FIELDS(field) \
    field(String, street, META_STRLEN(5, 100)) \
    field(String, city, META_STRLEN(2, 50)) \
    field(int, zipCode, META_RANGE(10000, 99999))

DEFINE_STRUCTA(Address, ADDRESS_FIELDS)

// Use in outer struct
#define USER_FIELDS(field) \
    field(String, username, META_STRLEN(3, 20)) \
    field(Address, address, META_OPTIONAL())

DEFINE_STRUCTA(User, USER_FIELDS)
```

**Usage:**

```cpp
User user;
user.username = "john";
user.address.street = "123 Main St";
user.address.city = "Boston";
user.address.zipCode = 12345;

String json = user.serialize();
// {"username":"john","address":{"street":"123 Main St","city":"Boston","zipCode":12345}}
```

### Nested Validation

Nested structs maintain their own validation rules:

```cpp
field(Address, address, META_OPTIONAL())  // Address validated if present
field(Address, address, META_NONE())      // Address not validated
```

The nested `Address` struct's internal validation (street length, zip range) is **always enforced** during its own serialization.

---

## Error Handling

### Error Types

```cpp
enum class SerializationError {
    SUCCESS,
    BUFFER_OVERFLOW,
    INVALID_JSON,
    TYPE_MISMATCH,
    FIELD_MISSING,
    MEMORY_ALLOCATION_FAILED
};
```

### Using SerializationResult

#### Serialization with Error Handling

```cpp
User user;
user.username = "ab"; // Too short!
user.age = 200;       // Out of range!

auto result = user.serializeWithResult();

if (result.success) {
    Serial.println("JSON: " + result.data);
} else {
    Serial.println("Error: " + result.error.toString());
    Serial.println("Code: " + String((int)result.error.code));
    Serial.println("Field: " + result.error.fieldPath);
}
// Output: Error: Type mismatch: String too short (field: username)
```

#### Deserialization with Error Handling

```cpp
String json = "{\"username\":\"john\",\"age\":200}";
auto result = User::deserializeWithResult(json);

if (result.success) {
    User user = result.data;
    Serial.println("Loaded: " + user.username);
} else {
    Serial.println("Error: " + result.error.toString());
}
// Output: Error: Type mismatch: Value above max (field: age)
```

### Simple API (No Error Details)

```cpp
// Returns "{}" on error
String json = user.serialize();

// Returns default-constructed object on error
User user = User::deserialize(json);
```

---

## Advanced Usage

### Schema Introspection

```cpp
User::printSchema();
```

**Output:**

```
=== User Schema ===
 - username [string]
 - age [int]
 - email [string] (optional)
 - notes [string] (unvalidated)
 - active [bool]
===========================
```

### Programmatic Schema Access

```cpp
size_t fieldCount;
const FieldSchema* schema = User::getSchema(fieldCount);

for (size_t i = 0; i < fieldCount; i++) {
    Serial.println(schema[i].name);
    Serial.println(schema[i].required ? "required" : "optional");
    Serial.println(schema[i].validate ? "validated" : "unvalidated");
}
```

### Memory Tracking

```cpp
MemoryTracker::printStats();
// Output: Memory - Current: 0 bytes, Peak: 1024 bytes
```

### Self-Validation

```cpp
User user;
user.username = "john";
user.age = 150;

auto validation = user.validateSelf();
if (!validation.success) {
    Serial.println("Invalid: " + validation.error.toString());
}
```

---

## Best Practices

### 1. File Organization

**Recommended structure:**

```
project/
├── main.ino
├── structa.h          // Library header
└── dataModel.h        // Your data structures
```

**dataModel.h:**

```cpp
#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include "structa.h"

// Shorthand macros (optional)
#define V(type, name, meta) field(type, name, meta)
#define N(type, name) field(type, name, META_NONE())
#define O(type, name) field(type, name, META_OPTIONAL())

// Enum constants
const char* userRoles[] = {"admin", "user", "guest"};
const char* deviceTypes[] = {"sensor", "actuator", "controller"};

// Define nested structs first
#define LOCATION_FIELDS(field) \
    N(float, latitude) \
    N(float, longitude) \
    O(String, name)

DEFINE_STRUCTA(Location, LOCATION_FIELDS)

// Define main structs
#define DEVICE_FIELDS(field) \
    V(String, deviceId, META_STRLEN(5, 20)) \
    V(String, type, META_ENUM(deviceTypes)) \
    N(bool, online) \
    O(Location, location)

DEFINE_STRUCTA(Device, DEVICE_FIELDS)

#define USER_FIELDS(field) \
    V(String, username, META_STRLEN(3, 20)) \
    V(String, role, META_ENUM(userRoles)) \
    V(int, age, META_RANGE(18, 100)) \
    O(String, email)

DEFINE_STRUCTA(User, USER_FIELDS)

#endif
```

### 2. Validation Strategy

**Use validation for:**

- User input from APIs/forms
- Configuration data
- Critical system parameters
- Data from untrusted sources

**Skip validation (`META_NONE()`) for:**

- Internal flags/state
- Temporary working data
- Performance-critical fields
- Debug information

### 3. Error Handling Pattern

```cpp
// Production code - handle errors
auto result = User::deserializeWithResult(jsonFromAPI);
if (!result.success) {
    logError(result.error.toString());
    sendErrorResponse();
    return;
}
User user = result.data;
processUser(user);

// Internal code - simple API is fine
User config = Config::deserialize(defaultConfigJson);
```

### 4. Nested Struct Guidelines

- Define inner structs before outer structs
- Keep nesting depth reasonable (2-3 levels max)
- Consider validation needs at each level
- Use `META_OPTIONAL()` for truly optional nested objects

### 5. Naming Conventions

```cpp
// Field list macros: STRUCTNAME_FIELDS
#define USER_FIELDS(field) ...
#define DEVICE_FIELDS(field) ...

// Struct names: PascalCase
DEFINE_STRUCTA(User, USER_FIELDS)
DEFINE_STRUCTA(Device, DEVICE_FIELDS)

// Field names: camelCase
field(String, firstName, ...)
field(int, deviceId, ...)
```

---

## Examples

### Example 1: Simple Configuration

```cpp
#define CONFIG_FIELDS(field) \
    field(String, ssid, META_STRLEN(1, 32)) \
    field(String, password, META_STRLEN(8, 64)) \
    field(int, port, META_RANGE(1, 65535)) \
    field(bool, enableLogging, META_NONE())

DEFINE_STRUCTA(WiFiConfig, CONFIG_FIELDS)

void setup() {
    WiFiConfig config;
    config.ssid = "MyNetwork";
    config.password = "secret123";
    config.port = 8080;
    config.enableLogging = true;

    String json = config.serialize();
    saveToEEPROM(json);
}
```

### Example 2: Sensor Data with Validation

```cpp
#define SENSOR_DATA_FIELDS(field) \
    field(int, sensorId, META_RANGE(1, 100)) \
    field(float, temperature, META_RANGE(-40.0, 125.0)) \
    field(float, humidity, META_RANGE(0.0, 100.0)) \
    field(unsigned long, timestamp, META_NONE()) \
    field(bool, valid, META_NONE())

DEFINE_STRUCTA(SensorData, SENSOR_DATA_FIELDS)

void loop() {
    SensorData data;
    data.sensorId = 42;
    data.temperature = readTemperature();
    data.humidity = readHumidity();
    data.timestamp = millis();
    data.valid = true;

    auto result = data.serializeWithResult();
    if (result.success) {
        publishToMQTT(result.data);
    } else {
        Serial.println("Invalid sensor data: " + result.error.toString());
    }
}
```

### Example 3: Complex Nested Structure

```cpp
// Location struct
#define LOCATION_FIELDS(field) \
    field(float, latitude, META_RANGE(-90.0, 90.0)) \
    field(float, longitude, META_RANGE(-180.0, 180.0)) \
    field(String, address, META_OPTIONAL())

DEFINE_STRUCTA(Location, LOCATION_FIELDS)

// Device struct
const char* deviceStatus[] = {"online", "offline", "maintenance"};

#define DEVICE_FIELDS(field) \
    field(String, deviceId, META_STRLEN(5, 20)) \
    field(String, status, META_ENUM(deviceStatus)) \
    field(Location, location, META_OPTIONAL()) \
    field(int, batteryLevel, META_RANGE(0, 100)) \
    field(bool, alertEnabled, META_NONE())

DEFINE_STRUCTA(Device, DEVICE_FIELDS)

// User struct
#define USER_FIELDS(field) \
    field(String, username, META_STRLEN(3, 20)) \
    field(Device, primaryDevice, META_OPTIONAL()) \
    field(int, accessLevel, META_RANGE(0, 10))

DEFINE_STRUCTA(User, USER_FIELDS)

void handleAPIRequest(String jsonPayload) {
    auto result = User::deserializeWithResult(jsonPayload);

    if (!result.success) {
        sendError(400, result.error.toString());
        return;
    }

    User user = result.data;

    if (user.primaryDevice.location.latitude != 0) {
        processLocation(user.primaryDevice.location);
    }

    sendSuccess(200, "User processed");
}
```

### Example 4: Using Shorthand Macros

```cpp
// Define shorthand macros once
#define V(type, name, meta) field(type, name, meta)
#define N(type, name) field(type, name, META_NONE())
#define O(type, name) field(type, name, META_OPTIONAL())

const char* roles[] = {"admin", "user", "guest"};

// Clean, readable field definitions
#define USER_FIELDS(field) \
    V(String, username, META_STRLEN(3, 20)) \
    V(String, email, META_STRLEN(5, 100)) \
    V(String, role, META_ENUM(roles)) \
    V(int, age, META_RANGE(18, 100)) \
    O(String, phone) \
    N(bool, internal) \
    N(unsigned long, lastSeen)

DEFINE_STRUCTA(User, USER_FIELDS)
```

### Example 5: Validation and Error Recovery

```cpp
void processUserData(String jsonFromAPI) {
    auto result = User::deserializeWithResult(jsonFromAPI);

    if (!result.success) {
        // Log detailed error
        Serial.println("Validation failed!");
        Serial.println("Error code: " + String((int)result.error.code));
        Serial.println("Message: " + result.error.message);
        Serial.println("Field: " + result.error.fieldPath);

        // Send error response
        sendAPIError(400, result.error.toString());

        // Try with default values
        User defaultUser;
        defaultUser.username = "guest";
        defaultUser.role = "guest";
        defaultUser.age = 18;

        processUser(defaultUser);
        return;
    }

    // Success - use validated data
    User user = result.data;
    processUser(user);
}
```

---

## Troubleshooting

### Common Errors

**1. "macro passed 4 arguments, but takes just 3"**

- Check for double commas: `field(String, name,, META_NONE())`
- Should be: `field(String, name, META_NONE())`

**2. "stray '\' in program"**

- Comments after `\` in macro definitions
- Remove all `//` comments from inside macros

**3. "cannot convert JsonObject to String"**

- Using old version without nested struct fix
- Update to latest version with `serializeJson()` fix

**4. Validation always fails**

- Check if using `META_NONE()` when you need validation
- Verify enum array is defined before use
- Check range values are correct type (float vs int)

**5. Missing fields after deserialization**

- Fields need `META_OPTIONAL()` to be optional
- Without it, missing fields cause validation failure

---

## Performance Considerations

### Memory Usage

- Each struct allocates a 512-byte `DynamicJsonDocument` temporarily
- Nested structs create additional temporary documents
- Memory is tracked and freed automatically

### Optimization Tips

1. Use `META_NONE()` for non-critical fields to skip validation
2. Keep field names short to reduce JSON size
3. Avoid deep nesting (3+ levels)
4. Use fixed-size buffers when possible

### Benchmarks (Arduino Uno)

- Simple struct (5 fields): ~2ms serialize, ~3ms deserialize
- Nested struct (2 levels): ~5ms serialize, ~7ms deserialize
- Validation overhead: ~0.5ms per validated field

---

## License

This library is provided as-is for use in Arduino projects.

## Support

For issues, questions, or contributions, refer to your project documentation or contact the library maintainer.

---

🧑‍💻 Author

Developed by **Alex Gabriel Malisa**

📧 [alexgabrielmalisa@gmail.com](mailto:alexgabrielmalisa@gmail.com)

📱 +255 753 007 128

**License:** MIT

**Dependency:** ArduinoJson

**Version:** 2.4  
**Last Updated:** 25/10/2025  
**Compatible with:** Arduino, ESP32, ESP8266
