// ============================================
// Structa V2.4 - Flexible Field Validation
// ============================================
#ifndef STRUCTA_H
#define STRUCTA_H

#include <ArduinoJson.h>
#include <math.h>

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
        switch (code) {
            case SerializationError::SUCCESS: return "Success";
            case SerializationError::BUFFER_OVERFLOW: result += "Buffer overflow"; break;
            case SerializationError::INVALID_JSON: result += "Invalid JSON"; break;
            case SerializationError::TYPE_MISMATCH: result += "Type mismatch"; break;
            case SerializationError::FIELD_MISSING: result += "Field missing"; break;
            case SerializationError::MEMORY_ALLOCATION_FAILED: result += "Memory allocation failed"; break;
        }
        if (message.length() > 0) result += ": " + message;
        if (fieldPath.length() > 0) result += " (field: " + fieldPath + ")";
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
        SerializationResult<T> r; r.success = true; r.data = value; return r;
    }
    static SerializationResult<T> Failure(SerializationError code, const String& msg, const String& path = "") {
        SerializationResult<T> r; r.success = false; r.error = ErrorInfo(code, msg, path); return r;
    }
    operator bool() const { return success; }
};

template<>
struct SerializationResult<void> {
    bool success;
    ErrorInfo error;
    static SerializationResult<void> Success() { SerializationResult<void> r; r.success = true; return r; }
    static SerializationResult<void> Failure(SerializationError code, const String& msg, const String& path = "") {
        SerializationResult<void> r; r.success = false; r.error = ErrorInfo(code, msg, path); return r;
    }
    operator bool() const { return success; }
};

// ======================================================
// Memory Tracker
// ======================================================
class MemoryTracker {
private:
    static size_t totalAllocated;
    static size_t peakUsage;

public:
    static void recordAllocation(size_t size) {
        totalAllocated += size;
        if (totalAllocated > peakUsage) peakUsage = totalAllocated;
    }
    static void recordDeallocation(size_t size) {
        if (totalAllocated >= size) totalAllocated -= size;
    }
    static void printStats() {
        Serial.println("Memory - Current: " + String(totalAllocated) +
                       " bytes, Peak: " + String(peakUsage) + " bytes");
    }
};
size_t MemoryTracker::totalAllocated = 0;
size_t MemoryTracker::peakUsage = 0;

// ======================================================
// Type Detection
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
    template<typename T>
    static typename std::enable_if<!HasSerialize<T>::value, void>::type
    serializeField(JsonObject& obj, const char* key, const T& value) {
        obj[key] = value;
    }

    template<typename T>
    static typename std::enable_if<HasSerialize<T>::value, void>::type
    serializeField(JsonObject& obj, const char* key, const T& value) {
        DynamicJsonDocument sub(256);
        if (deserializeJson(sub, value.serialize()) == DeserializationError::Ok)
            obj[key] = sub.as<JsonObject>();
    }

    template<typename T>
    static typename std::enable_if<!HasSerialize<T>::value, void>::type
    deserializeField(const JsonObject& obj, const char* key, T& value) {
        if (obj.containsKey(key)) value = obj[key].as<T>();
    }

    template<typename T>
    static typename std::enable_if<HasSerialize<T>::value, void>::type
    deserializeField(const JsonObject& obj, const char* key, T& value) {
        if (obj.containsKey(key) && obj[key].is<JsonObject>()) {
            String nestedJson;
            serializeJson(obj[key].as<JsonObject>(), nestedJson);
            value = T::deserialize(nestedJson);
        }
    }
};

// ======================================================
// Schema + Metadata
// ======================================================
enum class FieldType { INT, FLOAT, BOOL, STRING, OBJECT, UNKNOWN };

struct FieldSchema {
    const char* name;
    FieldType type;
    bool required;
    bool validate;
    float minValue;
    float maxValue;
    int minLength;
    int maxLength;
    const char** allowedValues;
    size_t allowedCount;
};

template<typename T, bool hasSerialize = HasSerialize<T>::value>
struct StructaTypeResolverImpl { static constexpr FieldType value = FieldType::UNKNOWN; };
template<typename T> struct StructaTypeResolverImpl<T, true> { static constexpr FieldType value = FieldType::OBJECT; };
template<typename T> struct StructaTypeResolver { static constexpr FieldType value = StructaTypeResolverImpl<T>::value; };
template<> struct StructaTypeResolver<int> { static constexpr FieldType value = FieldType::INT; };
template<> struct StructaTypeResolver<long> { static constexpr FieldType value = FieldType::INT; };
template<> struct StructaTypeResolver<unsigned int> { static constexpr FieldType value = FieldType::INT; };
template<> struct StructaTypeResolver<unsigned long> { static constexpr FieldType value = FieldType::INT; };
template<> struct StructaTypeResolver<float> { static constexpr FieldType value = FieldType::FLOAT; };
template<> struct StructaTypeResolver<double> { static constexpr FieldType value = FieldType::FLOAT; };
template<> struct StructaTypeResolver<bool> { static constexpr FieldType value = FieldType::BOOL; };
template<> struct StructaTypeResolver<String> { static constexpr FieldType value = FieldType::STRING; };

struct FieldMeta {
    float minValue;
    float maxValue;
    int minLength;
    int maxLength;
    const char** allowedValues;
    size_t allowedCount;
    bool required;
    bool validate;
    
    FieldMeta() : minValue(NAN), maxValue(NAN), minLength(-1), maxLength(-1), 
                  allowedValues(nullptr), allowedCount(0), required(true), validate(true) {}
};

// ======================================================
// Helper Functions for Metadata
// ======================================================
inline FieldMeta makeMetaNone() {
    FieldMeta m;
    m.validate = false;
    return m;
}

inline FieldMeta makeMetaOptional() {
    FieldMeta m;
    m.required = false;
    return m;
}

inline FieldMeta makeMetaOptionalUnvalidated() {
    FieldMeta m;
    m.required = false;
    m.validate = false;
    return m;
}

inline FieldMeta makeMetaRange(float minV, float maxV) {
    FieldMeta m;
    m.minValue = minV;
    m.maxValue = maxV;
    return m;
}

inline FieldMeta makeMetaStrlen(int minL, int maxL) {
    FieldMeta m;
    m.minLength = minL;
    m.maxLength = maxL;
    return m;
}

inline FieldMeta makeMetaEnum(const char** values, size_t count) {
    FieldMeta m;
    m.allowedValues = values;
    m.allowedCount = count;
    return m;
}

// ======================================================
// Metadata Macros
// ======================================================
#define META_NONE() makeMetaNone()
#define META_OPTIONAL() makeMetaOptional()
#define META_OPTIONAL_UNVALIDATED() makeMetaOptionalUnvalidated()
#define META_RANGE(minV, maxV) makeMetaRange((minV), (maxV))
#define META_STRLEN(minL, maxL) makeMetaStrlen((minL), (maxL))
#define META_ENUM(valuesArray) makeMetaEnum((valuesArray), sizeof(valuesArray)/sizeof(valuesArray[0]))

// ======================================================
// Macros
// ======================================================
#define DECLARE(type, name, meta) type name;
#define SERIALIZE_FIELD(type, name, meta) serializeField(obj, #name, name);
#define DESERIALIZE_FIELD(type, name, meta) deserializeField(o, #name, data.name);
#define SCHEMA_ENTRY(type, name, meta) \
    { #name, StructaTypeResolver<type>::value, (meta).required, (meta).validate, (meta).minValue, (meta).maxValue, \
      (meta).minLength, (meta).maxLength, (meta).allowedValues, (meta).allowedCount },

// ======================================================
// DEFINE_STRUCTA (final)
// ======================================================
#define DEFINE_STRUCTA(structName, FIELD_LIST)                                      \
struct structName : public JsonStruct {                                              \
    FIELD_LIST(DECLARE)                                                              \
                                                                                     \
    SerializationResult<void> validateSelf() const {                                 \
        DynamicJsonDocument doc(512);                                                \
        JsonObject obj = doc.to<JsonObject>();                                       \
        FIELD_LIST(SERIALIZE_FIELD)                                                  \
        return validateSchema(obj);                                                  \
    }                                                                                \
                                                                                     \
    SerializationResult<String> serializeWithResult() const {                        \
        auto validation = validateSelf();                                            \
        if (!validation.success) {                                                   \
            return SerializationResult<String>::Failure(                             \
                validation.error.code, validation.error.message, validation.error.fieldPath); \
        }                                                                            \
        DynamicJsonDocument doc(512);                                                \
        MemoryTracker::recordAllocation(512);                                        \
        JsonObject obj = doc.to<JsonObject>();                                       \
        FIELD_LIST(SERIALIZE_FIELD)                                                  \
        String result;                                                               \
        if (serializeJson(doc, result) == 0) {                                       \
            MemoryTracker::recordDeallocation(512);                                  \
            return SerializationResult<String>::Failure(                             \
                SerializationError::INVALID_JSON, "Failed to serialize");            \
        }                                                                            \
        MemoryTracker::recordDeallocation(512);                                      \
        return SerializationResult<String>::Success(result);                         \
    }                                                                                \
                                                                                     \
    String serialize() const { auto r = serializeWithResult(); return r.success ? r.data : "{}"; } \
                                                                                     \
    static const FieldSchema* getSchema(size_t& count) {                             \
        static const FieldSchema schema[] = { FIELD_LIST(SCHEMA_ENTRY) };            \
        count = sizeof(schema) / sizeof(schema[0]);                                  \
        return schema;                                                               \
    }                                                                                \
                                                                                     \
    static SerializationResult<void> validateSchema(const JsonObject& o) {           \
        size_t n; const FieldSchema* schema = getSchema(n);                          \
        for (size_t i = 0; i < n; ++i) {                                             \
            const FieldSchema& f = schema[i];                                        \
                                                                                     \
            if (!f.validate) continue;                                               \
                                                                                     \
            if (f.required && !o.containsKey(f.name))                                \
                return SerializationResult<void>::Failure(                           \
                    SerializationError::FIELD_MISSING, "Required field missing", f.name); \
            if (!o.containsKey(f.name)) continue;                                    \
            JsonVariant v = o[f.name];                                               \
            bool mismatch = false;                                                   \
            switch (f.type) {                                                        \
                case FieldType::INT: {                                               \
                    mismatch = !v.is<long>() && !v.is<int>();                        \
                    if (!mismatch) {                                                 \
                        long val = v.as<long>();                                     \
                        if (!isnan(f.minValue) && val < (long)f.minValue)            \
                            return SerializationResult<void>::Failure(                \
                                SerializationError::TYPE_MISMATCH, "Value below min", f.name); \
                        if (!isnan(f.maxValue) && val > (long)f.maxValue)            \
                            return SerializationResult<void>::Failure(                \
                                SerializationError::TYPE_MISMATCH, "Value above max", f.name); \
                    }                                                                 \
                    break; }                                                         \
                case FieldType::FLOAT: {                                             \
                    mismatch = !(v.is<float>() || v.is<double>());                   \
                    if (!mismatch) {                                                 \
                        float val = v.as<float>();                                   \
                        if (!isnan(f.minValue) && val < f.minValue)                  \
                            return SerializationResult<void>::Failure(                \
                                SerializationError::TYPE_MISMATCH, "Value below min", f.name); \
                        if (!isnan(f.maxValue) && val > f.maxValue)                  \
                            return SerializationResult<void>::Failure(                \
                                SerializationError::TYPE_MISMATCH, "Value above max", f.name); \
                    }                                                                 \
                    break; }                                                         \
                case FieldType::BOOL: mismatch = !v.is<bool>(); break;               \
                case FieldType::STRING: {                                            \
                    mismatch = !v.is<const char*>();                                 \
                    if (!mismatch) {                                                 \
                        const char* s = v.as<const char*>();                         \
                        int len = strlen(s);                                         \
                        if (f.minLength >= 0 && len < f.minLength)                   \
                            return SerializationResult<void>::Failure(                \
                                SerializationError::TYPE_MISMATCH, "String too short", f.name); \
                        if (f.maxLength >= 0 && len > f.maxLength)                   \
                            return SerializationResult<void>::Failure(                \
                                SerializationError::TYPE_MISMATCH, "String too long", f.name); \
                        if (f.allowedValues) {                                       \
                            bool ok = false;                                         \
                            for (size_t j = 0; j < f.allowedCount; ++j)              \
                                if (strcmp(s, f.allowedValues[j]) == 0) { ok = true; break; } \
                            if (!ok)                                                 \
                                return SerializationResult<void>::Failure(            \
                                    SerializationError::TYPE_MISMATCH, "Invalid enum value", f.name); \
                        }                                                             \
                    } break; }                                                       \
                case FieldType::OBJECT: mismatch = !v.is<JsonObject>(); break;       \
                default: break;                                                      \
            }                                                                         \
            if (mismatch)                                                            \
                return SerializationResult<void>::Failure(                           \
                    SerializationError::TYPE_MISMATCH, "Expected different type", f.name); \
        }                                                                             \
        return SerializationResult<void>::Success();                                  \
    }                                                                                \
                                                                                     \
    static SerializationResult<structName> deserializeWithResult(const String& jsonStr) { \
        DynamicJsonDocument doc(512);                                                \
        MemoryTracker::recordAllocation(512);                                        \
        DeserializationError err = deserializeJson(doc, jsonStr);                    \
        if (err) {                                                                   \
            MemoryTracker::recordDeallocation(512);                                  \
            return SerializationResult<structName>::Failure(                         \
                SerializationError::INVALID_JSON, String("Parse error: ") + err.c_str()); \
        }                                                                             \
        JsonObject o = doc.as<JsonObject>();                                         \
        auto val = validateSchema(o);                                                \
        if (!val.success) {                                                          \
            MemoryTracker::recordDeallocation(512);                                  \
            return SerializationResult<structName>::Failure(                         \
                val.error.code, val.error.message, val.error.fieldPath);             \
        }                                                                             \
        structName data;                                                              \
        FIELD_LIST(DESERIALIZE_FIELD)                                                \
        MemoryTracker::recordDeallocation(512);                                      \
        return SerializationResult<structName>::Success(data);                       \
    }                                                                                \
                                                                                     \
    static structName deserialize(const String& jsonStr) {                           \
        auto r = deserializeWithResult(jsonStr);                                     \
        return r.success ? r.data : structName();                                    \
    }                                                                                \
                                                                                     \
    static void printSchema() {                                                      \
        size_t n; const FieldSchema* s = getSchema(n);                               \
        Serial.println("=== " #structName " Schema ===");                            \
        for (size_t i = 0; i < n; ++i) {                                             \
            const FieldSchema& f = s[i];                                             \
            Serial.print(" - "); Serial.print(f.name); Serial.print(" [");           \
            switch (f.type) {                                                        \
                case FieldType::INT: Serial.print("int"); break;                     \
                case FieldType::FLOAT: Serial.print("float"); break;                 \
                case FieldType::BOOL: Serial.print("bool"); break;                   \
                case FieldType::STRING: Serial.print("string"); break;               \
                case FieldType::OBJECT: Serial.print("object"); break;               \
                default: Serial.print("unknown"); }                                  \
            Serial.print("]");                                                       \
            if (!f.required) Serial.print(" (optional)");                            \
            if (!f.validate) Serial.print(" (unvalidated)");                         \
            Serial.println();                                                        \
        }                                                                             \
        Serial.println("===========================");                               \
    }                                                                                \
                                                                             \
};

// ======================================================
// Helper Class for Guidance
// ======================================================
class StructaHelper {
public:
    static void showMacroWritingGuide() {
        Serial.println("╔════════════════════════════════════════════════════════╗");
        Serial.println("║        STRUCTA MACRO WRITING GUIDE                     ║");
        Serial.println("╚════════════════════════════════════════════════════════╝");
        Serial.println();
        Serial.println("1. BASIC SYNTAX");
        Serial.println("   #define STRUCT_NAME_FIELDS(field) \\");
        Serial.println("       field(Type, name, META_RULE) \\");
        Serial.println("       field(Type, name, META_RULE) \\");
        Serial.println("       field(Type, name, META_RULE)");
        Serial.println();
        Serial.println("   DEFINE_STRUCTA(StructName, STRUCT_NAME_FIELDS)");
        Serial.println();
        Serial.println("2. FIELD PATTERN: field(TYPE, NAME, METADATA)");
        Serial.println("   - TYPE: int, float, bool, String, or custom struct");
        Serial.println("   - NAME: variable identifier (camelCase recommended)");
        Serial.println("   - METADATA: validation rule (see section 5)");
        Serial.println();
        Serial.println("3. IMPORTANT RULES");
        Serial.println("   ✓ Each line ends with \\ (except last line)");
        Serial.println("   ✓ NO semicolons at end of field lines");
        Serial.println("   ✓ NO commas between field definitions");
        Serial.println("   ✓ NO comments inside the macro");
        Serial.println("   ✓ Exactly 3 args per field: (type, name, meta)");
        Serial.println();
        Serial.println("4. CORRECT EXAMPLE");
        Serial.println("   #define USER_FIELDS(field) \\");
        Serial.println("       field(String, username, META_STRLEN(3, 20)) \\");
        Serial.println("       field(int, age, META_RANGE(18, 100)) \\");
        Serial.println("       field(bool, active, META_NONE())");
        Serial.println();
        Serial.println("   DEFINE_STRUCTA(User, USER_FIELDS)");
        Serial.println();
        Serial.println("5. METADATA OPTIONS");
        Serial.println("   META_NONE()           - No validation");
        Serial.println("   META_OPTIONAL()       - Optional, validated if present");
        Serial.println("   META_RANGE(min, max)  - Numeric range validation");
        Serial.println("   META_STRLEN(min, max) - String length validation");
        Serial.println("   META_ENUM(array)      - Enum value validation");
        Serial.println();
        Serial.println("6. SHORTHAND MACROS (Optional)");
        Serial.println("   Define once at top of file:");
        Serial.println("   #define V(t,n,m) field(t,n,m)  // Validated");
        Serial.println("   #define N(t,n) field(t,n,META_NONE())  // Not validated");
        Serial.println("   #define O(t,n) field(t,n,META_OPTIONAL())  // Optional");
        Serial.println();
        Serial.println("   Usage:");
        Serial.println("   #define USER_FIELDS(field)             \\");
        Serial.println("       V(String, name, META_STRLEN(3,20)) \\");
        Serial.println("       V(int, age, META_RANGE(18,100))    \\");
        Serial.println("       O(String, email)                   \\");
        Serial.println("       N(bool, internal)");
        Serial.println();
        Serial.println("7. COMMON MISTAKES");
        Serial.println("   ✗ field(String, name,, META_NONE())  // double comma");
        Serial.println("   ✗ field(String, name, META_NONE());  // semicolon");
        Serial.println("   ✗ field(String, name, META_NONE()) \\  // comment");
        Serial.println("       field(int, age, META_NONE())  // missing \\");
        Serial.println();
        Serial.println("8. NESTED STRUCTS");
        Serial.println("   Define inner struct first:");
        Serial.println("   #define ADDRESS_FIELDS(field)              \\");
        Serial.println("       field(String, city, META_NONE())       \\");
        Serial.println("       field(int, zip, META_NONE())");
        Serial.println("   DEFINE_STRUCTA(Address, ADDRESS_FIELDS)");
        Serial.println();
        Serial.println("   Then use in outer struct:");
        Serial.println("   #define USER_FIELDS(field)                 \\");
        Serial.println("       field(String, name, META_STRLEN(3,20)) \\");
        Serial.println("       field(Address, address, META_OPTIONAL())");
        Serial.println("   DEFINE_STRUCTA(User, USER_FIELDS)");
        Serial.println();
        Serial.println("9. ENUM VALIDATION");
        Serial.println("   Declare array BEFORE field definition:");
        Serial.println("   const char* roles[] = {\"admin\", \"user\", \"guest\"};");
        Serial.println();
        Serial.println("   #define USER_FIELDS(field) \\");
        Serial.println("       field(String, role, META_ENUM(roles))");
        Serial.println();
        Serial.println("10. COMPLETE EXAMPLE");
        Serial.println("    const char* status[] = {\"active\", \"inactive\"};");
        Serial.println();
        Serial.println("    #define DEVICE_FIELDS(field)                    \\");
        Serial.println("        field(String, deviceId, META_STRLEN(5,20))  \\");
        Serial.println("        field(String, status, META_ENUM(status))    \\");
        Serial.println("        field(float, temp, META_RANGE(-40.0,125.0)) \\");
        Serial.println("        field(int, battery, META_RANGE(0,100))      \\");
        Serial.println("        field(bool, online, META_NONE())            \\");
        Serial.println("        field(String, notes, META_OPTIONAL())");
        Serial.println();
        Serial.println("    DEFINE_STRUCTA(Device, DEVICE_FIELDS)");
        Serial.println();
        Serial.println("════════════════════════════════════════════════════════");
    }
    
    static void showQuickReference() {
        Serial.println("╔═══════════════════════════════════╗");
        Serial.println("║  STRUCTA QUICK REFERENCE          ║");
        Serial.println("╚═══════════════════════════════════╝");
        Serial.println();
        Serial.println("VALIDATION MACROS:");
        Serial.println("  META_NONE()              No validation");
        Serial.println("  META_OPTIONAL()          Optional field");
        Serial.println("  META_RANGE(min, max)     Numeric range");
        Serial.println("  META_STRLEN(min, max)    String length");
        Serial.println("  META_ENUM(array)         Enum values");
        Serial.println();
        Serial.println("SHORTHAND (define yourself):");
        Serial.println("  V(t,n,m)  Validated field");
        Serial.println("  N(t,n)    No validation");
        Serial.println("  O(t,n)    Optional");
        Serial.println();
        Serial.println("METHODS:");
        Serial.println("  .serialize()             → String");
        Serial.println("  .serializeWithResult()   → Result<String>");
        Serial.println("  ::deserialize(json)      → Struct");
        Serial.println("  ::deserializeWithResult(json) → Result<Struct>");
        Serial.println("  ::printSchema()          Show fields");
        Serial.println();
        Serial.println("══════════════════════════════════");
    }
};

#endif // STRUCTA_H