Structa – Lightweight Reflective Struct Framework for Embedded C++

Structa is a **lightweight, macro-based data modeling framework** for embedded C++ projects (Arduino, ESP32, etc.) that automates JSON serialization, deserialization, memory tracking, and documentation. It’s designed to make defining and managing structured data effortless, while staying fast and memory efficient.

* * *

🚀 Features

✅ **Simple Macro-Based Definition** – Define structs with minimal code

✅ **Automatic JSON Serialization/Deserialization** – Powered by ArduinoJson

✅ **Comprehensive Error Handling** – Detailed messages with field paths

✅ **Memory Tracking** – Track allocations and peak usage

✅ **Nested Struct Support** – Works with complex hierarchical data

✅ **Self-Documenting** – Auto-generates struct info and usage examples

✅ **Reflection Utilities** – Print fields, definitions, and current values

✅ **IoT-Ready** – Perfect for ESP32/ESP8266 and sensor data models



* * *

⚡ Quick Start

### 1. Include Header

    #include <Structa.h>

### 2. Define Your Struct

    #define PERSON_FIELDS(field) \
        field(String, name) \
        field(int, age) \
        field(float, weight)
    
    DEFINE_STRUCTA(Person, PERSON_FIELDS)

### 3. Use It

    Person p;
    p.name = "Alex";
    p.age = 30;
    p.weight = 72.5;
    
    String json = p.serialize();
    Serial.println(json); // {"name":"Alex","age":30,"weight":72.50}

* * *

🧠 Advanced Example
    #define SENSOR_FIELDS(field) \
        field(String, deviceId) \
        field(float, temperature) \
        field(float, humidity) \
        field(unsigned long, timestamp)
    DEFINE_STRUCTA(Sensor, SENSOR_FIELDS)

    void setup() {
      Serial.begin(115200);

      Sensor s;
      s.deviceId = "TEMP01";
      s.temperature = 26.4;
      s.humidity = 58.3;
      s.timestamp = millis();

      auto json = s.serialize();
      Serial.println(json);

      Sensor copy = Sensor::deserialize(json);
      copy.printCurrentValues();
    }

* * *

🧩 Nested Struct Example
    #define GPS_FIELDS(field) \
        field(float, latitude) \
        field(float, longitude)
    DEFINE_STRUCTA(GPS, GPS_FIELDS)

    #define LOCATION_FIELDS(field) \
        field(String, name) \
        field(GPS, coordinates)

    DEFINE_STRUCTA(Location, LOCATION_FIELDS)

* * *

🧰 Error Handling
    auto result = Person::deserializeWithResult(json);
    if (!result.success) {
      Serial.println(result.error.toString());
    }

Errors include:

* `BUFFER_OVERFLOW`

* `INVALID_JSON`

* `TYPE_MISMATCH`

* `FIELD_MISSING`

* `MEMORY_ALLOCATION_FAILED`

* * *

💾 Memory Tracking
    MemoryTracker::printStats();
    // Output: Memory - Current: 512 bytes, Peak: 1024 bytes

* * *

🧱 Helper Utilities

* `MemoryTracker::showMacroWritingGuide()` – Prints guide for creating structs.

* `MemoryTracker::printExistingStructDefinition()` – Analyzes JSON and suggests struct pattern.

* * *

📦 Folder Structure
    Structa/
    ├── src/
    │   ├── Structa.h
    │   └── examples/
    │       ├── person_example/
    │       │   ├── person.ino
    │       │   └── dataModel.h
    ├── README.md
    ├── LICENSE
    └── library.properties

* * *

⚙️ API Reference

### Core Macro

    DEFINE_STRUCTA(structName, FIELD_LIST)

Automatically generates:

* `serialize()` / `serializeWithResult()`

* `deserialize()` / `deserializeWithResult()`

* `printStructDefinition()` / `printFieldInfo()` / `printCurrentValues()`

### Example

    Person::printStructDefinition();
    Person::printFieldInfo();
    Person p;
    p.printCurrentValues();

* * *

🧩 Supported Data Types

| Category  | Types                          |
| --------- | ------------------------------ |
| Primitive | int, float, double, bool, char |
| String    | String, const char*            |
| Time      | unsigned long                  |
| Nested    | Any Structa-defined struct     |

* * *

💡 Best Practices

✅ Use consistent, clear field names✅ Keep structures small for embedded environments✅ Use nested structs for logical grouping✅ Monitor memory usage during development✅ Handle deserialization errors explicitly

* * *

🧑‍💻 Author

Developed by **Alex Gabriel Malisa**📧 [alexgabrielmalisa@gmail.com](mailto:alexgabrielmalisa@gmail.com)📱 +255 753 007 128

**License:** MIT**Dependencies:** ArduinoJson**Compatibility:** Arduino IDE, PlatformIO
