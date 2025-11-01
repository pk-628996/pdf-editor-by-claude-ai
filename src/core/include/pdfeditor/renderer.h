#pragma once

#include "core.h"
#include "document.h"
#include <memory>
#include <vector>

namespace pdfeditor {

// Render quality/DPI presets
enum class RenderQuality {
    Draft = 72,        // Screen preview
    Low = 96,          // Low quality
    Medium = 150,      // Standard quality
    High = 300,        // Print quality
    VeryHigh = 600     // High-end printing
};

// Anti-aliasing options
enum class AntiAliasing {
    None,
    Text,
    Graphics,
    All
};

// Color rendering mode
enum class ColorMode {
    RGB,
    CMYK,
    Grayscale,
    Monochrome
};

// Image format for rendering
enum class ImageFormat {
    RGB24,      // 24-bit RGB
    RGBA32,     // 32-bit RGBA
    BGR24,      // 24-bit BGR (Windows bitmap order)
    BGRA32,     // 32-bit BGRA
    Gray8,      // 8-bit grayscale
    Mono1       // 1-bit monochrome
};

// Render options
struct RenderOptions {
    float dpi = 150.0f;
    AntiAliasing anti_aliasing = AntiAliasing::All;
    ColorMode color_mode = ColorMode::RGB;
    ImageFormat image_format = ImageFormat::RGB24;
    bool render_annotations = true;
    bool render_forms = true;
    bool render_xfa_forms = false;
    bool render_transparent = false;
    Color background_color = Color::white();
    
    // Clipping rectangle (in page coordinates)
    Rect clip_rect;
    bool use_clip_rect = false;
    
    // Rotation override (if different from page rotation)
    PageRotation rotation = PageRotation::None;
    bool override_rotation = false;
};

// Rendered image buffer
class PDFEDITOR_API ImageBuffer {
public:
    ImageBuffer();
    ~ImageBuffer();
    
    // Dimensions
    int width() const;
    int height() const;
    int stride() const;  // Bytes per row
    
    // Image format
    ImageFormat format() const;
    int bytes_per_pixel() const;
    
    // Raw pixel data
    const uint8_t* data() const;
    uint8_t* data();
    size_t size() const;
    
    // Save to file
    bool save_png(const std::string& path) const;
    bool save_jpeg(const std::string& path, int quality = 85) const;
    bool save_bmp(const std::string& path) const;
    bool save_tiff(const std::string& path) const;
    
    // Copy to buffer
    std::vector<uint8_t> to_vector() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// PDF Renderer
class PDFEDITOR_API Renderer {
public:
    Renderer();
    ~Renderer();
    
    // ===== Single Page Rendering =====
    
    // Render page to image buffer
    Result<std::unique_ptr<ImageBuffer>> render_page(
        Page* page,
        const RenderOptions& options = RenderOptions()
    );
    
    // Render page to existing buffer (must be pre-allocated)
    bool render_page_to_buffer(
        Page* page,
        uint8_t* buffer,
        size_t buffer_size,
        const RenderOptions& options = RenderOptions()
    );
    
    // Render page at specific scale
    Result<std::unique_ptr<ImageBuffer>> render_page_scaled(
        Page* page,
        float scale_x,
        float scale_y,
        const RenderOptions& options = RenderOptions()
    );
    
    // Render page to specific dimensions
    Result<std::unique_ptr<ImageBuffer>> render_page_to_size(
        Page* page,
        int width,
        int height,
        const RenderOptions& options = RenderOptions()
    );
    
    // ===== Batch Rendering =====
    
    // Render multiple pages
    std::vector<Result<std::unique_ptr<ImageBuffer>>> render_pages(
        Document* doc,
        const std::vector<int>& page_indices,
        const RenderOptions& options = RenderOptions(),
        ProgressCallback callback = nullptr
    );
    
    // Render all pages
    std::vector<Result<std::unique_ptr<ImageBuffer>>> render_all_pages(
        Document* doc,
        const RenderOptions& options = RenderOptions(),
        ProgressCallback callback = nullptr
    );
    
    // ===== Thumbnail Generation =====
    
    // Generate thumbnail of page
    Result<std::unique_ptr<ImageBuffer>> render_thumbnail(
        Page* page,
        int max_width,
        int max_height,
        bool maintain_aspect = true
    );
    
    // Generate thumbnails for all pages
    std::vector<Result<std::unique_ptr<ImageBuffer>>> render_all_thumbnails(
        Document* doc,
        int max_width,
        int max_height,
        ProgressCallback callback = nullptr
    );
    
    // ===== Tile Rendering (for large pages) =====
    
    struct TileInfo {
        int tile_x;
        int tile_y;
        int tile_width;
        int tile_height;
        Rect page_rect;  // Area of page this tile covers
    };
    
    // Calculate tile layout for page
    std::vector<TileInfo> calculate_tiles(
        Page* page,
        int tile_width,
        int tile_height,
        const RenderOptions& options = RenderOptions()
    );
    
    // Render specific tile
    Result<std::unique_ptr<ImageBuffer>> render_tile(
        Page* page,
        const TileInfo& tile,
        const RenderOptions& options = RenderOptions()
    );
    
    // ===== Progressive Rendering =====
    
    // Start progressive render (for large pages)
    bool start_progressive_render(
        Page* page,
        const RenderOptions& options = RenderOptions()
    );
    
    // Continue progressive render (returns true if more work needed)
    bool continue_progressive_render();
    
    // Get current progressive render state
    Result<std::unique_ptr<ImageBuffer>> get_progressive_buffer();
    
    // Cancel progressive render
    void cancel_progressive_render();
    
    // ===== Caching =====
    
    // Enable/disable render cache
    void set_cache_enabled(bool enabled);
    bool is_cache_enabled() const;
    
    // Set cache size (in MB)
    void set_cache_size(size_t size_mb);
    size_t get_cache_size() const;
    
    // Clear render cache
    void clear_cache();
    
    // Invalidate cache for specific page
    void invalidate_page(Page* page);
    
    // ===== Utility Functions =====
    
    // Calculate rendered dimensions for page at DPI
    static void calculate_dimensions(
        Page* page,
        float dpi,
        int& width,
        int& height
    );
    
    // Calculate scale to fit within dimensions
    static float calculate_scale_to_fit(
        Page* page,
        int max_width,
        int max_height
    );
    
    // Convert page coordinates to pixel coordinates
    static Point page_to_pixel(
        const Point& page_point,
        float dpi,
        float page_height
    );
    
    // Convert pixel coordinates to page coordinates
    static Point pixel_to_page(
        const Point& pixel_point,
        float dpi,
        float page_height
    );
    
    // ===== Performance Settings =====
    
    // Set number of threads for rendering (0 = auto)
    void set_thread_count(int count);
    int get_thread_count() const;
    
    // Enable/disable GPU acceleration (if available)
    void set_gpu_acceleration(bool enabled);
    bool is_gpu_acceleration_enabled() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Render job (for async rendering)
class PDFEDITOR_API RenderJob {
public:
    RenderJob();
    ~RenderJob();
    
    // Job status
    enum class Status {
        Pending,
        Running,
        Completed,
        Failed,
        Cancelled
    };
    
    Status get_status() const;
    float get_progress() const;  // 0.0 to 1.0
    
    // Wait for completion
    bool wait(int timeout_ms = -1);
    
    // Cancel job
    void cancel();
    
    // Get result (blocks until complete)
    Result<std::unique_ptr<ImageBuffer>> get_result();
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Async renderer (for background rendering)
class PDFEDITOR_API AsyncRenderer {
public:
    AsyncRenderer();
    ~AsyncRenderer();
    
    // Queue render job
    std::shared_ptr<RenderJob> queue_render(
        Page* page,
        const RenderOptions& options = RenderOptions()
    );
    
    // Queue multiple pages
    std::vector<std::shared_ptr<RenderJob>> queue_batch(
        Document* doc,
        const std::vector<int>& page_indices,
        const RenderOptions& options = RenderOptions()
    );
    
    // Get number of pending jobs
    int pending_count() const;
    
    // Cancel all jobs
    void cancel_all();
    
    // Wait for all jobs to complete
    void wait_all();
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace pdfeditor
