// ============================================
// Structa V2 - Enhanced Version
// ============================================
#ifndef STRUCTA_H
#define STRUCTA_H

#include <ArduinoJson.h>

// ======================================================
// Error Handling
// ======================================================
enum class SerializationError {
    SUCCESS = 0,
    BUFFER_OVERFLOW,
    INVALID_JSON,
    TYPE_MISMATCH,
    FIELD_MISSING,
    MEMORY_ALLOCATION_FAILED
};

struct ErrorInfo {
    SerializationError code;
    String message;
    String fieldPath;
    
    ErrorInfo() : code(SerializationError::SUCCESS) {}
    ErrorInfo(SerializationError c, const String& msg, const String& path = "") 
        : code(c), message(msg), fieldPath(path) {}
    
    String toString() const {
        String result = "Error: ";
        switch(code) {
            case SerializationError::SUCCESS: return "Success";
            case SerializationError::BUFFER_OVERFLOW: result += "Buffer overflow"; break;
            case SerializationError::INVALID_JSON: result += "Invalid JSON"; break;
            case SerializationError::TYPE_MISMATCH: result += "Type mismatch"; break;
            case SerializationError::FIELD_MISSING: result += "Field missing"; break;
            case SerializationError::MEMORY_ALLOCATION_FAILED: result += "Memory allocation failed"; break;
        }
        if(message.length() > 0) result += ": " + message;
        if(fieldPath.length() > 0) result += " (field: " + fieldPath + ")";
        return result;
    }
};

template<typename T>
struct SerializationResult {
    bool success;
    T data;
    ErrorInfo error;
    
    SerializationResult() : success(false) {}
    
    static SerializationResult<T> Success(const T& value) {
        SerializationResult<T> result;
        result.success = true;
        result.data = value;
        return result;
    }
    
    static SerializationResult<T> Failure(SerializationError code, const String& msg, const String& path = "") {
        SerializationResult<T> result;
        result.success = false;
        result.error = ErrorInfo(code, msg, path);
        return result;
    }
    
    operator bool() const { return success; }
};

// ======================================================
// Memory Tracking
// ======================================================
class MemoryTracker {
private:
    static size_t totalAllocated;
    static size_t peakUsage;
    
public:
    static void recordAllocation(size_t size) {
        totalAllocated += size;
        if(totalAllocated > peakUsage) peakUsage = totalAllocated;
    }
    
    static void recordDeallocation(size_t size) {
        if(totalAllocated >= size) totalAllocated -= size;
    }
    
    static size_t getCurrentUsage() { return totalAllocated; }
    static size_t getPeakUsage() { return peakUsage; }
    
    static void printStats() {
        Serial.println("Memory - Current: " + String(totalAllocated) + " bytes, Peak: " + String(peakUsage) + " bytes");
    }
    static void printExistingStructDefinition(const String& structName, const String& fieldsJson) {
        Serial.println("=== Existing Struct Definition ===");
        Serial.println("Struct Name: " + structName);
        Serial.println("Current JSON Structure:");
        Serial.println(fieldsJson);
        Serial.println();
        
        // Parse and display field information
        DynamicJsonDocument doc(512);
        DeserializationError err = deserializeJson(doc, fieldsJson);
        if (!err) {
            Serial.println("Detected Fields:");
            JsonObject obj = doc.as<JsonObject>();
            for (JsonPair kv : obj) {
                String fieldName = kv.key().c_str();
                String fieldType = "Unknown";
                
                // Determine field type from JSON value
                if (kv.value().is<int>()) {
                    fieldType = "int";
                } else if (kv.value().is<float>()) {
                    fieldType = "float";
                } else if (kv.value().is<bool>()) {
                    fieldType = "bool";
                } else if (kv.value().is<const char*>()) {
                    fieldType = "String";
                } else if (kv.value().is<JsonObject>()) {
                    fieldType = "NestedStruct";
                }
                
                Serial.println("  - " + fieldName + " (" + fieldType + ")");
            }
        }
        Serial.println("===================================");
    }
    
    static void showMacroWritingGuide() {
        Serial.println("=== How to Write Struct Macros ===");
        Serial.println();
        Serial.println("Step 1: Define your fields macro");
        Serial.println("Pattern: #define STRUCT_NAME_FIELDS(field) \\");
        Serial.println("    field(Type, fieldName) \\");
        Serial.println("    field(Type, fieldName) \\");
        Serial.println("    // ... more fields");
        Serial.println();
        Serial.println("Step 2: Create the struct");
        Serial.println("DEFINE_STRUCTA(StructName, STRUCT_NAME_FIELDS)");
        Serial.println();
        Serial.println("=== Example 1: Simple Person Struct ===");
        Serial.println("#define PERSON_FIELDS(field) \\");
        Serial.println("    field(String, name) \\");
        Serial.println("    field(int, age) \\");
        Serial.println("    field(float, height)");
        Serial.println();
        Serial.println("DEFINE_STRUCTA(Person, PERSON_FIELDS)");
        Serial.println();
        Serial.println("=== Example 2: IoT Sensor Data ===");
        Serial.println("#define SENSOR_FIELDS(field) \\");
        Serial.println("    field(String, deviceId) \\");
        Serial.println("    field(float, temperature) \\");
        Serial.println("    field(float, humidity) \\");
        Serial.println("    field(int, batteryLevel) \\");
        Serial.println("    field(unsigned long, timestamp)");
        Serial.println();
        Serial.println("DEFINE_STRUCTA(SensorReading, SENSOR_FIELDS)");
        Serial.println();
        Serial.println("=== Example 3: Nested Structures ===");
        Serial.println("// First define the nested struct");
        Serial.println("#define GPS_FIELDS(field) \\");
        Serial.println("    field(float, latitude) \\");
        Serial.println("    field(float, longitude) \\");
        Serial.println("    field(float, altitude)");
        Serial.println();
        Serial.println("DEFINE_STRUCTA(GPSCoordinate, GPS_FIELDS)");
        Serial.println();
        Serial.println("// Then use it in parent struct");
        Serial.println("#define LOCATION_FIELDS(field) \\");
        Serial.println("    field(String, locationName) \\");
        Serial.println("    field(GPSCoordinate, coordinates) \\");
        Serial.println("    field(String, description)");
        Serial.println();
        Serial.println("DEFINE_STRUCTA(Location, LOCATION_FIELDS)");
        Serial.println();
        Serial.println("=== Supported Types ===");
        Serial.println("Primitives: int, float, double, bool, char");
        Serial.println("Strings: String, const char*");
        Serial.println("Time: unsigned long (for timestamps)");
        Serial.println("Nested: Any struct created with DEFINE_STRUCTA");
        Serial.println();
        Serial.println("=== Important Notes ===");
        Serial.println("1. Always end field lines with backslash (\\) except the last");
        Serial.println("2. Use consistent naming conventions");
        Serial.println("3. Define nested structs before parent structs");
        Serial.println("4. Field names become JSON keys automatically");
        Serial.println("5. Use descriptive struct and field names");
        Serial.println("=====================================");
    }
};

size_t MemoryTracker::totalAllocated = 0;
size_t MemoryTracker::peakUsage = 0;

// ======================================================
// Type Detection (renamed to avoid conflicts)
// ======================================================
template<typename T>
class HasSerialize {
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().serialize(), std::true_type{});
    template<typename>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

// ======================================================
// Base Class
// ======================================================
class JsonStruct {
protected:
    // Serialize primitives
    template<typename T>
    static typename std::enable_if<!HasSerialize<T>::value, void>::type
    serializeField(JsonObject& obj, const char* key, const T& value) {
        obj[key] = value;
    }
    
    // Serialize nested structs
    template<typename T>
    static typename std::enable_if<HasSerialize<T>::value, void>::type
    serializeField(JsonObject& obj, const char* key, const T& value) {
        DynamicJsonDocument sub(256);
        deserializeJson(sub, value.serialize());
        obj[key] = sub.as<JsonObject>();
    }
    
    // Deserialize primitives
    template<typename T>
    static typename std::enable_if<!HasSerialize<T>::value, void>::type
    deserializeField(const JsonObject& obj, const char* key, T& value) {
        if (obj.containsKey(key)) {
            value = obj[key].as<T>();
        }
    }
    
    // Deserialize nested structs
    template<typename T>
    static typename std::enable_if<HasSerialize<T>::value, void>::type
    deserializeField(const JsonObject& obj, const char* key, T& value) {
        if (obj.containsKey(key) && obj[key].is<JsonObject>()) {
            JsonObject sub = obj[key].as<JsonObject>();
            value = T::deserialize(sub);
        }
    }
};

// ======================================================
// Macros
// ======================================================
#define DECLARE(type, name) type name;
#define SERIALIZE_FIELD(type, name) serializeField(obj, #name, name);
#define DESERIALIZE_FIELD(type, name) deserializeField(o, #name, data.name);

// ======================================================
// Main Struct Definition Macro
// ======================================================
#define DEFINE_STRUCTA(structName, FIELD_LIST)                             \
struct structName : public JsonStruct {                                   \
    FIELD_LIST(DECLARE)                                                   \
                                                                              \
    SerializationResult<String> serializeWithResult() const {                \
        DynamicJsonDocument doc(512);                                        \
        MemoryTracker::recordAllocation(512);                                \
        JsonObject obj = doc.to<JsonObject>();                               \
        FIELD_LIST(SERIALIZE_FIELD)                                       \
        String result;                                                        \
        if(serializeJson(doc, result) == 0) {                                \
            MemoryTracker::recordDeallocation(512);                          \
            return SerializationResult<String>::Failure(                     \
                SerializationError::INVALID_JSON, "Failed to serialize");    \
        }                                                                     \
        MemoryTracker::recordDeallocation(512);                              \
        return SerializationResult<String>::Success(result);                 \
    }                                                                         \
                                                                              \
    String serialize() const {                                                \
        auto result = serializeWithResult();                                 \
        return result.success ? result.data : "{}";                          \
    }                                                                         \
                                                                              \
    static SerializationResult<structName> deserializeWithResult(const String& jsonStr) { \
        DynamicJsonDocument doc(512);                                        \
        MemoryTracker::recordAllocation(512);                                \
        DeserializationError err = deserializeJson(doc, jsonStr);            \
        if(err) {                                                             \
            MemoryTracker::recordDeallocation(512);                          \
            return SerializationResult<structName>::Failure(                 \
                SerializationError::INVALID_JSON, String("Parse error: ") + err.c_str()); \
        }                                                                     \
        JsonObject o = doc.as<JsonObject>();                                 \
        structName data;                                                      \
        FIELD_LIST(DESERIALIZE_FIELD)                                     \
        MemoryTracker::recordDeallocation(512);                              \
        return SerializationResult<structName>::Success(data);               \
    }                                                                         \
                                                                              \
    static SerializationResult<structName> deserializeWithResult(const JsonObject& o) { \
        structName data;                                                      \
        FIELD_LIST(DESERIALIZE_FIELD)                                     \
        return SerializationResult<structName>::Success(data);               \
    }                                                                         \
                                                                              \
    static structName deserialize(const String& jsonStr) {                   \
        auto result = deserializeWithResult(jsonStr);                        \
        return result.success ? result.data : structName();                  \
    }                                                                         \
                                                                              \
    static structName deserialize(const JsonObject& o) {                     \
        auto result = deserializeWithResult(o);                              \
        return result.success ? result.data : structName();                  \
    }                                                                         \
                                                                              \
    static void printStructDefinition() {                                    \
        Serial.println("=== " #structName " Struct Definition ===");        \
        Serial.println("Struct Name: " #structName);                         \
        Serial.println("Generated Methods:");                                 \
        Serial.println("  - serialize() -> String");                         \
        Serial.println("  - serializeWithResult() -> SerializationResult<String>"); \
        Serial.println("  - deserialize(String) -> " #structName);           \
        Serial.println("  - deserialize(JsonObject) -> " #structName);       \
        Serial.println("  - deserializeWithResult(String) -> SerializationResult<" #structName ">"); \
        Serial.println("  - deserializeWithResult(JsonObject) -> SerializationResult<" #structName ">"); \
        Serial.println("  - printStructDefinition() -> void");               \
        Serial.println("  - printFieldInfo() -> void");                      \
        Serial.println("  - printCurrentValues() -> void");                  \
        Serial.println();                                                     \
        Serial.println("Usage Example:");                                     \
        Serial.println("  " #structName " obj;");                            \
        Serial.println("  String json = obj.serialize();");                  \
        Serial.println("  " #structName " copy = " #structName "::deserialize(json);"); \
        Serial.println("=======================================");             \
    }                                                                         \
                                                                              \
    static void printFieldInfo() {                                           \
        Serial.println("=== " #structName " Field Information ===");        \
        Serial.println("To see actual field values, create an instance and call printCurrentValues()"); \
        Serial.println();                                                     \
        Serial.println("Macro Definition Pattern:");                         \
        Serial.println("#define " #structName "_FIELDS(field) \\");          \
        Serial.println("    field(Type, fieldName) \\");                     \
        Serial.println("    // ... more fields");                            \
        Serial.println();                                                     \
        Serial.println("Then use: DEFINE_STRUCTA(" #structName ", " #structName "_FIELDS)"); \
        Serial.println();                                                     \
        Serial.println("For detailed macro writing guide, call:");           \
        Serial.println("MemoryTracker::showMacroWritingGuide();");           \
        Serial.println("=========================================");          \
    }                                                                         \
                                                                              \
    void printCurrentValues() const {                                        \
        Serial.println("=== " #structName " Current Values ===");           \
        String json = serialize();                                            \
        Serial.println("JSON Representation:");                              \
        Serial.println(json);                                                 \
        Serial.println();                                                     \
        Serial.println("Formatted Output:");                                 \
        DynamicJsonDocument doc(512);                                        \
        deserializeJson(doc, json);                                          \
        JsonObject obj = doc.as<JsonObject>();                               \
        for (JsonPair kv : obj) {                                            \
            String fieldName = kv.key().c_str();                             \
            String fieldValue;                                                \
            if (kv.value().is<int>()) {                                      \
                fieldValue = String(kv.value().as<int>());                   \
            } else if (kv.value().is<float>()) {                             \
                fieldValue = String(kv.value().as<float>(), 2);              \
            } else if (kv.value().is<bool>()) {                              \
                fieldValue = kv.value().as<bool>() ? "true" : "false";      \
            } else if (kv.value().is<const char*>()) {                       \
                fieldValue = "\"" + String(kv.value().as<const char*>()) + "\""; \
            } else if (kv.value().is<JsonObject>()) {                        \
                fieldValue = "[Nested Object]";                              \
            } else {                                                          \
                fieldValue = "[Unknown Type]";                               \
            }                                                                 \
            Serial.println("  " + fieldName + ": " + fieldValue);           \
        }                                                                     \
        Serial.println("=====================================");             \
    }                                                                         \
};

#endif // STRUCTA_H