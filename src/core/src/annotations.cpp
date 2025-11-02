#include "pdfeditor/annotations.h"
#include "pdfeditor/core.h"
#include <algorithm>
#include <sstream>

#ifdef USE_MUPDF
#include <mupdf/fitz.h>
#endif

namespace pdfeditor {

namespace {
    std::string generate_annotation_id() {
        static int counter = 0;
        return "annot_" + std::to_string(++counter);
    }
}

// Annotations implementation
std::vector<std::shared_ptr<Annotation>> Annotations::get_annotations(Page* page) {
    std::vector<std::shared_ptr<Annotation>> annotations;
    
    if (!page) return annotations;
    
#ifdef USE_MUPDF
    void* handle = page->get_handle();
    if (!handle) return annotations;
    
    fz_page* fz_pg = static_cast<fz_page*>(handle);
    // TODO: Get context from page
    fz_context* ctx = nullptr;
    
    if (!ctx) return annotations;
    
    fz_try(ctx) {
        pdf_page* pdf_pg = pdf_page_from_fz_page(ctx, fz_pg);
        if (pdf_pg) {
            pdf_annot* annot = pdf_first_annot(ctx, pdf_pg);
            while (annot) {
                auto annotation = std::make_shared<Annotation>();
                annotation->id = generate_annotation_id();
                annotation->type = AnnotationType::Text; // TODO: Get actual type
                
                // Get rect
                fz_rect rect = pdf_bound_annot(ctx, annot);
                annotation->rect = Rect(rect.x0, rect.y0, rect.x1, rect.y1);
                
                // Get contents
                const char* contents = pdf_annot_contents(ctx, annot);
                if (contents) {
                    annotation->contents = contents;
                }
                
                annotations.push_back(annotation);
                annot = pdf_next_annot(ctx, annot);
            }
        }
    }
    fz_catch(ctx) {
        // Error loading annotations
    }
#endif
    
    return annotations;
}

Result<std::shared_ptr<Annotation>> Annotations::get_annotation(
    Page* page,
    const std::string& id
) {
    auto annotations = get_annotations(page);
    
    auto it = std::find_if(annotations.begin(), annotations.end(),
        [&id](const std::shared_ptr<Annotation>& a) { return a->id == id; });
    
    if (it != annotations.end()) {
        return Result<std::shared_ptr<Annotation>>(*it);
    }
    
    return Result<std::shared_ptr<Annotation>>(
        ErrorCode::InvalidArgument,
        "Annotation not found"
    );
}

std::vector<std::shared_ptr<Annotation>> Annotations::get_by_type(
    Page* page,
    AnnotationType type
) {
    std::vector<std::shared_ptr<Annotation>> result;
    auto all = get_annotations(page);
    
    std::copy_if(all.begin(), all.end(), std::back_inserter(result),
        [type](const std::shared_ptr<Annotation>& a) { return a->type == type; });
    
    return result;
}

std::vector<std::shared_ptr<Annotation>> Annotations::get_in_area(
    Page* page,
    const Rect& area
) {
    std::vector<std::shared_ptr<Annotation>> result;
    auto all = get_annotations(page);
    
    for (const auto& annot : all) {
        // Check if annotation rect intersects with area
        if (annot->rect.x0 < area.x1 && annot->rect.x1 > area.x0 &&
            annot->rect.y0 < area.y1 && annot->rect.y1 > area.y0) {
            result.push_back(annot);
        }
    }
    
    return result;
}

int Annotations::count(Page* page) {
    return static_cast<int>(get_annotations(page).size());
}

int Annotations::count_by_type(Page* page, AnnotationType type) {
    return static_cast<int>(get_by_type(page, type).size());
}

Result<std::string> Annotations::add_text(
    Page* page,
    const Point& position,
    const std::string& contents,
    TextAnnotation::Icon icon
) {
    if (!page) {
        return Result<std::string>(ErrorCode::InvalidArgument, "Invalid page");
    }
    
    std::string id = generate_annotation_id();
    
#ifdef USE_MUPDF
    void* handle = page->get_handle();
    if (!handle) {
        return Result<std::string>(ErrorCode::InvalidArgument, "Invalid page handle");
    }
    
    // TODO: Implement actual annotation creation with MuPDF
    // This is a stub that would need full MuPDF annotation API integration
#endif
    
    return Result<std::string>(id);
}

Result<std::string> Annotations::add_free_text(
    Page* page,
    const Rect& rect,
    const std::string& text,
    const std::string& font,
    float font_size
) {
    if (!page) {
        return Result<std::string>(ErrorCode::InvalidArgument, "Invalid page");
    }
    
    std::string id = generate_annotation_id();
    // TODO: Implement with MuPDF
    return Result<std::string>(id);
}

Result<std::string> Annotations::add_highlight(
    Page* page,
    const std::vector<Rect>& areas,
    const Color& color
) {
    if (!page || areas.empty()) {
        return Result<std::string>(ErrorCode::InvalidArgument, "Invalid arguments");
    }
    
    std::string id = generate_annotation_id();
    // TODO: Implement with MuPDF
    return Result<std::string>(id);
}

Result<std::string> Annotations::add_underline(
    Page* page,
    const std::vector<Rect>& areas,
    const Color& color
) {
    if (!page || areas.empty()) {
        return Result<std::string>(ErrorCode::InvalidArgument, "Invalid arguments");
    }
    
    std::string id = generate_annotation_id();
    // TODO: Implement with MuPDF
    return Result<std::string>(id);
}

Result<std::string> Annotations::add_strikeout(
    Page* page,
    const std::vector<Rect>& areas,
    const Color& color
) {
    if (!page || areas.empty()) {
        return Result<std::string>(ErrorCode::InvalidArgument, "Invalid arguments");
    }
    
    std::string id = generate_annotation_id();
    // TODO: Implement with MuPDF
    return Result<std::string>(id);
}

Result<std::string> Annotations::add_line(
    Page* page,
    const Point& start,
    const Point& end,
    const Color& color,
    float width
) {
    if (!page) {
        return Result<std::string>(ErrorCode::InvalidArgument, "Invalid page");
    }
    
    std::string id = generate_annotation_id();
    // TODO: Implement with MuPDF
    return Result<std::string>(id);
}

Result<std::string> Annotations::add_rectangle(
    Page* page,
    const Rect& rect,
    const Color& color,
    bool filled,
    const Color& fill_color
) {
    if (!page) {
        return Result<std::string>(ErrorCode::InvalidArgument, "Invalid page");
    }
    
    std::string id = generate_annotation_id();
    // TODO: Implement with MuPDF
    return Result<std::string>(id);
}

Result<std::string> Annotations::add_circle(
    Page* page,
    const Rect& rect,
    const Color& color,
    bool filled,
    const Color& fill_color
) {
    if (!page) {
        return Result<std::string>(ErrorCode::InvalidArgument, "Invalid page");
    }
    
    std::string id = generate_annotation_id();
    // TODO: Implement with MuPDF
    return Result<std::string>(id);
}

Result<std::string> Annotations::add_polygon(
    Page* page,
    const std::vector<Point>& points,
    const Color& color,
    bool filled
) {
    if (!page || points.size() < 3) {
        return Result<std::string>(ErrorCode::InvalidArgument, "Invalid arguments");
    }
    
    std::string id = generate_annotation_id();
    // TODO: Implement with MuPDF
    return Result<std::string>(id);
}

Result<std::string> Annotations::add_ink(
    Page* page,
    const std::vector<std::vector<Point>>& strokes,
    const Color& color,
    float width
) {
    if (!page || strokes.empty()) {
        return Result<std::string>(ErrorCode::InvalidArgument, "Invalid arguments");
    }
    
    std::string id = generate_annotation_id();
    // TODO: Implement with MuPDF
    return Result<std::string>(id);
}

Result<std::string> Annotations::add_stamp(
    Page* page,
    const Rect& rect,
    StampAnnotation::StandardStamp stamp_type
) {
    if (!page) {
        return Result<std::string>(ErrorCode::InvalidArgument, "Invalid page");
    }
    
    std::string id = generate_annotation_id();
    // TODO: Implement with MuPDF
    return Result<std::string>(id);
}

Result<std::string> Annotations::add_custom_stamp(
    Page* page,
    const Rect& rect,
    const std::string& image_path
) {
    if (!page) {
        return Result<std::string>(ErrorCode::InvalidArgument, "Invalid page");
    }
    
    std::string id = generate_annotation_id();
    // TODO: Implement with MuPDF
    return Result<std::string>(id);
}

Result<std::string> Annotations::add_file_attachment(
    Page* page,
    const Point& position,
    const std::string& file_path
) {
    if (!page) {
        return Result<std::string>(ErrorCode::InvalidArgument, "Invalid page");
    }
    
    std::string id = generate_annotation_id();
    // TODO: Implement with MuPDF
    return Result<std::string>(id);
}

bool Annotations::update(Page* page, const std::shared_ptr<Annotation>& annotation) {
    if (!page || !annotation) return false;
    // TODO: Implement with MuPDF
    return true;
}

bool Annotations::set_contents(
    Page* page,
    const std::string& id,
    const std::string& contents
) {
    if (!page) return false;
    // TODO: Implement with MuPDF
    return true;
}

bool Annotations::set_color(
    Page* page,
    const std::string& id,
    const Color& color
) {
    if (!page) return false;
    // TODO: Implement with MuPDF
    return true;
}

bool Annotations::set_rect(
    Page* page,
    const std::string& id,
    const Rect& rect
) {
    if (!page) return false;
    // TODO: Implement with MuPDF
    return true;
}

bool Annotations::set_flags(
    Page* page,
    const std::string& id,
    uint32_t flags
) {
    if (!page) return false;
    // TODO: Implement with MuPDF
    return true;
}

bool Annotations::remove(Page* page, const std::string& id) {
    if (!page) return false;
    // TODO: Implement with MuPDF
    return true;
}

bool Annotations::remove_all(Page* page) {
    if (!page) return false;
    // TODO: Implement with MuPDF - remove all annotations
    return true;
}

bool Annotations::remove_by_type(Page* page, AnnotationType type) {
    if (!page) return false;
    
    auto annotations = get_by_type(page, type);
    for (const auto& annot : annotations) {
        remove(page, annot->id);
    }
    
    return true;
}

bool Annotations::remove_in_area(Page* page, const Rect& area) {
    if (!page) return false;
    
    auto annotations = get_in_area(page, area);
    for (const auto& annot : annotations) {
        remove(page, annot->id);
    }
    
    return true;
}

std::string Annotations::export_xfdf(Document* doc) {
    if (!doc) return "";
    
    std::ostringstream xfdf;
    xfdf << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xfdf << "<xfdf xmlns=\"http://ns.adobe.com/xfdf/\" xml:space=\"preserve\">\n";
    xfdf << "  <annots>\n";
    
    // TODO: Export all annotations from all pages
    
    xfdf << "  </annots>\n";
    xfdf << "</xfdf>\n";
    
    return xfdf.str();
}

bool Annotations::import_xfdf(Document* doc, const std::string& xfdf) {
    if (!doc) return false;
    // TODO: Parse XFDF and create annotations
    return true;
}

std::string Annotations::export_json(Page* page) {
    if (!page) return "{}";
    
    auto annotations = get_annotations(page);
    
    std::ostringstream json;
    json << "{\n  \"annotations\": [\n";
    
    for (size_t i = 0; i < annotations.size(); ++i) {
        const auto& annot = annotations[i];
        json << "    {\n";
        json << "      \"id\": \"" << annot->id << "\",\n";
        json << "      \"type\": " << static_cast<int>(annot->type) << ",\n";
        json << "      \"rect\": [" << annot->rect.x0 << ", " << annot->rect.y0 
             << ", " << annot->rect.x1 << ", " << annot->rect.y1 << "],\n";
        json << "      \"contents\": \"" << annot->contents << "\"\n";
        json << "    }";
        if (i < annotations.size() - 1) json << ",";
        json << "\n";
    }
    
    json << "  ]\n}\n";
    return json.str();
}

bool Annotations::import_json(Page* page, const std::string& json) {
    if (!page) return false;
    // TODO: Parse JSON and create annotations
    return true;
}

bool Annotations::flatten(Page* page, const std::string& id) {
    if (!page) return false;
    // TODO: Convert annotation to page content
    return true;
}

bool Annotations::flatten_all(Page* page) {
    if (!page) return false;
    
    auto annotations = get_annotations(page);
    for (const auto& annot : annotations) {
        flatten(page, annot->id);
    }
    
    return true;
}

bool Annotations::flatten_all(Document* doc) {
    if (!doc) return false;
    
    for (int i = 0; i < doc->page_count(); ++i) {
        Page* page = doc->get_page(i);
        if (page) {
            flatten_all(page);
        }
    }
    
    return true;
}

} // namespace pdfeditor
