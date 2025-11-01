#pragma once

#include "core.h"
#include "document.h"

namespace pdfeditor {

// Optimization profile
enum class OptimizationProfile {
    Web,           // Optimize for web viewing (fast loading)
    Print,         // Optimize for printing (high quality)
    Screen,        // Optimize for screen viewing (balanced)
    Minimal,       // Minimize file size aggressively
    Archive,       // Optimize for long-term storage
    Custom         // Custom optimization settings
};

// Image compression quality
enum class ImageQuality {
    Maximum = 100,
    High = 85,
    Medium = 70,
    Low = 50,
    VeryLow = 30
};

// Optimization options
struct OptimizationOptions {
    // Image optimization
    bool compress_images;
    ImageQuality image_quality;
    int downsample_dpi;            // 0 = no downsampling
    bool convert_to_jpeg;          // Convert non-JPEG images
    bool remove_duplicate_images;
    
    // Content stream optimization
    bool compress_streams;
    bool remove_unused_objects;
    bool merge_duplicate_resources;
    bool optimize_content_streams;
    
    // Font optimization
    bool subset_fonts;
    bool remove_unused_fonts;
    bool embed_base14_fonts;       // Embed standard fonts
    
    // Structural optimization
    bool linearize;                // Fast web view
    bool remove_invalid_bookmarks;
    bool remove_invalid_links;
    bool flatten_form_fields;
    bool flatten_annotations;
    
    // Metadata optimization
    bool remove_metadata;
    bool remove_thumbnails;
    bool remove_embedded_files;
    bool remove_javascript;
    
    // Advanced
    bool discard_private_data;
    bool clean_document;
    CompressionLevel compression_level;
    
    OptimizationOptions()
        : compress_images(true)
        , image_quality(ImageQuality::High)
        , downsample_dpi(150)
        , convert_to_jpeg(false)
        , remove_duplicate_images(true)
        , compress_streams(true)
        , remove_unused_objects(true)
        , merge_duplicate_resources(true)
        , optimize_content_streams(true)
        , subset_fonts(true)
        , remove_unused_fonts(true)
        , embed_base14_fonts(false)
        , linearize(true)
        , remove_invalid_bookmarks(true)
        , remove_invalid_links(true)
        , flatten_form_fields(false)
        , flatten_annotations(false)
        , remove_metadata(false)
        , remove_thumbnails(true)
        , remove_embedded_files(false)
        , remove_javascript(false)
        , discard_private_data(false)
        , clean_document(true)
        , compression_level(CompressionLevel::Default) {}
    
    // Create options from profile
    static OptimizationOptions from_profile(OptimizationProfile profile);
};

// Optimization result
struct OptimizationResult {
    bool success;
    size_t original_size;
    size_t optimized_size;
    size_t size_reduction;
    float reduction_percentage;
    double processing_time_seconds;
    
    struct Details {
        size_t images_compressed;
        size_t images_removed;
        size_t fonts_subset;
        size_t fonts_removed;
        size_t objects_removed;
        size_t streams_compressed;
        bool linearized;
    } details;
    
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

// Optimizer class
class PDFEDITOR_API Optimizer {
public:
    // ===== Main Optimization =====
    
    // Optimize document with options
    static OptimizationResult optimize(
        Document* doc,
        const OptimizationOptions& options = OptimizationOptions()
    );
    
    // Optimize with preset profile
    static OptimizationResult optimize_with_profile(
        Document* doc,
        OptimizationProfile profile
    );
    
    // Optimize to target size (best effort)
    static OptimizationResult optimize_to_size(
        Document* doc,
        size_t target_size_bytes,
        ProgressCallback callback = nullptr
    );
    
    // ===== Image Optimization =====
    
    // Compress all images
    static bool compress_images(
        Document* doc,
        ImageQuality quality = ImageQuality::High
    );
    
    // Downsample images
    static bool downsample_images(Document* doc, int target_dpi);
    
    // Convert images to JPEG
    static bool convert_images_to_jpeg(
        Document* doc,
        ImageQuality quality = ImageQuality::High
    );
    
    // Remove duplicate images
    static int remove_duplicate_images(Document* doc);
    
    // Get image statistics
    struct ImageStatistics {
        int total_images;
        int duplicate_images;
        size_t total_image_size;
        int max_dpi;
        int min_dpi;
        float avg_dpi;
        std::map<std::string, int> formats;  // format -> count
    };
    
    static ImageStatistics analyze_images(Document* doc);
    
    // ===== Font Optimization =====
    
    // Subset all fonts
    static bool subset_fonts(Document* doc);
    
    // Remove unused fonts
    static int remove_unused_fonts(Document* doc);
    
    // Embed fonts
    static bool embed_fonts(Document* doc);
    
    // Unembed fonts (use system fonts)
    static bool unembed_fonts(Document* doc);
    
    // Get font statistics
    struct FontStatistics {
        int total_fonts;
        int embedded_fonts;
        int subset_fonts;
        size_t total_font_size;
        std::vector<std::string> font_names;
    };
    
    static FontStatistics analyze_fonts(Document* doc);
    
    // ===== Content Stream Optimization =====
    
    // Compress all content streams
    static bool compress_streams(Document* doc);
    
    // Optimize content streams (remove redundant operators)
    static bool optimize_content_streams(Document* doc);
    
    // Merge duplicate resources
    static int merge_duplicate_resources(Document* doc);
    
    // ===== Object Optimization =====
    
    // Remove unused objects
    static int remove_unused_objects(Document* doc);
    
    // Remove orphaned objects
    static int remove_orphaned_objects(Document* doc);
    
    // Compact object numbers (renumber sequentially)
    static bool compact_objects(Document* doc);
    
    // Get object statistics
    struct ObjectStatistics {
        int total_objects;
        int unused_objects;
        int orphaned_objects;
        int compressed_objects;
        size_t total_size;
    };
    
    static ObjectStatistics analyze_objects(Document* doc);
    
    // ===== Linearization =====
    
    // Linearize document (Fast Web View)
    static bool linearize(Document* doc);
    
    // Check if document is linearized
    static bool is_linearized(Document* doc);
    
    // Remove linearization
    static bool delinearize(Document* doc);
    
    // ===== Structure Optimization =====
    
    // Remove invalid bookmarks
    static int remove_invalid_bookmarks(Document* doc);
    
    // Remove invalid links
    static int remove_invalid_links(Document* doc);
    
    // Remove invalid annotations
    static int remove_invalid_annotations(Document* doc);
    
    // Fix page tree
    static bool fix_page_tree(Document* doc);
    
    // ===== Metadata Optimization =====
    
    // Remove page thumbnails
    static int remove_thumbnails(Document* doc);
    
    // Remove embedded files
    static int remove_embedded_files(Document* doc);
    
    // Remove JavaScript
    static int remove_javascript(Document* doc);
    
    // Remove private data
    static bool remove_private_data(Document* doc);
    
    // ===== Document Cleanup =====
    
    // Clean up document structure
    static bool clean_document(Document* doc);
    
    // Repair document
    static bool repair_document(Document* doc);
    
    // Validate and fix
    static bool validate_and_fix(Document* doc);
    
    // ===== Analysis =====
    
    // Analyze document for optimization opportunities
    struct OptimizationAnalysis {
        size_t current_size;
        size_t estimated_optimized_size;
        float estimated_reduction_percentage;
        
        struct Recommendations {
            bool should_compress_images;
            bool should_downsample_images;
            bool should_subset_fonts;
            bool should_remove_unused_objects;
            bool should_linearize;
            bool should_compress_streams;
            std::vector<std::string> details;
        } recommendations;
        
        ImageStatistics image_stats;
        FontStatistics font_stats;
        ObjectStatistics object_stats;
    };
    
    static OptimizationAnalysis analyze(Document* doc);
    
    // Estimate optimization result
    static size_t estimate_optimized_size(
        Document* doc,
        const OptimizationOptions& options
    );
    
    // ===== Batch Optimization =====
    
    // Batch optimize multiple files
    struct BatchOptimizationJob {
        std::string input_path;
        std::string output_path;
        OptimizationOptions options;
    };
    
    static std::vector<OptimizationResult> batch_optimize(
        const std::vector<BatchOptimizationJob>& jobs,
        ProgressCallback callback = nullptr
    );
    
    // ===== Comparison =====
    
    // Compare two documents
    struct ComparisonResult {
        size_t size_difference;
        float size_reduction_percentage;
        int page_count_difference;
        int object_count_difference;
        int image_count_difference;
        int font_count_difference;
        bool both_linearized;
    };
    
    static ComparisonResult compare_documents(
        Document* original,
        Document* optimized
    );
    
    // ===== Presets =====
    
    // Get Web preset options
    static OptimizationOptions get_web_preset();
    
    // Get Print preset options
    static OptimizationOptions get_print_preset();
    
    // Get Minimal preset options
    static OptimizationOptions get_minimal_preset();
    
    // Get Archive preset options
    static OptimizationOptions get_archive_preset();
};

} // namespace pdfeditor
