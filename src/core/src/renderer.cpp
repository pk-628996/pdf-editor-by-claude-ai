#include "pdfeditor/renderer.h"
#include "pdfeditor/core.h"
#include <cmath>
#include <algorithm>
#include <thread>
#include <mutex>
#include <map>

#ifdef USE_MUPDF
#include <mupdf/fitz.h>
#endif

namespace pdfeditor {

// ImageBuffer implementation
class ImageBuffer::Impl {
public:
    std::vector<uint8_t> data;
    int width = 0;
    int height = 0;
    int stride = 0;
    ImageFormat format = ImageFormat::RGB24;
};

ImageBuffer::ImageBuffer() : impl_(std::make_unique<Impl>()) {}
ImageBuffer::~ImageBuffer() = default;

int ImageBuffer::width() const { return impl_->width; }
int ImageBuffer::height() const { return impl_->height; }
int ImageBuffer::stride() const { return impl_->stride; }

ImageFormat ImageBuffer::format() const { return impl_->format; }

int ImageBuffer::bytes_per_pixel() const {
    switch (impl_->format) {
        case ImageFormat::RGB24:
        case ImageFormat::BGR24:
            return 3;
        case ImageFormat::RGBA32:
        case ImageFormat::BGRA32:
            return 4;
        case ImageFormat::Gray8:
            return 1;
        case ImageFormat::Mono1:
            return 1; // Note: 1 bit per pixel, but byte-aligned
        default:
            return 0;
    }
}

const uint8_t* ImageBuffer::data() const {
    return impl_->data.data();
}

uint8_t* ImageBuffer::data() {
    return impl_->data.data();
}

size_t ImageBuffer::size() const {
    return impl_->data.size();
}

bool ImageBuffer::save_png(const std::string& path) const {
    // TODO: Implement PNG saving using stb_image_write or libpng
    return false;
}

bool ImageBuffer::save_jpeg(const std::string& path, int quality) const {
    // TODO: Implement JPEG saving
    return false;
}

bool ImageBuffer::save_bmp(const std::string& path) const {
    // TODO: Implement BMP saving
    return false;
}

bool ImageBuffer::save_tiff(const std::string& path) const {
    // TODO: Implement TIFF saving
    return false;
}

std::vector<uint8_t> ImageBuffer::to_vector() const {
    return impl_->data;
}

// Renderer implementation
class Renderer::Impl {
public:
    Impl() : cache_enabled_(true), cache_size_mb_(100), thread_count_(0) {}
    
    bool cache_enabled_;
    size_t cache_size_mb_;
    int thread_count_;
    std::mutex cache_mutex_;
    std::map<void*, std::unique_ptr<ImageBuffer>> cache_;
    
#ifdef USE_MUPDF
    fz_context* get_context() {
        // Thread-local context for thread safety
        static thread_local fz_context* ctx = nullptr;
        if (!ctx) {
            ctx = fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT);
            if (ctx) {
                fz_register_document_handlers(ctx);
            }
        }
        return ctx;
    }
#endif
};

Renderer::Renderer() : impl_(std::make_unique<Impl>()) {}
Renderer::~Renderer() = default;

Result<std::unique_ptr<ImageBuffer>> Renderer::render_page(
    Page* page,
    const RenderOptions& options
) {
    if (!page) {
        return Result<std::unique_ptr<ImageBuffer>>(
            ErrorCode::InvalidArgument,
            "Invalid page"
        );
    }
    
#ifdef USE_MUPDF
    fz_context* ctx = impl_->get_context();
    if (!ctx) {
        return Result<std::unique_ptr<ImageBuffer>>(
            ErrorCode::RenderError,
            "Failed to get rendering context"
        );
    }
    
    auto buffer = std::make_unique<ImageBuffer>();
    
    fz_try(ctx) {
        fz_page* fz_pg = static_cast<fz_page*>(page->get_handle());
        if (!fz_pg) {
            fz_throw(ctx, FZ_ERROR_GENERIC, "Invalid page handle");
        }
        
        // Calculate transform matrix
        fz_matrix transform = fz_scale(options.dpi / 72.0f, options.dpi / 72.0f);
        
        // Get page bounds
        fz_rect bounds = fz_bound_page(ctx, fz_pg);
        bounds = fz_transform_rect(bounds, transform);
        
        // Calculate pixel dimensions
        int width = static_cast<int>(bounds.x1 - bounds.x0);
        int height = static_cast<int>(bounds.y1 - bounds.y0);
        
        // Create pixmap
        fz_colorspace* colorspace = nullptr;
        int alpha = 0;
        
        switch (options.color_mode) {
            case ColorMode::RGB:
                colorspace = fz_device_rgb(ctx);
                break;
            case ColorMode::Grayscale:
                colorspace = fz_device_gray(ctx);
                break;
            case ColorMode::CMYK:
                colorspace = fz_device_cmyk(ctx);
                break;
            default:
                colorspace = fz_device_rgb(ctx);
        }
        
        if (options.render_transparent) {
            alpha = 1;
        }
        
        fz_pixmap* pix = fz_new_pixmap_with_bbox(
            ctx,
            colorspace,
            fz_round_rect(bounds),
            nullptr,
            alpha
        );
        
        // Clear background
        if (!options.render_transparent) {
            fz_clear_pixmap_with_value(
                ctx,
                pix,
                static_cast<int>(options.background_color.r * 255)
            );
        } else {
            fz_clear_pixmap(ctx, pix);
        }
        
        // Create device and render
        fz_device* dev = fz_new_draw_device(ctx, transform, pix);
        fz_run_page(ctx, fz_pg, dev, fz_identity, nullptr);
        fz_close_device(ctx, dev);
        fz_drop_device(ctx, dev);
        
        // Copy pixmap data to ImageBuffer
        buffer->impl_->width = fz_pixmap_width(ctx, pix);
        buffer->impl_->height = fz_pixmap_height(ctx, pix);
        buffer->impl_->stride = fz_pixmap_stride(ctx, pix);
        
        // Determine format
        int n = fz_pixmap_components(ctx, pix);
        if (alpha) {
            buffer->impl_->format = (n == 4) ? ImageFormat::RGBA32 : ImageFormat::BGRA32;
        } else {
            buffer->impl_->format = (n == 3) ? ImageFormat::RGB24 : ImageFormat::BGR24;
        }
        
        // Copy pixel data
        size_t data_size = buffer->impl_->stride * buffer->impl_->height;
        buffer->impl_->data.resize(data_size);
        std::memcpy(
            buffer->impl_->data.data(),
            fz_pixmap_samples(ctx, pix),
            data_size
        );
        
        fz_drop_pixmap(ctx, pix);
    }
    fz_catch(ctx) {
        return Result<std::unique_ptr<ImageBuffer>>(
            ErrorCode::RenderError,
            fz_caught_message(ctx)
        );
    }
    
    return Result<std::unique_ptr<ImageBuffer>>(std::move(buffer));
#else
    return Result<std::unique_ptr<ImageBuffer>>(
        ErrorCode::NotImplemented,
        "Rendering not implemented for this backend"
    );
#endif
}

bool Renderer::render_page_to_buffer(
    Page* page,
    uint8_t* buffer,
    size_t buffer_size,
    const RenderOptions& options
) {
    auto result = render_page(page, options);
    if (!result.is_ok()) {
        return false;
    }
    
    auto img = result.value();
    size_t img_size = img->size();
    
    if (img_size > buffer_size) {
        return false;
    }
    
    std::memcpy(buffer, img->data(), img_size);
    return true;
}

Result<std::unique_ptr<ImageBuffer>> Renderer::render_page_scaled(
    Page* page,
    float scale_x,
    float scale_y,
    const RenderOptions& options
) {
    // Calculate DPI based on scale
    RenderOptions scaled_options = options;
    scaled_options.dpi *= (scale_x + scale_y) / 2.0f; // Average scale
    
    return render_page(page, scaled_options);
}

Result<std::unique_ptr<ImageBuffer>> Renderer::render_page_to_size(
    Page* page,
    int width,
    int height,
    const RenderOptions& options
) {
    if (!page) {
        return Result<std::unique_ptr<ImageBuffer>>(
            ErrorCode::InvalidArgument,
            "Invalid page"
        );
    }
    
    // Calculate required DPI
    float page_width = page->width();
    float page_height = page->height();
    
    float scale_x = width / page_width;
    float scale_y = height / page_height;
    float scale = std::min(scale_x, scale_y);
    
    RenderOptions sized_options = options;
    sized_options.dpi = 72.0f * scale;
    
    return render_page(page, sized_options);
}

std::vector<Result<std::unique_ptr<ImageBuffer>>> Renderer::render_pages(
    Document* doc,
    const std::vector<int>& page_indices,
    const RenderOptions& options,
    ProgressCallback callback
) {
    std::vector<Result<std::unique_ptr<ImageBuffer>>> results;
    
    for (size_t i = 0; i < page_indices.size(); ++i) {
        if (callback) {
            bool should_continue = callback(
                static_cast<int>(i),
                static_cast<int>(page_indices.size()),
                "Rendering page " + std::to_string(page_indices[i])
            );
            
            if (!should_continue) {
                break;
            }
        }
        
        Page* page = doc->get_page(page_indices[i]);
        if (page) {
            results.push_back(render_page(page, options));
        } else {
            results.push_back(Result<std::unique_ptr<ImageBuffer>>(
                ErrorCode::InvalidArgument,
                "Invalid page index"
            ));
        }
    }
    
    return results;
}

std::vector<Result<std::unique_ptr<ImageBuffer>>> Renderer::render_all_pages(
    Document* doc,
    const RenderOptions& options,
    ProgressCallback callback
) {
    if (!doc) return {};
    
    std::vector<int> indices;
    for (int i = 0; i < doc->page_count(); ++i) {
        indices.push_back(i);
    }
    
    return render_pages(doc, indices, options, callback);
}

Result<std::unique_ptr<ImageBuffer>> Renderer::render_thumbnail(
    Page* page,
    int max_width,
    int max_height,
    bool maintain_aspect
) {
    if (!page) {
        return Result<std::unique_ptr<ImageBuffer>>(
            ErrorCode::InvalidArgument,
            "Invalid page"
        );
    }
    
    float page_width = page->width();
    float page_height = page->height();
    
    float scale_x = max_width / page_width;
    float scale_y = max_height / page_height;
    
    float scale = maintain_aspect ? std::min(scale_x, scale_y) : scale_x;
    
    RenderOptions options;
    options.dpi = 72.0f * scale;
    options.anti_aliasing = AntiAliasing::All;
    
    return render_page(page, options);
}

std::vector<Result<std::unique_ptr<ImageBuffer>>> Renderer::render_all_thumbnails(
    Document* doc,
    int max_width,
    int max_height,
    ProgressCallback callback
) {
    std::vector<Result<std::unique_ptr<ImageBuffer>>> results;
    
    if (!doc) return results;
    
    for (int i = 0; i < doc->page_count(); ++i) {
        if (callback) {
            bool should_continue = callback(
                i,
                doc->page_count(),
                "Generating thumbnail " + std::to_string(i + 1)
            );
            
            if (!should_continue) {
                break;
            }
        }
        
        Page* page = doc->get_page(i);
        if (page) {
            results.push_back(render_thumbnail(page, max_width, max_height));
        }
    }
    
    return results;
}

std::vector<Renderer::TileInfo> Renderer::calculate_tiles(
    Page* page,
    int tile_width,
    int tile_height,
    const RenderOptions& options
) {
    std::vector<TileInfo> tiles;
    
    if (!page) return tiles;
    
    float page_width = page->width();
    float page_height = page->height();
    
    int cols = static_cast<int>(std::ceil(page_width / tile_width));
    int rows = static_cast<int>(std::ceil(page_height / tile_height));
    
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            TileInfo tile;
            tile.tile_x = col;
            tile.tile_y = row;
            tile.tile_width = tile_width;
            tile.tile_height = tile_height;
            
            tile.page_rect.x0 = col * tile_width;
            tile.page_rect.y0 = row * tile_height;
            tile.page_rect.x1 = std::min((col + 1) * tile_width, 
                                         static_cast<int>(page_width));
            tile.page_rect.y1 = std::min((row + 1) * tile_height,
                                         static_cast<int>(page_height));
            
            tiles.push_back(tile);
        }
    }
    
    return tiles;
}

Result<std::unique_ptr<ImageBuffer>> Renderer::render_tile(
    Page* page,
    const TileInfo& tile,
    const RenderOptions& options
) {
    RenderOptions tile_options = options;
    tile_options.use_clip_rect = true;
    tile_options.clip_rect = tile.page_rect;
    
    return render_page(page, tile_options);
}

bool Renderer::start_progressive_render(
    Page* page,
    const RenderOptions& options
) {
    // TODO: Implement progressive rendering
    return false;
}

bool Renderer::continue_progressive_render() {
    // TODO: Continue progressive render
    return false;
}

Result<std::unique_ptr<ImageBuffer>> Renderer::get_progressive_buffer() {
    // TODO: Get progressive buffer
    return Result<std::unique_ptr<ImageBuffer>>(ErrorCode::NotImplemented);
}

void Renderer::cancel_progressive_render() {
    // TODO: Cancel progressive render
}

void Renderer::set_cache_enabled(bool enabled) {
    impl_->cache_enabled_ = enabled;
}

bool Renderer::is_cache_enabled() const {
    return impl_->cache_enabled_;
}

void Renderer::set_cache_size(size_t size_mb) {
    impl_->cache_size_mb_ = size_mb;
}

size_t Renderer::get_cache_size() const {
    return impl_->cache_size_mb_;
}

void Renderer::clear_cache() {
    std::lock_guard<std::mutex> lock(impl_->cache_mutex_);
    impl_->cache_.clear();
}

void Renderer::invalidate_page(Page* page) {
    std::lock_guard<std::mutex> lock(impl_->cache_mutex_);
    impl_->cache_.erase(page->get_handle());
}

void Renderer::calculate_dimensions(
    Page* page,
    float dpi,
    int& width,
    int& height
) {
    if (!page) {
        width = height = 0;
        return;
    }
    
    float scale = dpi / 72.0f;
    width = static_cast<int>(page->width() * scale);
    height = static_cast<int>(page->height() * scale);
}

float Renderer::calculate_scale_to_fit(
    Page* page,
    int max_width,
    int max_height
) {
    if (!page) return 1.0f;
    
    float page_width = page->width();
    float page_height = page->height();
    
    float scale_x = max_width / page_width;
    float scale_y = max_height / page_height;
    
    return std::min(scale_x, scale_y);
}

Point Renderer::page_to_pixel(
    const Point& page_point,
    float dpi,
    float page_height
) {
    float scale = dpi / 72.0f;
    return Point(
        page_point.x * scale,
        (page_height - page_point.y) * scale  // Flip Y axis
    );
}

Point Renderer::pixel_to_page(
    const Point& pixel_point,
    float dpi,
    float page_height
) {
    float scale = 72.0f / dpi;
    return Point(
        pixel_point.x * scale,
        page_height - (pixel_point.y * scale)  // Flip Y axis
    );
}

void Renderer::set_thread_count(int count) {
    if (count == 0) {
        impl_->thread_count_ = std::thread::hardware_concurrency();
    } else {
        impl_->thread_count_ = count;
    }
}

int Renderer::get_thread_count() const {
    return impl_->thread_count_ == 0 ? 
           std::thread::hardware_concurrency() : 
           impl_->thread_count_;
}

void Renderer::set_gpu_acceleration(bool enabled) {
    // TODO: Implement GPU acceleration
}

bool Renderer::is_gpu_acceleration_enabled() const {
    // TODO: Check GPU acceleration status
    return false;
}

// RenderJob implementation
class RenderJob::Impl {
public:
    Status status = Status::Pending;
    float progress = 0.0f;
    std::unique_ptr<ImageBuffer> result;
    std::mutex mutex;
    std::condition_variable cv;
};

RenderJob::RenderJob() : impl_(std::make_unique<Impl>()) {}
RenderJob::~RenderJob() = default;

RenderJob::Status RenderJob::get_status() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->status;
}

float RenderJob::get_progress() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->progress;
}

bool RenderJob::wait(int timeout_ms) {
    std::unique_lock<std::mutex> lock(impl_->mutex);
    
    if (timeout_ms < 0) {
        impl_->cv.wait(lock, [this] {
            return impl_->status == Status::Completed ||
                   impl_->status == Status::Failed ||
                   impl_->status == Status::Cancelled;
        });
        return true;
    } else {
        return impl_->cv.wait_for(
            lock,
            std::chrono::milliseconds(timeout_ms),
            [this] {
                return impl_->status == Status::Completed ||
                       impl_->status == Status::Failed ||
                       impl_->status == Status::Cancelled;
            }
        );
    }
}

void RenderJob::cancel() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (impl_->status == Status::Pending || impl_->status == Status::Running) {
        impl_->status = Status::Cancelled;
        impl_->cv.notify_all();
    }
}

Result<std::unique_ptr<ImageBuffer>> RenderJob::get_result() {
    wait();
    
    std::lock_guard<std::mutex> lock(impl_->mutex);
    
    if (impl_->status == Status::Completed) {
        return Result<std::unique_ptr<ImageBuffer>>(std::move(impl_->result));
    } else {
        return Result<std::unique_ptr<ImageBuffer>>(
            ErrorCode::RenderError,
            "Render job did not complete successfully"
        );
    }
}

// AsyncRenderer implementation
class AsyncRenderer::Impl {
public:
    // TODO: Implement async rendering with thread pool
};

AsyncRenderer::AsyncRenderer() : impl_(std::make_unique<Impl>()) {}
AsyncRenderer::~AsyncRenderer() = default;

std::shared_ptr<RenderJob> AsyncRenderer::queue_render(
    Page* page,
    const RenderOptions& options
) {
    // TODO: Queue render job in thread pool
    auto job = std::make_shared<RenderJob>();
    return job;
}

std::vector<std::shared_ptr<RenderJob>> AsyncRenderer::queue_batch(
    Document* doc,
    const std::vector<int>& page_indices,
    const RenderOptions& options
) {
    std::vector<std::shared_ptr<RenderJob>> jobs;
    
    for (int idx : page_indices) {
        Page* page = doc->get_page(idx);
        if (page) {
            jobs.push_back(queue_render(page, options));
        }
    }
    
    return jobs;
}

int AsyncRenderer::pending_count() const {
    // TODO: Return number of pending jobs
    return 0;
}

void AsyncRenderer::cancel_all() {
    // TODO: Cancel all pending jobs
}

void AsyncRenderer::wait_all() {
    // TODO: Wait for all jobs to complete
}

} // namespace pdfeditor
