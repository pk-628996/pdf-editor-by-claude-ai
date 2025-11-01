#pragma once

#include "core.h"
#include "document.h"
#include <string>
#include <vector>
#include <memory>

namespace pdfeditor {

// Annotation types
enum class AnnotationType {
    Text,           // Sticky note
    FreeText,       // Free text box
    Line,           // Line
    Square,         // Rectangle
    Circle,         // Circle/Ellipse
    Polygon,        // Polygon
    PolyLine,       // Multi-segment line
    Highlight,      // Text highlight
    Underline,      // Text underline
    Squiggly,       // Squiggly underline
    StrikeOut,      // Strike through
    Stamp,          // Rubber stamp
    Caret,          // Caret (insertion point)
    Ink,            // Freehand drawing
    Popup,          // Popup window
    FileAttachment, // Attached file
    Sound,          // Sound annotation
    Movie,          // Movie annotation
    Widget,         // Form field
    Screen,         // Screen annotation
    PrinterMark,    // Printer's mark
    TrapNet,        // Trap network
    Watermark,      // Watermark
    Redact          // Redaction
};

// Annotation flags
enum class AnnotationFlag : uint32_t {
    None = 0,
    Invisible = 1 << 0,         // Not displayed
    Hidden = 1 << 1,             // Not displayed, not printed
    Print = 1 << 2,              // Print when page is printed
    NoZoom = 1 << 3,             // Don't scale with page
    NoRotate = 1 << 4,           // Don't rotate with page
    NoView = 1 << 5,             // Don't display
    ReadOnly = 1 << 6,           // Not editable
    Locked = 1 << 7,             // Not deletable
    ToggleNoView = 1 << 8,       // Toggle visibility
    LockedContents = 1 << 9      // Contents not editable
};

// Text markup style
enum class MarkupStyle {
    Highlight,
    Underline,
    Squiggly,
    StrikeOut
};

// Line ending styles
enum class LineEnding {
    None,
    Square,
    Circle,
    Diamond,
    OpenArrow,
    ClosedArrow,
    Butt,
    ROpenArrow,
    RClosedArrow,
    Slash
};

// Border style
struct BorderStyle {
    float width;
    std::vector<float> dash_pattern;
    
    BorderStyle() : width(1.0f) {}
};

// Base annotation
struct Annotation {
    std::string id;
    AnnotationType type;
    Rect rect;
    std::string contents;  // Text content
    std::string author;
    std::string subject;
    std::string created_date;
    std::string modified_date;
    Color color;
    float opacity;  // 0.0 to 1.0
    uint32_t flags;
    BorderStyle border;
    
    Annotation() : opacity(1.0f), flags(0) {}
    virtual ~Annotation() = default;
};

// Text annotation (sticky note)
struct TextAnnotation : public Annotation {
    enum class Icon {
        Comment,
        Key,
        Note,
        Help,
        NewParagraph,
        Paragraph,
        Insert
    };
    
    Icon icon;
    bool is_open;  // Initially open
    
    TextAnnotation() : icon(Icon::Note), is_open(false) {
        type = AnnotationType::Text;
    }
};

// Free text annotation
struct FreeTextAnnotation : public Annotation {
    std::string font_name;
    float font_size;
    Color text_color;
    
    enum class Alignment {
        Left,
        Center,
        Right
    };
    Alignment alignment;
    
    FreeTextAnnotation() 
        : font_name("Helvetica")
        , font_size(12.0f)
        , text_color(Color::black())
        , alignment(Alignment::Left) {
        type = AnnotationType::FreeText;
    }
};

// Line annotation
struct LineAnnotation : public Annotation {
    Point start;
    Point end;
    LineEnding start_style;
    LineEnding end_style;
    
    LineAnnotation() 
        : start_style(LineEnding::None)
        , end_style(LineEnding::None) {
        type = AnnotationType::Line;
    }
};

// Square/Circle annotation
struct ShapeAnnotation : public Annotation {
    bool filled;
    Color fill_color;
    
    ShapeAnnotation(AnnotationType t = AnnotationType::Square) 
        : filled(false)
        , fill_color(Color::white()) {
        type = t;
    }
};

// Polygon/PolyLine annotation
struct PolyAnnotation : public Annotation {
    std::vector<Point> points;
    bool filled;
    Color fill_color;
    
    PolyAnnotation(AnnotationType t = AnnotationType::Polygon)
        : filled(false)
        , fill_color(Color::white()) {
        type = t;
    }
};

// Text markup annotation
struct TextMarkupAnnotation : public Annotation {
    std::vector<Rect> quad_points;  // Highlighted areas
    MarkupStyle style;
    
    TextMarkupAnnotation(MarkupStyle s = MarkupStyle::Highlight) 
        : style(s) {
        switch(s) {
            case MarkupStyle::Highlight: type = AnnotationType::Highlight; break;
            case MarkupStyle::Underline: type = AnnotationType::Underline; break;
            case MarkupStyle::Squiggly: type = AnnotationType::Squiggly; break;
            case MarkupStyle::StrikeOut: type = AnnotationType::StrikeOut; break;
        }
    }
};

// Stamp annotation
struct StampAnnotation : public Annotation {
    enum class StandardStamp {
        Approved,
        Experimental,
        NotApproved,
        AsIs,
        Expired,
        NotForPublicRelease,
        Confidential,
        Final,
        Sold,
        Departmental,
        ForComment,
        TopSecret,
        ForPublicRelease,
        Draft
    };
    
    StandardStamp stamp_type;
    std::string custom_image_path;  // For custom stamps
    
    StampAnnotation() : stamp_type(StandardStamp::Draft) {
        type = AnnotationType::Stamp;
    }
};

// Ink annotation (freehand drawing)
struct InkAnnotation : public Annotation {
    std::vector<std::vector<Point>> strokes;  // Multiple strokes
    
    InkAnnotation() {
        type = AnnotationType::Ink;
    }
};

// File attachment annotation
struct FileAttachmentAnnotation : public Annotation {
    std::string file_name;
    std::vector<uint8_t> file_data;
    std::string mime_type;
    
    enum class Icon {
        Graph,
        PushPin,
        Paperclip,
        Tag
    };
    Icon icon;
    
    FileAttachmentAnnotation() : icon(Icon::PushPin) {
        type = AnnotationType::FileAttachment;
    }
};

// Annotations management
class PDFEDITOR_API Annotations {
public:
    // ===== Querying =====
    
    // Get all annotations on a page
    static std::vector<std::shared_ptr<Annotation>> get_annotations(Page* page);
    
    // Get annotation by ID
    static Result<std::shared_ptr<Annotation>> get_annotation(
        Page* page,
        const std::string& id
    );
    
    // Get annotations by type
    static std::vector<std::shared_ptr<Annotation>> get_by_type(
        Page* page,
        AnnotationType type
    );
    
    // Get annotations in area
    static std::vector<std::shared_ptr<Annotation>> get_in_area(
        Page* page,
        const Rect& area
    );
    
    // Count annotations
    static int count(Page* page);
    static int count_by_type(Page* page, AnnotationType type);
    
    // ===== Creation =====
    
    // Add text annotation (sticky note)
    static Result<std::string> add_text(
        Page* page,
        const Point& position,
        const std::string& contents,
        TextAnnotation::Icon icon = TextAnnotation::Icon::Note
    );
    
    // Add free text annotation
    static Result<std::string> add_free_text(
        Page* page,
        const Rect& rect,
        const std::string& text,
        const std::string& font = "Helvetica",
        float font_size = 12.0f
    );
    
    // Add highlight
    static Result<std::string> add_highlight(
        Page* page,
        const std::vector<Rect>& areas,
        const Color& color = Color::yellow()
    );
    
    // Add underline
    static Result<std::string> add_underline(
        Page* page,
        const std::vector<Rect>& areas,
        const Color& color = Color::blue()
    );
    
    // Add strikeout
    static Result<std::string> add_strikeout(
        Page* page,
        const std::vector<Rect>& areas,
        const Color& color = Color::red()
    );
    
    // Add line
    static Result<std::string> add_line(
        Page* page,
        const Point& start,
        const Point& end,
        const Color& color = Color::black(),
        float width = 1.0f
    );
    
    // Add rectangle
    static Result<std::string> add_rectangle(
        Page* page,
        const Rect& rect,
        const Color& color = Color::black(),
        bool filled = false,
        const Color& fill_color = Color::white()
    );
    
    // Add circle
    static Result<std::string> add_circle(
        Page* page,
        const Rect& rect,
        const Color& color = Color::black(),
        bool filled = false,
        const Color& fill_color = Color::white()
    );
    
    // Add polygon
    static Result<std::string> add_polygon(
        Page* page,
        const std::vector<Point>& points,
        const Color& color = Color::black(),
        bool filled = false
    );
    
    // Add ink annotation (freehand)
    static Result<std::string> add_ink(
        Page* page,
        const std::vector<std::vector<Point>>& strokes,
        const Color& color = Color::black(),
        float width = 1.0f
    );
    
    // Add stamp
    static Result<std::string> add_stamp(
        Page* page,
        const Rect& rect,
        StampAnnotation::StandardStamp stamp_type
    );
    
    // Add custom stamp from image
    static Result<std::string> add_custom_stamp(
        Page* page,
        const Rect& rect,
        const std::string& image_path
    );
    
    // Add file attachment
    static Result<std::string> add_file_attachment(
        Page* page,
        const Point& position,
        const std::string& file_path
    );
    
    // ===== Modification =====
    
    // Update annotation
    static bool update(Page* page, const std::shared_ptr<Annotation>& annotation);
    
    // Set annotation contents
    static bool set_contents(
        Page* page,
        const std::string& id,
        const std::string& contents
    );
    
    // Set annotation color
    static bool set_color(
        Page* page,
        const std::string& id,
        const Color& color
    );
    
    // Set annotation rect
    static bool set_rect(
        Page* page,
        const std::string& id,
        const Rect& rect
    );
    
    // Set annotation flags
    static bool set_flags(
        Page* page,
        const std::string& id,
        uint32_t flags
    );
    
    // ===== Deletion =====
    
    // Remove annotation
    static bool remove(Page* page, const std::string& id);
    
    // Remove all annotations
    static bool remove_all(Page* page);
    
    // Remove annotations by type
    static bool remove_by_type(Page* page, AnnotationType type);
    
    // Remove annotations in area
    static bool remove_in_area(Page* page, const Rect& area);
    
    // ===== Import/Export =====
    
    // Export annotations to XFDF
    static std::string export_xfdf(Document* doc);
    
    // Import annotations from XFDF
    static bool import_xfdf(Document* doc, const std::string& xfdf);
    
    // Export page annotations to JSON
    static std::string export_json(Page* page);
    
    // Import annotations from JSON
    static bool import_json(Page* page, const std::string& json);
    
    // ===== Flattening =====
    
    // Flatten annotation (make it part of page content)
    static bool flatten(Page* page, const std::string& id);
    
    // Flatten all annotations on page
    static bool flatten_all(Page* page);
    
    // Flatten all annotations in document
    static bool flatten_all(Document* doc);
};

} // namespace pdfeditor
