#pragma once

#include "core.h"
#include "document.h"
#include <string>
#include <vector>

namespace pdfeditor {

// Font information
struct FontInfo {
    std::string name;
    std::string family;
    float size;
    bool bold;
    bool italic;
    bool embedded;
    std::string encoding;
};

// Text properties
struct TextProperties {
    FontInfo font;
    Color color;
    float line_height;
    float char_spacing;
    float word_spacing;
    
    enum class Alignment {
        Left,
        Center,
        Right,
        Justify
    };
    Alignment alignment;
    
    TextProperties() 
        : line_height(1.2f)
        , char_spacing(0.0f)
        , word_spacing(0.0f)
        , alignment(Alignment::Left) {}
};

// Content stream operation
struct ContentOperation {
    enum class Type {
        DrawText,
        DrawImage,
        DrawPath,
        SetColor,
        SetFont,
        Transform
    };
    
    Type type;
    std::vector<float> parameters;
    std::string data;
};

// Editor class for modifying PDF content
class PDFEDITOR_API Editor {
public:
    // ===== Text Editing =====
    
    // Insert text at position
    static bool insert_text(
        Page* page,
        const std::string& text,
        const Point& position,
        const TextProperties& props = TextProperties()
    );
    
    // Replace text in area
    static bool replace_text(
        Page* page,
        const Rect& area,
        const std::string& new_text,
        const TextProperties& props = TextProperties()
    );
    
    // Delete text in area
    static bool delete_text(Page* page, const Rect& area);
    
    // Move text block
    static bool move_text(Page* page, const Rect& from, const Point& to);
    
    // Resize text (scale font)
    static bool resize_text(Page* page, const Rect& area, float scale);
    
    // Change text color
    static bool set_text_color(Page* page, const Rect& area, const Color& color);
    
    // Change font
    static bool set_text_font(
        Page* page,
        const Rect& area,
        const std::string& font_name,
        float font_size
    );
    
    // ===== Image Editing =====
    
    // Insert image from file
    static bool insert_image(
        Page* page,
        const std::string& image_path,
        const Rect& rect
    );
    
    // Insert image from memory
    static bool insert_image_data(
        Page* page,
        const uint8_t* data,
        size_t size,
        const Rect& rect
    );
    
    // Replace image in area
    static bool replace_image(
        Page* page,
        const Rect& area,
        const std::string& new_image_path
    );
    
    // Delete image in area
    static bool delete_image(Page* page, const Rect& area);
    
    // Move image
    static bool move_image(Page* page, const Rect& from, const Point& to);
    
    // Resize image
    static bool resize_image(Page* page, const Rect& from, const Rect& to);
    
    // Rotate image
    static bool rotate_image(Page* page, const Rect& area, float degrees);
    
    // Crop image
    static bool crop_image(Page* page, const Rect& image_rect, const Rect& crop_rect);
    
    // ===== Shape Drawing =====
    
    // Draw line
    static bool draw_line(
        Page* page,
        const Point& start,
        const Point& end,
        const Color& color,
        float width = 1.0f
    );
    
    // Draw rectangle
    static bool draw_rectangle(
        Page* page,
        const Rect& rect,
        const Color& stroke_color,
        const Color& fill_color,
        float stroke_width = 1.0f,
        bool filled = false
    );
    
    // Draw circle
    static bool draw_circle(
        Page* page,
        const Point& center,
        float radius,
        const Color& stroke_color,
        const Color& fill_color,
        float stroke_width = 1.0f,
        bool filled = false
    );
    
    // Draw polygon
    static bool draw_polygon(
        Page* page,
        const std::vector<Point>& points,
        const Color& stroke_color,
        const Color& fill_color,
        float stroke_width = 1.0f,
        bool filled = false,
        bool closed = true
    );
    
    // Draw bezier curve
    static bool draw_curve(
        Page* page,
        const std::vector<Point>& control_points,
        const Color& color,
        float width = 1.0f
    );
    
    // ===== Content Manipulation =====
    
    // Get all content operations on page
    static std::vector<ContentOperation> get_content_stream(Page* page);
    
    // Set content operations
    static bool set_content_stream(
        Page* page,
        const std::vector<ContentOperation>& operations
    );
    
    // Clear all content
    static bool clear_page(Page* page);
    
    // Crop page content to area
    static bool crop_content(Page* page, const Rect& crop_rect);
    
    // Scale page content
    static bool scale_content(Page* page, float scale_x, float scale_y);
    
    // Rotate page content
    static bool rotate_content(Page* page, float degrees);
    
    // Translate page content
    static bool translate_content(Page* page, float dx, float dy);
    
    // ===== Content Analysis =====
    
    // Detect text blocks
    static std::vector<Rect> detect_text_blocks(Page* page);
    
    // Detect images
    static std::vector<Rect> detect_images(Page* page);
    
    // Detect white space
    static std::vector<Rect> detect_whitespace(Page* page);
    
    // Get content bounds (actual used area)
    static Rect get_content_bounds(Page* page);
    
    // ===== Font Management =====
    
    // List fonts used in document
    static std::vector<FontInfo> list_fonts(Document* doc);
    
    // List fonts on page
    static std::vector<FontInfo> list_page_fonts(Page* page);
    
    // Embed font
    static bool embed_font(Document* doc, const std::string& font_path);
    
    // Subset font (remove unused glyphs)
    static bool subset_font(Document* doc, const std::string& font_name);
    
    // Replace font
    static bool replace_font(
        Document* doc,
        const std::string& old_font,
        const std::string& new_font
    );
    
    // ===== Advanced Editing =====
    
    // Reflow text (adjust to new boundaries)
    static bool reflow_text(
        Page* page,
        const Rect& area,
        const Rect& new_bounds
    );
    
    // Merge text blocks
    static bool merge_text_blocks(
        Page* page,
        const std::vector<Rect>& blocks
    );
    
    // Split text block
    static std::vector<Rect> split_text_block(
        Page* page,
        const Rect& block,
        int max_lines
    );
    
    // Auto-layout content
    static bool auto_layout(
        Page* page,
        float margin_left,
        float margin_right,
        float margin_top,
        float margin_bottom
    );
    
    // ===== Watermarking =====
    
    // Add text watermark
    static bool add_text_watermark(
        Page* page,
        const std::string& text,
        const Point& position,
        float opacity = 0.3f,
        float rotation = 45.0f
    );
    
    // Add image watermark
    static bool add_image_watermark(
        Page* page,
        const std::string& image_path,
        const Point& position,
        float opacity = 0.3f,
        float scale = 1.0f
    );
    
    // Remove watermarks
    static bool remove_watermarks(Page* page);
    
    // ===== Stamping =====
    
    // Add stamp (Approved, Draft, etc.)
    static bool add_stamp(
        Page* page,
        const std::string& stamp_text,
        const Point& position,
        const Color& color = Color::red()
    );
    
    // ===== Background & Foreground =====
    
    // Set page background color
    static bool set_background_color(Page* page, const Color& color);
    
    // Add background image
    static bool set_background_image(
        Page* page,
        const std::string& image_path,
        bool scale_to_fit = true
    );
    
    // Add overlay content
    static bool add_overlay(
        Page* page,
        const std::string& overlay_pdf_path,
        int overlay_page_index = 0
    );
};

} // namespace pdfeditor
