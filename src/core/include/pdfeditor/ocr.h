#pragma once

#include "core.h"
#include "document.h"
#include <string>
#include <vector>

namespace pdfeditor {

// OCR engine type
enum class OCREngine {
    Tesseract,
    System  // Use system OCR (macOS Vision, Windows OCR)
};

// OCR language
struct OCRLanguage {
    std::string code;        // ISO 639-2/3 code (e.g., "eng", "fra")
    std::string name;        // Full name (e.g., "English", "French")
    bool installed;
    
    OCRLanguage(const std::string& c, const std::string& n, bool i = true)
        : code(c), name(n), installed(i) {}
};

// Page segmentation mode
enum class PageSegMode {
    OSD_Only = 0,           // Orientation and script detection only
    AutoOSD = 1,            // Auto with OSD
    AutoOnly = 2,           // Auto without OSD
    Auto = 3,               // Fully automatic (default)
    SingleColumn = 4,       // Single uniform block of text
    SingleBlockVertical = 5, // Single vertical block
    SingleBlock = 6,        // Single uniform block
    SingleLine = 7,         // Single text line
    SingleWord = 8,         // Single word
    CircleWord = 9,         // Circle single word
    SingleChar = 10,        // Single character
    SparseText = 11,        // Sparse text
    SparseTextOSD = 12,     // Sparse text with OSD
    RawLine = 13            // Raw line (special mode)
};

// OCR result confidence level
enum class ConfidenceLevel {
    VeryLow,    // < 60%
    Low,        // 60-70%
    Medium,     // 70-80%
    High,       // 80-90%
    VeryHigh    // > 90%
};

// Recognized word
struct OCRWord {
    std::string text;
    Rect bounding_box;
    float confidence;
    ConfidenceLevel confidence_level;
    std::string language;
};

// Recognized line
struct OCRLine {
    std::string text;
    Rect bounding_box;
    float confidence;
    std::vector<OCRWord> words;
};

// Recognized paragraph
struct OCRParagraph {
    std::string text;
    Rect bounding_box;
    float confidence;
    std::vector<OCRLine> lines;
};

// OCR result for a page
struct OCRResult {
    int page_index;
    std::string full_text;
    float average_confidence;
    std::vector<OCRParagraph> paragraphs;
    std::vector<OCRLine> lines;
    std::vector<OCRWord> words;
};

// OCR options
struct OCROptions {
    std::vector<std::string> languages;  // Language codes
    PageSegMode page_seg_mode;
    OCREngine engine;
    float min_confidence;                // Filter out results below this
    bool deskew;                         // Auto-straighten images
    bool remove_noise;                   // Denoise images
    bool enhance_contrast;               // Enhance image contrast
    int dpi;                             // DPI for rendering
    bool preserve_layout;                // Try to preserve original layout
    bool create_searchable_pdf;          // Embed text layer
    
    OCROptions()
        : page_seg_mode(PageSegMode::Auto)
        , engine(OCREngine::Tesseract)
        , min_confidence(0.0f)
        , deskew(true)
        , remove_noise(true)
        , enhance_contrast(true)
        , dpi(300)
        , preserve_layout(true)
        , create_searchable_pdf(true) {
        languages.push_back("eng");  // Default to English
    }
};

// OCR class
class PDFEDITOR_API OCR {
public:
    // ===== Engine Management =====
    
    // Initialize OCR engine
    static bool initialize(OCREngine engine = OCREngine::Tesseract);
    
    // Shutdown OCR engine
    static void shutdown();
    
    // Check if engine is initialized
    static bool is_initialized();
    
    // Get available engines
    static std::vector<OCREngine> get_available_engines();
    
    // ===== Language Management =====
    
    // List all available languages
    static std::vector<OCRLanguage> list_languages();
    
    // List installed languages
    static std::vector<OCRLanguage> list_installed_languages();
    
    // Check if language is installed
    static bool is_language_installed(const std::string& language_code);
    
    // Download/install language data
    static bool install_language(const std::string& language_code);
    
    // Get language name from code
    static std::string get_language_name(const std::string& code);
    
    // Detect language in image
    static std::vector<std::string> detect_languages(const uint8_t* image_data, size_t size);
    
    // ===== OCR Operations =====
    
    // Perform OCR on single page
    static Result<OCRResult> ocr_page(
        Page* page,
        const OCROptions& options = OCROptions()
    );
    
    // Perform OCR on multiple pages
    static std::vector<Result<OCRResult>> ocr_pages(
        Document* doc,
        const std::vector<int>& page_indices,
        const OCROptions& options = OCROptions(),
        ProgressCallback callback = nullptr
    );
    
    // Perform OCR on entire document
    static std::vector<Result<OCRResult>> ocr_document(
        Document* doc,
        const OCROptions& options = OCROptions(),
        ProgressCallback callback = nullptr
    );
    
    // OCR specific area on page
    static Result<OCRResult> ocr_area(
        Page* page,
        const Rect& area,
        const OCROptions& options = OCROptions()
    );
    
    // ===== Image OCR =====
    
    // OCR from image file
    static Result<OCRResult> ocr_image(
        const std::string& image_path,
        const OCROptions& options = OCROptions()
    );
    
    // OCR from image data
    static Result<OCRResult> ocr_image_data(
        const uint8_t* image_data,
        size_t size,
        int width,
        int height,
        const OCROptions& options = OCROptions()
    );
    
    // ===== Searchable PDF Creation =====
    
    // Convert scanned PDF to searchable PDF
    static bool make_searchable(
        Document* doc,
        const OCROptions& options = OCROptions(),
        ProgressCallback callback = nullptr
    );
    
    // Add text layer to page
    static bool add_text_layer(
        Page* page,
        const OCRResult& ocr_result
    );
    
    // Remove text layer from page
    static bool remove_text_layer(Page* page);
    
    // Check if page has OCR text layer
    static bool has_text_layer(Page* page);
    
    // ===== Layout Analysis =====
    
    // Detect page orientation
    enum class Orientation {
        Portrait,
        Landscape,
        RotatedLeft,
        RotatedRight
    };
    static Orientation detect_orientation(Page* page);
    
    // Auto-rotate page to correct orientation
    static bool auto_rotate(Page* page);
    
    // Detect columns
    static std::vector<Rect> detect_columns(Page* page);
    
    // Detect reading order
    static std::vector<Rect> detect_reading_order(Page* page);
    
    // ===== Quality Assessment =====
    
    // Assess image quality for OCR
    struct QualityAssessment {
        float overall_quality;      // 0.0 to 1.0
        float sharpness;
        float contrast;
        float brightness;
        bool skewed;
        float skew_angle;
        bool noisy;
        bool recommended_for_ocr;
        std::vector<std::string> issues;
        std::vector<std::string> recommendations;
    };
    
    static QualityAssessment assess_quality(Page* page);
    
    // Enhance image for better OCR
    static bool enhance_for_ocr(Page* page);
    
    // ===== Batch Processing =====
    
    // OCR multiple documents
    struct BatchOCRJob {
        std::string input_path;
        std::string output_path;
        OCROptions options;
    };
    
    static bool batch_ocr(
        const std::vector<BatchOCRJob>& jobs,
        ProgressCallback callback = nullptr
    );
    
    // ===== Export Options =====
    
    // Export OCR result to text
    static std::string export_text(const OCRResult& result);
    
    // Export OCR result to JSON
    static std::string export_json(const OCRResult& result);
    
    // Export OCR result to hOCR (HTML-based format)
    static std::string export_hocr(const OCRResult& result);
    
    // Export OCR result to ALTO XML
    static std::string export_alto(const OCRResult& result);
    
    // ===== Statistics =====
    
    // Get OCR statistics
    struct OCRStatistics {
        int total_pages;
        int successful_pages;
        int failed_pages;
        float average_confidence;
        int total_words;
        int low_confidence_words;
        std::map<std::string, int> detected_languages;
        double processing_time_seconds;
    };
    
    static OCRStatistics get_statistics(const std::vector<Result<OCRResult>>& results);
    
    // ===== Advanced Options =====
    
    // Set Tesseract variable
    static bool set_tesseract_variable(const std::string& name, const std::string& value);
    
    // Get Tesseract variable
    static std::string get_tesseract_variable(const std::string& name);
    
    // Load custom training data
    static bool load_training_data(const std::string& path);
    
    // Set whitelist characters
    static bool set_char_whitelist(const std::string& chars);
    
    // Set blacklist characters
    static bool set_char_blacklist(const std::string& chars);
};

} // namespace pdfeditor
