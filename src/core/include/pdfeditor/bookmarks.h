#pragma once

#include "core.h"
#include "document.h"
#include <string>
#include <vector>
#include <memory>

namespace pdfeditor {

// Bookmark destination types
enum class DestinationType {
    Page,           // Go to a specific page
    XYZ,            // Go to specific coordinates with zoom
    Fit,            // Fit page in window
    FitH,           // Fit page width
    FitV,           // Fit page height
    FitR,           // Fit rectangle
    FitB,           // Fit bounding box
    FitBH,          // Fit bounding box width
    FitBV,          // Fit bounding box height
    URI,            // External URI link
    NamedDest       // Named destination
};

// Bookmark destination
struct BookmarkDestination {
    DestinationType type;
    int page_index;         // Target page (0-indexed)
    float left;             // X coordinate (-1 if not used)
    float top;              // Y coordinate (-1 if not used)
    float right;            // Right coordinate for FitR
    float bottom;           // Bottom coordinate for FitR
    float zoom;             // Zoom level (-1 for no zoom)
    std::string uri;        // For URI type
    std::string named_dest; // For NamedDest type
    
    BookmarkDestination() 
        : type(DestinationType::Page)
        , page_index(0)
        , left(-1), top(-1), right(-1), bottom(-1)
        , zoom(-1) {}
};

// Bookmark entry structure
struct BookmarkEntry {
    std::string id;                     // Unique stable identifier
    std::string title;                  // Display title
    BookmarkDestination destination;     // Where bookmark points to
    bool open;                          // Expanded state in UI
    Color color;                        // Display color (default black)
    bool bold;                          // Bold text style
    bool italic;                        // Italic text style
    int level;                          // Nesting level (0 = root)
    std::vector<BookmarkEntry> children; // Child bookmarks
    
    BookmarkEntry() 
        : open(false)
        , color(Color::black())
        , bold(false)
        , italic(false)
        , level(0) {}
};

// Bookmarks management class
class PDFEDITOR_API Bookmarks {
public:
    // Get all bookmarks from document (hierarchical)
    static std::vector<BookmarkEntry> list(Document* doc);
    
    // Get flat list of all bookmarks (depth-first order)
    static std::vector<BookmarkEntry> list_flat(Document* doc);
    
    // Get bookmark count
    static int count(Document* doc);
    
    // Get bookmark by ID
    static Result<BookmarkEntry> get(Document* doc, const std::string& id);
    
    // Add bookmark
    // parent_id: empty string for root level
    // Returns: ID of newly created bookmark
    static Result<std::string> add(
        Document* doc,
        const std::string& parent_id,
        const std::string& title,
        const BookmarkDestination& destination
    );
    
    // Add simple bookmark to page
    static Result<std::string> add_to_page(
        Document* doc,
        const std::string& parent_id,
        const std::string& title,
        int page_index,
        float top = -1.0
    );
    
    // Remove bookmark and all its children
    static bool remove(Document* doc, const std::string& bookmark_id);
    
    // Edit bookmark properties
    static bool edit(
        Document* doc,
        const std::string& bookmark_id,
        const BookmarkEntry& new_values
    );
    
    // Update bookmark title
    static bool set_title(
        Document* doc,
        const std::string& bookmark_id,
        const std::string& title
    );
    
    // Update bookmark destination
    static bool set_destination(
        Document* doc,
        const std::string& bookmark_id,
        const BookmarkDestination& destination
    );
    
    // Update bookmark style
    static bool set_style(
        Document* doc,
        const std::string& bookmark_id,
        const Color& color,
        bool bold,
        bool italic
    );
    
    // Move bookmark to new parent/position
    // new_index: position among siblings (-1 = append to end)
    static bool move(
        Document* doc,
        const std::string& bookmark_id,
        const std::string& new_parent_id,
        int new_index = -1
    );
    
    // Reorder bookmark within same parent
    static bool reorder(
        Document* doc,
        const std::string& bookmark_id,
        int new_index
    );
    
    // Expand/collapse bookmark
    static bool set_open_state(
        Document* doc,
        const std::string& bookmark_id,
        bool open
    );
    
    // Expand all bookmarks recursively
    static bool expand_all(Document* doc);
    
    // Collapse all bookmarks
    static bool collapse_all(Document* doc);
    
    // Import/Export
    
    // Export to JSON format
    static std::string export_json(Document* doc);
    
    // Import from JSON format
    static bool import_json(Document* doc, const std::string& json);
    
    // Export to OPML format (Outline Processor Markup Language)
    static std::string export_opml(Document* doc);
    
    // Import from OPML format
    static bool import_opml(Document* doc, const std::string& opml);
    
    // Export to simple text format (indented hierarchy)
    static std::string export_text(Document* doc);
    
    // Batch operations
    
    // Remove all bookmarks
    static bool clear(Document* doc);
    
    // Auto-generate bookmarks from document structure
    // (headings, sections based on text analysis)
    static bool auto_generate(
        Document* doc,
        bool from_headings = true,
        bool from_toc = true
    );
    
    // Update all bookmark destinations after page operations
    // (e.g., after deleting/moving pages)
    static bool update_destinations_after_page_changes(
        Document* doc,
        const std::map<int, int>& page_mapping  // old_index -> new_index
    );
    
    // Validation
    
    // Check if all bookmark destinations are valid
    static bool validate(Document* doc);
    
    // Get list of broken bookmarks (invalid destinations)
    static std::vector<std::string> find_broken(Document* doc);
    
    // Fix broken bookmarks (remove or redirect to page 0)
    static bool fix_broken(Document* doc, bool remove = false);
    
    // Utility functions
    
    // Find bookmarks by title (case-insensitive search)
    static std::vector<BookmarkEntry> find_by_title(
        Document* doc,
        const std::string& search_text
    );
    
    // Find bookmarks pointing to specific page
    static std::vector<BookmarkEntry> find_by_page(
        Document* doc,
        int page_index
    );
    
    // Get bookmark tree depth
    static int get_max_depth(Document* doc);
    
    // Count bookmarks at specific level
    static int count_at_level(Document* doc, int level);
};

// Named destinations management
class PDFEDITOR_API NamedDestinations {
public:
    // List all named destinations
    static std::map<std::string, BookmarkDestination> list(Document* doc);
    
    // Get named destination
    static Result<BookmarkDestination> get(
        Document* doc,
        const std::string& name
    );
    
    // Add named destination
    static bool add(
        Document* doc,
        const std::string& name,
        const BookmarkDestination& destination
    );
    
    // Remove named destination
    static bool remove(Document* doc, const std::string& name);
    
    // Check if named destination exists
    static bool exists(Document* doc, const std::string& name);
};

} // namespace pdfeditor
