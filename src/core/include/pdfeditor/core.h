#pragma once

#include <string>
#include <memory>
#include <cstdint>
#include <vector>
#include <functional>

// Platform-specific export/import macros
#ifdef _WIN32
    #ifdef PDFEDITOR_EXPORTS
        #define PDFEDITOR_API __declspec(dllexport)
    #else
        #define PDFEDITOR_API __declspec(dllimport)
    #endif
#else
    #define PDFEDITOR_API __attribute__((visibility("default")))
#endif

namespace pdfeditor {

// Forward declarations
class Document;
class Page;
class Renderer;
class Annotation;

// Version information
constexpr int VERSION_MAJOR = 1;
constexpr int VERSION_MINOR = 0;
constexpr int VERSION_PATCH = 0;

// Error codes
enum class ErrorCode {
    Success = 0,
    FileNotFound,
    InvalidPDF,
    PasswordRequired,
    PermissionDenied,
    OutOfMemory,
    NotImplemented,
    InvalidArgument,
    IOError,
    EncryptionError,
    SignatureError,
    OCRError,
    RenderError,
    UnknownError
};

// PDF version enum
enum class PDFVersion {
    PDF_1_0,
    PDF_1_1,
    PDF_1_2,
    PDF_1_3,
    PDF_1_4,
    PDF_1_5,
    PDF_1_6,
    PDF_1_7,
    PDF_2_0
};

// Page layout modes
enum class PageLayout {
    SinglePage,      // Display one page at a time
    Continuous,      // Display pages in a continuous column
    TwoPageLeft,     // Display pages in two columns with odd pages on left
    TwoPageRight,    // Display pages in two columns with odd pages on right
    TwoPageContinuous // Continuous two-column layout
};

// Page rotation
enum class PageRotation {
    None = 0,
    Clockwise90 = 90,
    Clockwise180 = 180,
    Clockwise270 = 270
};

// Permission flags
enum class Permission : uint32_t {
    None = 0,
    Print = 1 << 0,
    Modify = 1 << 1,
    Copy = 1 << 2,
    ModifyAnnotations = 1 << 3,
    FillForms = 1 << 4,
    ExtractForAccessibility = 1 << 5,
    Assemble = 1 << 6,
    PrintHighQuality = 1 << 7,
    All = 0xFFFFFFFF
};

// Compression options
enum class CompressionLevel {
    None,
    Fast,
    Default,
    Maximum
};

// Color space
enum class ColorSpace {
    DeviceGray,
    DeviceRGB,
    DeviceCMYK,
    CalGray,
    CalRGB,
    Lab,
    ICCBased,
    Indexed,
    Pattern,
    Separation,
    DeviceN
};

// Rectangle structure
struct Rect {
    float x0, y0;  // Bottom-left corner
    float x1, y1;  // Top-right corner
    
    Rect() : x0(0), y0(0), x1(0), y1(0) {}
    Rect(float x0, float y0, float x1, float y1) 
        : x0(x0), y0(y0), x1(x1), y1(y1) {}
    
    float width() const { return x1 - x0; }
    float height() const { return y1 - y0; }
    bool is_empty() const { return width() <= 0 || height() <= 0; }
};

// Point structure
struct Point {
    float x, y;
    
    Point() : x(0), y(0) {}
    Point(float x, float y) : x(x), y(y) {}
};

// Color structure (RGBA)
struct Color {
    float r, g, b, a;
    
    Color() : r(0), g(0), b(0), a(1) {}
    Color(float r, float g, float b, float a = 1.0f) 
        : r(r), g(g), b(b), a(a) {}
    
    static Color black() { return Color(0, 0, 0); }
    static Color white() { return Color(1, 1, 1); }
    static Color red() { return Color(1, 0, 0); }
    static Color green() { return Color(0, 1, 0); }
    static Color blue() { return Color(0, 0, 1); }
    static Color yellow() { return Color(1, 1, 0); }
};

// Result type for error handling
template<typename T>
class Result {
public:
    Result(const T& value) : value_(value), error_(ErrorCode::Success) {}
    Result(ErrorCode error) : error_(error) {}
    Result(ErrorCode error, const std::string& message) 
        : error_(error), error_message_(message) {}
    
    bool is_ok() const { return error_ == ErrorCode::Success; }
    bool is_error() const { return !is_ok(); }
    
    const T& value() const { return value_; }
    T& value() { return value_; }
    
    ErrorCode error() const { return error_; }
    const std::string& error_message() const { return error_message_; }
    
    operator bool() const { return is_ok(); }
    
private:
    T value_;
    ErrorCode error_;
    std::string error_message_;
};

// Progress callback type
using ProgressCallback = std::function<bool(int current, int total, const std::string& message)>;

// Library initialization and cleanup
class PDFEDITOR_API Library {
public:
    // Initialize the library
    static bool initialize();
    
    // Shutdown the library
    static void shutdown();
    
    // Check if library is initialized
    static bool is_initialized();
    
    // Get version string
    static std::string get_version();
    
    // Get version components
    static void get_version(int& major, int& minor, int& patch);
    
    // Set global cache size (in MB)
    static void set_cache_size(size_t size_mb);
    
    // Get last error message
    static std::string get_last_error();
    
    // Set log callback
    using LogCallback = std::function<void(const std::string& message)>;
    static void set_log_callback(LogCallback callback);
    
private:
    Library() = delete;
};

// Exception class
class PDFEDITOR_API Exception : public std::exception {
public:
    explicit Exception(ErrorCode code, const std::string& message = "")
        : code_(code), message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
    ErrorCode code() const { return code_; }
    
private:
    ErrorCode code_;
    std::string message_;
};

// Utility functions
namespace util {
    // Convert error code to string
    PDFEDITOR_API std::string error_to_string(ErrorCode code);
    
    // Check if file exists
    PDFEDITOR_API bool file_exists(const std::string& path);
    
    // Get file size
    PDFEDITOR_API size_t get_file_size(const std::string& path);
    
    // Get temporary directory
    PDFEDITOR_API std::string get_temp_directory();
    
    // Generate unique ID
    PDFEDITOR_API std::string generate_uuid();
}

} // namespace pdfeditor
