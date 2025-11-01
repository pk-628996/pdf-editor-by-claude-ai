#include "pdfeditor/bookmarks.h"
#include "pdfeditor/core.h"
#include <algorithm>
#include <sstream>
#include <map>

#ifdef USE_MUPDF
#include <mupdf/fitz.h>
#endif

namespace pdfeditor {

namespace {
    // Generate unique ID for bookmark
    std::string generate_bookmark_id() {
        static int counter = 0;
        return "bm_" + std::to_string(++counter);
    }
    
    // Helper to convert MuPDF outline to BookmarkEntry
#ifdef USE_MUPDF
    void convert_outline_recursive(
        fz_context* ctx,
        fz_outline* outline,
        std::vector<BookmarkEntry>& entries,
        int level = 0
    ) {
        while (outline) {
            BookmarkEntry entry;
            entry.id = generate_bookmark_id();
            entry.title = outline->title ? outline->title : "";
            entry.level = level;
            entry.open = (outline->is_open != 0);
            
            // Set destination
            if (outline->page >= 0) {
                entry.destination.type = DestinationType::Page;
                entry.destination.page_index = outline->page;
                entry.destination.top = outline->y;
            } else if (outline->uri) {
                entry.destination.type = DestinationType::URI;
                entry.destination.uri = outline->uri;
            }
            
            // Convert children recursively
            if (outline->down) {
                convert_outline_recursive(ctx, outline->down, entry.children, level + 1);
            }
            
            entries.push_back(std::move(entry));
            outline = outline->next;
        }
    }
#endif
}

// Bookmarks implementation
std::vector<BookmarkEntry> Bookmarks::list(Document* doc) {
    if (!doc) return {};
    
    std::vector<BookmarkEntry> entries;
    
#ifdef USE_MUPDF
    void* handle = doc->get_handle();
    if (!handle) return {};
    
    fz_document* fz_doc = static_cast<fz_document*>(handle);
    fz_context* ctx = nullptr; // TODO: Get context from document
    
    if (!ctx) return {};
    
    fz_outline* outline = nullptr;
    fz_try(ctx) {
        outline = fz_load_outline(ctx, fz_doc);
        if (outline) {
            convert_outline_recursive(ctx, outline, entries);
        }
    }
    fz_always(ctx) {
        if (outline) {
            fz_drop_outline(ctx, outline);
        }
    }
    fz_catch(ctx) {
        // Error loading outline
    }
#endif
    
    return entries;
}

std::vector<BookmarkEntry> Bookmarks::list_flat(Document* doc) {
    std::vector<BookmarkEntry> result;
    std::vector<BookmarkEntry> hierarchical = list(doc);
    
    std::function<void(const std::vector<BookmarkEntry>&)> flatten;
    flatten = [&](const std::vector<BookmarkEntry>& entries) {
        for (const auto& entry : entries) {
            result.push_back(entry);
            if (!entry.children.empty()) {
                flatten(entry.children);
            }
        }
    };
    
    flatten(hierarchical);
    return result;
}

int Bookmarks::count(Document* doc) {
    return static_cast<int>(list_flat(doc).size());
}

Result<BookmarkEntry> Bookmarks::get(Document* doc, const std::string& id) {
    auto bookmarks = list_flat(doc);
    
    auto it = std::find_if(bookmarks.begin(), bookmarks.end(),
        [&id](const BookmarkEntry& entry) { return entry.id == id; });
    
    if (it != bookmarks.end()) {
        return Result<BookmarkEntry>(*it);
    }
    
    return Result<BookmarkEntry>(
        ErrorCode::InvalidArgument,
        "Bookmark not found"
    );
}

Result<std::string> Bookmarks::add(
    Document* doc,
    const std::string& parent_id,
    const std::string& title,
    const BookmarkDestination& destination
) {
    if (!doc) {
        return Result<std::string>(ErrorCode::InvalidArgument, "Invalid document");
    }
    
    // TODO: Implement actual bookmark addition using MuPDF
    // For now, return a placeholder ID
    std::string new_id = generate_bookmark_id();
    
    // In real implementation:
    // 1. Get PDF document handle
    // 2. Find parent bookmark (or root if parent_id is empty)
    // 3. Create new outline entry
    // 4. Set title, destination, etc.
    // 5. Insert into outline tree
    // 6. Mark document as modified
    
    return Result<std::string>(new_id);
}

Result<std::string> Bookmarks::add_to_page(
    Document* doc,
    const std::string& parent_id,
    const std::string& title,
    int page_index,
    float top
) {
    BookmarkDestination dest;
    dest.type = DestinationType::Page;
    dest.page_index = page_index;
    dest.top = top;
    
    return add(doc, parent_id, title, dest);
}

bool Bookmarks::remove(Document* doc, const std::string& bookmark_id) {
    if (!doc) return false;
    
    // TODO: Implement bookmark removal
    // 1. Find bookmark in outline tree
    // 2. Remove from parent's children
    // 3. Free resources
    // 4. Mark document as modified
    
    return true;
}

bool Bookmarks::edit(
    Document* doc,
    const std::string& bookmark_id,
    const BookmarkEntry& new_values
) {
    if (!doc) return false;
    
    // TODO: Implement bookmark editing
    // Update title, destination, style, etc.
    
    return true;
}

bool Bookmarks::set_title(
    Document* doc,
    const std::string& bookmark_id,
    const std::string& title
) {
    // TODO: Implement
    return true;
}

bool Bookmarks::set_destination(
    Document* doc,
    const std::string& bookmark_id,
    const BookmarkDestination& destination
) {
    // TODO: Implement
    return true;
}

bool Bookmarks::set_style(
    Document* doc,
    const std::string& bookmark_id,
    const Color& color,
    bool bold,
    bool italic
) {
    // TODO: Implement
    return true;
}

bool Bookmarks::move(
    Document* doc,
    const std::string& bookmark_id,
    const std::string& new_parent_id,
    int new_index
) {
    if (!doc) return false;
    
    // TODO: Implement bookmark moving
    // 1. Find bookmark and new parent
    // 2. Remove from current parent
    // 3. Insert into new parent at specified index
    // 4. Update outline structure
    
    return true;
}

bool Bookmarks::reorder(
    Document* doc,
    const std::string& bookmark_id,
    int new_index
) {
    // TODO: Implement reordering within same parent
    return true;
}

bool Bookmarks::set_open_state(
    Document* doc,
    const std::string& bookmark_id,
    bool open
) {
    // TODO: Implement open/close state
    return true;
}

bool Bookmarks::expand_all(Document* doc) {
    auto bookmarks = list_flat(doc);
    for (const auto& bm : bookmarks) {
        set_open_state(doc, bm.id, true);
    }
    return true;
}

bool Bookmarks::collapse_all(Document* doc) {
    auto bookmarks = list_flat(doc);
    for (const auto& bm : bookmarks) {
        set_open_state(doc, bm.id, false);
    }
    return true;
}

std::string Bookmarks::export_json(Document* doc) {
    auto bookmarks = list(doc);
    
    // Simple JSON serialization (in production, use proper JSON library)
    std::ostringstream json;
    json << "{\n  \"bookmarks\": [\n";
    
    std::function<void(const std::vector<BookmarkEntry>&, int)> write_entries;
    write_entries = [&](const std::vector<BookmarkEntry>& entries, int indent) {
        for (size_t i = 0; i < entries.size(); ++i) {
            const auto& entry = entries[i];
            std::string indent_str(indent, ' ');
            
            json << indent_str << "{\n";
            json << indent_str << "  \"id\": \"" << entry.id << "\",\n";
            json << indent_str << "  \"title\": \"" << entry.title << "\",\n";
            json << indent_str << "  \"page\": " << entry.destination.page_index << ",\n";
            json << indent_str << "  \"level\": " << entry.level;
            
            if (!entry.children.empty()) {
                json << ",\n" << indent_str << "  \"children\": [\n";
                write_entries(entry.children, indent + 4);
                json << indent_str << "  ]\n";
            } else {
                json << "\n";
            }
            
            json << indent_str << "}";
            if (i < entries.size() - 1) json << ",";
            json << "\n";
        }
    };
    
    write_entries(bookmarks, 4);
    
    json << "  ]\n}\n";
    return json.str();
}

bool Bookmarks::import_json(Document* doc, const std::string& json) {
    // TODO: Parse JSON and create bookmarks
    return true;
}

std::string Bookmarks::export_opml(Document* doc) {
    auto bookmarks = list(doc);
    
    std::ostringstream opml;
    opml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    opml << "<opml version=\"2.0\">\n";
    opml << "  <head>\n";
    opml << "    <title>PDF Bookmarks</title>\n";
    opml << "  </head>\n";
    opml << "  <body>\n";
    
    std::function<void(const std::vector<BookmarkEntry>&, int)> write_outline;
    write_outline = [&](const std::vector<BookmarkEntry>& entries, int indent) {
        std::string indent_str(indent, ' ');
        for (const auto& entry : entries) {
            opml << indent_str << "<outline text=\"" << entry.title << "\" ";
            opml << "page=\"" << (entry.destination.page_index + 1) << "\"";
            if (!entry.children.empty()) {
                opml << ">\n";
                write_outline(entry.children, indent + 2);
                opml << indent_str << "</outline>\n";
            } else {
                opml << " />\n";
            }
        }
    };
    
    write_outline(bookmarks, 4);
    
    opml << "  </body>\n";
    opml << "</opml>\n";
    
    return opml.str();
}

bool Bookmarks::import_opml(Document* doc, const std::string& opml) {
    // TODO: Parse OPML and create bookmarks
    return true;
}

std::string Bookmarks::export_text(Document* doc) {
    auto bookmarks = list_flat(doc);
    
    std::ostringstream text;
    for (const auto& bm : bookmarks) {
        // Indent based on level
        for (int i = 0; i < bm.level; ++i) {
            text << "  ";
        }
        text << bm.title << " (page " << (bm.destination.page_index + 1) << ")\n";
    }
    
    return text.str();
}

bool Bookmarks::clear(Document* doc) {
    if (!doc) return false;
    
    // TODO: Remove all bookmarks
    return true;
}

bool Bookmarks::auto_generate(
    Document* doc,
    bool from_headings,
    bool from_toc
) {
    // TODO: Analyze document structure and generate bookmarks
    return true;
}

bool Bookmarks::update_destinations_after_page_changes(
    Document* doc,
    const std::map<int, int>& page_mapping
) {
    auto bookmarks = list_flat(doc);
    
    for (const auto& bm : bookmarks) {
        auto it = page_mapping.find(bm.destination.page_index);
        if (it != page_mapping.end()) {
            BookmarkDestination new_dest = bm.destination;
            new_dest.page_index = it->second;
            set_destination(doc, bm.id, new_dest);
        }
    }
    
    return true;
}

bool Bookmarks::validate(Document* doc) {
    if (!doc) return false;
    
    int page_count = doc->page_count();
    auto bookmarks = list_flat(doc);
    
    for (const auto& bm : bookmarks) {
        if (bm.destination.type == DestinationType::Page) {
            if (bm.destination.page_index < 0 || 
                bm.destination.page_index >= page_count) {
                return false;
            }
        }
    }
    
    return true;
}

std::vector<std::string> Bookmarks::find_broken(Document* doc) {
    std::vector<std::string> broken;
    
    if (!doc) return broken;
    
    int page_count = doc->page_count();
    auto bookmarks = list_flat(doc);
    
    for (const auto& bm : bookmarks) {
        if (bm.destination.type == DestinationType::Page) {
            if (bm.destination.page_index < 0 || 
                bm.destination.page_index >= page_count) {
                broken.push_back(bm.id);
            }
        }
    }
    
    return broken;
}

bool Bookmarks::fix_broken(Document* doc, bool remove) {
    auto broken = find_broken(doc);
    
    for (const auto& id : broken) {
        if (remove) {
            Bookmarks::remove(doc, id);
        } else {
            // Redirect to page 0
            BookmarkDestination dest;
            dest.type = DestinationType::Page;
            dest.page_index = 0;
            set_destination(doc, id, dest);
        }
    }
    
    return true;
}

std::vector<BookmarkEntry> Bookmarks::find_by_title(
    Document* doc,
    const std::string& search_text
) {
    std::vector<BookmarkEntry> results;
    auto bookmarks = list_flat(doc);
    
    std::string search_lower = search_text;
    std::transform(search_lower.begin(), search_lower.end(), 
                   search_lower.begin(), ::tolower);
    
    for (const auto& bm : bookmarks) {
        std::string title_lower = bm.title;
        std::transform(title_lower.begin(), title_lower.end(),
                       title_lower.begin(), ::tolower);
        
        if (title_lower.find(search_lower) != std::string::npos) {
            results.push_back(bm);
        }
    }
    
    return results;
}

std::vector<BookmarkEntry> Bookmarks::find_by_page(
    Document* doc,
    int page_index
) {
    std::vector<BookmarkEntry> results;
    auto bookmarks = list_flat(doc);
    
    for (const auto& bm : bookmarks) {
        if (bm.destination.type == DestinationType::Page &&
            bm.destination.page_index == page_index) {
            results.push_back(bm);
        }
    }
    
    return results;
}

int Bookmarks::get_max_depth(Document* doc) {
    auto bookmarks = list_flat(doc);
    
    int max_depth = 0;
    for (const auto& bm : bookmarks) {
        max_depth = std::max(max_depth, bm.level);
    }
    
    return max_depth + 1;  // Convert level to depth
}

int Bookmarks::count_at_level(Document* doc, int level) {
    auto bookmarks = list_flat(doc);
    
    return std::count_if(bookmarks.begin(), bookmarks.end(),
        [level](const BookmarkEntry& bm) { return bm.level == level; });
}

// Named Destinations implementation
std::map<std::string, BookmarkDestination> NamedDestinations::list(Document* doc) {
    // TODO: Implement
    return {};
}

Result<BookmarkDestination> NamedDestinations::get(
    Document* doc,
    const std::string& name
) {
    // TODO: Implement
    return Result<BookmarkDestination>(ErrorCode::NotImplemented);
}

bool NamedDestinations::add(
    Document* doc,
    const std::string& name,
    const BookmarkDestination& destination
) {
    // TODO: Implement
    return true;
}

bool NamedDestinations::remove(Document* doc, const std::string& name) {
    // TODO: Implement
    return true;
}

bool NamedDestinations::exists(Document* doc, const std::string& name) {
    // TODO: Implement
    return false;
}

} // namespace pdfeditor
