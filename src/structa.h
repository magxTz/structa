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
    MEMORY_ALLOCATION_FAILED,
    VALIDATION_FAILED  // NEW: Validation error
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
            case SerializationError::VALIDATION_FAILED: result += "Validation failed"; break;  // NEW
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
// Validation Support (NEW)
// ======================================================

// Validator interface - all validators inherit from this
struct ValidatorBase {
    virtual bool validate(const String& fieldName, const String& value, String& errorMsg) const { return true; }
    virtual bool validate(const String& fieldName, int value, String& errorMsg) const { return true; }
    virtual bool validate(const String& fieldName, float value, String& errorMsg) const { return true; }
    virtual bool validate(const String& fieldName, double value, String& errorMsg) const { return true; }
    virtual bool validate(const String& fieldName, bool value, String& errorMsg) const { return true; }
    virtual ~ValidatorBase() {}
};

// Range validator for numeric types
template<typename T>
struct RangeValidator : public ValidatorBase {
    T minVal;
    T maxVal;
    bool hasMin;
    bool hasMax;
    
    RangeValidator() : hasMin(false), hasMax(false) {}
    RangeValidator(T min, T max) : minVal(min), maxVal(max), hasMin(true), hasMax(true) {}
    
    bool validate(const String& fieldName, T value, String& errorMsg) const {
        if (hasMin && value < minVal) {
            errorMsg = "Value " + String(value) + " is below minimum " + String(minVal);
            return false;
        }
        if (hasMax && value > maxVal) {
            errorMsg = "Value " + String(value) + " exceeds maximum " + String(maxVal);
            return false;
        }
        return true;
    }
};

// String length validator
struct StringLengthValidator : public ValidatorBase {
    size_t minLen;
    size_t maxLen;
    bool hasMin;
    bool hasMax;
    
    StringLengthValidator() : hasMin(false), hasMax(false) {}
    StringLengthValidator(size_t min, size_t max) : minLen(min), maxLen(max), hasMin(true), hasMax(true) {}
    StringLengthValidator(size_t exactLen) : minLen(exactLen), maxLen(exactLen), hasMin(true), hasMax(true) {}
    
    static StringLengthValidator minLength(size_t min) {
        StringLengthValidator v;
        v.minLen = min;
        v.hasMin = true;
        v.hasMax = false;
        return v;
    }
    
    static StringLengthValidator maxLength(size_t max) {
        StringLengthValidator v;
        v.maxLen = max;
        v.hasMin = false;
        v.hasMax = true;
        return v;
    }
    
    bool validate(const String& fieldName, const String& value, String& errorMsg) const {
        if (hasMin && value.length() < minLen) {
            errorMsg = "String length " + String(value.length()) + " is below minimum " + String(minLen);
            return false;
        }
        if (hasMax && value.length() > maxLen) {
            errorMsg = "String length " + String(value.length()) + " exceeds maximum " + String(maxLen);
            return false;
        }
        return true;
    }
};

// Required field validator
struct RequiredValidator : public ValidatorBase {
    bool validate(const String& fieldName, const String& value, String& errorMsg) const {
        if (value.length() == 0) {
            errorMsg = "Field is required but empty";
            return false;
        }
        return true;
    }
    
    bool validate(const String& fieldName, int value, String& errorMsg) const {
        // For numeric types, we could consider 0 as empty or not - this is configurable
        return true; // By default, any numeric value is considered valid for required
    }
    
    bool validate(const String& fieldName, float value, String& errorMsg) const {
        return true; // By default, any numeric value is considered valid for required
    }
    
    bool validate(const String& fieldName, double value, String& errorMsg) const {
        return true; // By default, any numeric value is considered valid for required
    }
    
    bool validate(const String& fieldName, bool value, String& errorMsg) const {
        return true; // Boolean values are always considered valid for required
    }
};

// Custom function validator
template<typename T>
struct CustomValidator : public ValidatorBase {
    bool (*validatorFunc)(T);
    String customErrorMsg;
    
    CustomValidator(bool (*func)(T), const String& errorMessage = "Custom validation failed")
        : validatorFunc(func), customErrorMsg(errorMessage) {}
    
    bool validate(const String& fieldName, T value, String& errorMsg) const {
        if (!validatorFunc(value)) {
            errorMsg = customErrorMsg;
            return false;
        }
        return true;
    }
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
        Serial.println("=== Example 3: WITH VALIDATION (NEW!) ===");
        Serial.println("#define SENSOR_FIELDS_V(field) \\");
        Serial.println("    field(String, deviceId) \\");
        Serial.println("    field(float, temperature) \\");
        Serial.println("    field(float, humidity) \\");
        Serial.println("    field(int, batteryLevel)");
        Serial.println();
        Serial.println("#define SENSOR_VALIDATORS(v) \\");
        Serial.println("    v(temperature, makeRangeValidatorFloat(-40, 85)) \\");
        Serial.println("    v(humidity, makeRangeValidatorFloat(0, 100)) \\");
        Serial.println("    v(batteryLevel, makeRangeValidatorInt(0, 100)) \\");
        Serial.println("    v(deviceId, makeRequiredValidator())");
        Serial.println();
        Serial.println("DEFINE_STRUCTA_WITH_VALIDATION(Sensor, SENSOR_FIELDS_V, SENSOR_VALIDATORS)");
        Serial.println();
        Serial.println("=== Example 4: Nested Structures ===");
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
        Serial.println("=== Validation Types (NEW!) ===");
        Serial.println("RangeValidator<T>(min, max) - For numeric types");
        Serial.println("StringLengthValidator(min, max) - For strings");
        Serial.println("StringLengthValidator::minLength(min) - Minimum length only");
        Serial.println("StringLengthValidator::maxLength(max) - Maximum length only");
        Serial.println("RequiredValidator() - Field cannot be empty");
        Serial.println("CustomValidator<T>(func, errorMsg) - Custom validation function");
        Serial.println();
        Serial.println("=== Important Notes ===");
        Serial.println("1. Always end field lines with backslash (\\) except the last");
        Serial.println("2. Use consistent naming conventions");
        Serial.println("3. Define nested structs before parent structs");
        Serial.println("4. Field names become JSON keys automatically");
        Serial.println("5. Validation is optional - use DEFINE_STRUCTA or DEFINE_STRUCTA_WITH_VALIDATION");
        Serial.println("6. Validation occurs automatically during deserializeWithResult()");
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

// NEW: Simple validation macros that avoid comma issues
#define DECLARE_VALIDATOR(fieldName, validatorInstance) auto validator_##fieldName = validatorInstance;
#define VALIDATE_FIELD(fieldName, validatorInstance) \
    { \
        String errMsg; \
        if (!validator_##fieldName.validate(#fieldName, fieldName, errMsg)) { \
            return SerializationResult<bool>::Failure( \
                SerializationError::VALIDATION_FAILED, errMsg, #fieldName); \
        } \
    }

// Helper functions to create validators without comma issues
inline RangeValidator<int> makeRangeValidatorInt(int min, int max) {
    return RangeValidator<int>(min, max);
}

inline RangeValidator<float> makeRangeValidatorFloat(float min, float max) {
    return RangeValidator<float>(min, max);
}

inline StringLengthValidator makeStringLengthValidator(size_t min, size_t max) {
    return StringLengthValidator(min, max);
}

inline StringLengthValidator makeStringMinLengthValidator(size_t min) {
    return StringLengthValidator::minLength(min);
}

inline StringLengthValidator makeStringMaxLengthValidator(size_t max) {
    return StringLengthValidator::maxLength(max);
}

inline RequiredValidator makeRequiredValidator() {
    return RequiredValidator();
}

template<typename T>
inline CustomValidator<T> makeCustomValidator(bool (*func)(T), const String& errorMsg = "Custom validation failed") {
    return CustomValidator<T>(func, errorMsg);
}

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

// ======================================================
// NEW: Struct Definition WITH Validation Support
// ======================================================
#define DEFINE_STRUCTA_WITH_VALIDATION(structName, FIELD_LIST, VALIDATOR_LIST) \
struct structName : public JsonStruct {                                   \
    FIELD_LIST(DECLARE)                                                   \
    VALIDATOR_LIST(DECLARE_VALIDATOR)                                     \
                                                                              \
    structName() {}                                                       \
                                                                              \
    SerializationResult<bool> validate() const {                            \
        VALIDATOR_LIST(VALIDATE_FIELD)                                    \
        return SerializationResult<bool>::Success(true);                    \
    }                                                                         \
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
    static SerializationResult<structName> deserializeWithResult(const String& jsonStr, bool validateData = true) { \
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
        if (validateData) {                                                   \
            auto validationResult = data.validate();                         \
            if (!validationResult.success) {                                 \
                return SerializationResult<structName>::Failure(             \
                    validationResult.error.code,                             \
                    validationResult.error.message,                          \
                    validationResult.error.fieldPath);                       \
            }                                                                 \
        }                                                                     \
        return SerializationResult<structName>::Success(data);               \
    }                                                                         \
                                                                              \
    static SerializationResult<structName> deserializeWithResult(const JsonObject& o, bool validateData = true) { \
        structName data;                                                      \
        FIELD_LIST(DESERIALIZE_FIELD)                                     \
        if (validateData) {                                                   \
            auto validationResult = data.validate();                         \
            if (!validationResult.success) {                                 \
                return SerializationResult<structName>::Failure(             \
                    validationResult.error.code,                             \
                    validationResult.error.message,                          \
                    validationResult.error.fieldPath);                       \
            }                                                                 \
        }                                                                     \
        return SerializationResult<structName>::Success(data);               \
    }                                                                         \
                                                                              \
    static structName deserialize(const String& jsonStr, bool validateData = false) { \
        auto result = deserializeWithResult(jsonStr, validateData);          \
        return result.success ? result.data : structName();                  \
    }                                                                         \
                                                                              \
    static structName deserialize(const JsonObject& o, bool validateData = false) { \
        auto result = deserializeWithResult(o, validateData);                \
        return result.success ? result.data : structName();                  \
    }                                                                         \
                                                                              \
    static void printStructDefinition() {                                    \
        Serial.println("=== " #structName " Struct Definition (WITH VALIDATION) ==="); \
        Serial.println("Struct Name: " #structName);                         \
        Serial.println("Generated Methods:");                                 \
        Serial.println("  - serialize() -> String");                         \
        Serial.println("  - serializeWithResult() -> SerializationResult<String>"); \
        Serial.println("  - deserialize(String, validate=false) -> " #structName); \
        Serial.println("  - deserialize(JsonObject, validate=false) -> " #structName); \
        Serial.println("  - deserializeWithResult(String, validate=true) -> SerializationResult<" #structName ">"); \
        Serial.println("  - deserializeWithResult(JsonObject, validate=true) -> SerializationResult<" #structName ">"); \
        Serial.println("  - validate() -> SerializationResult<bool>");      \
        Serial.println("  - printStructDefinition() -> void");               \
        Serial.println("  - printFieldInfo() -> void");                      \
        Serial.println("  - printCurrentValues() -> void");                  \
        Serial.println();                                                     \
        Serial.println("Usage Example:");                                     \
        Serial.println("  " #structName " obj;");                            \
        Serial.println("  String json = obj.serialize();");                  \
        Serial.println("  auto result = " #structName "::deserializeWithResult(json);"); \
        Serial.println("  if (!result.success) {");                          \
        Serial.println("    Serial.println(result.error.toString());");      \
        Serial.println("  }");                                                \
        Serial.println("=======================================");             \
    }                                                                         \
                                                                              \
    static void printFieldInfo() {                                           \
        Serial.println("=== " #structName " Field Information (WITH VALIDATION) ==="); \
        Serial.println("This struct includes automatic validation on deserialization."); \
        Serial.println("To see actual field values, create an instance and call printCurrentValues()"); \
        Serial.println();                                                     \
        Serial.println("Validation is performed automatically in deserializeWithResult()"); \
        Serial.println("You can also manually validate with: obj.validate()"); \
        Serial.println();                                                     \
        Serial.println("For detailed macro writing guide with validation, call:"); \
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
        Serial.println();                                                     \
        Serial.println("Validation Status:");                                \
        auto validationResult = validate();                                  \
        if (validationResult.success) {                                      \
            Serial.println("  ✓ All validations passed");                   \
        } else {                                                              \
            Serial.println("  ✗ Validation failed:");                       \
            Serial.println("    " + validationResult.error.toString());     \
        }                                                                     \
        Serial.println("=====================================");             \
    }                                                                         \
};

#endif // STRUCTA_H