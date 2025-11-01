#pragma once

#include "core.h"
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace pdfeditor {

// Forward declarations
class Page;
class Outline;

// Document information
struct DocumentInfo {
    std::string title;
    std::string author;
    std::string subject;
    std::string keywords;
    std::string creator;
    std::string producer;
    std::string creation_date;
    std::string modification_date;
    bool is_encrypted;
    bool is_linearized;
    PDFVersion version;
    size_t file_size;
    int page_count;
};

// Document class
class PDFEDITOR_API Document {
public:
    // Open document from file
    static Result<std::unique_ptr<Document>> open(
        const std::string& path, 
        const std::string& password = ""
    );
    
    // Create new empty document
    static std::unique_ptr<Document> create();
    
    // Open from memory buffer
    static Result<std::unique_ptr<Document>> open_from_memory(
        const uint8_t* data,
        size_t size,
        const std::string& password = ""
    );
    
    virtual ~Document();
    
    // Save operations
    bool save(const std::string& path);
    bool save_incremental(const std::string& path);
    bool save_to_memory(std::vector<uint8_t>& buffer);
    
    // Document properties
    int page_count() const;
    DocumentInfo get_info() const;
    PDFVersion get_version() const;
    
    // Page access
    Page* get_page(int index);
    const Page* get_page(int index) const;
    std::vector<Page*> get_pages();
    
    // Page manipulation
    bool insert_page(int index, float width, float height);
    bool delete_page(int index);
    bool move_page(int from_index, int to_index);
    bool rotate_page(int index, PageRotation rotation);
    
    // Page extraction and merging
    std::unique_ptr<Document> extract_pages(const std::vector<int>& page_indices);
    bool merge_document(const Document& other, int insert_at = -1);
    
    // Document metadata
    std::string get_title() const;
    std::string get_author() const;
    std::string get_subject() const;
    std::string get_keywords() const;
    std::string get_creator() const;
    std::string get_producer() const;
    
    void set_title(const std::string& title);
    void set_author(const std::string& author);
    void set_subject(const std::string& subject);
    void set_keywords(const std::string& keywords);
    void set_creator(const std::string& creator);
    void set_producer(const std::string& producer);
    
    // Security
    bool is_encrypted() const;
    bool has_user_password() const;
    bool has_owner_password() const;
    uint32_t get_permissions() const;
    
    bool set_password(
        const std::string& user_password,
        const std::string& owner_password,
        uint32_t permissions = static_cast<uint32_t>(Permission::All)
    );
    bool remove_password();
    
    // Linearization (Fast Web View)
    bool is_linearized() const;
    bool linearize();
    
    // Optimization
    bool optimize(CompressionLevel level = CompressionLevel::Default);
    bool compress_images(int quality = 85);
    bool remove_unused_objects();
    
    // Search
    struct SearchResult {
        int page_index;
        Rect bounding_box;
        std::string context;
    };
    
    std::vector<SearchResult> search(
        const std::string& text,
        bool case_sensitive = false,
        bool whole_words = false
    );
    
    // Outline/Bookmarks access
    bool has_outline() const;
    Outline* get_outline();
    
    // Form fields
    bool has_forms() const;
    int get_form_field_count() const;
    
    // JavaScript
    bool has_javascript() const;
    std::vector<std::string> get_javascript_names() const;
    std::string get_javascript(const std::string& name) const;
    
    // Validation
    bool validate() const;
    std::vector<std::string> get_validation_errors() const;
    
    // PDF/A compliance
    bool is_pdfa_compliant() const;
    bool convert_to_pdfa();
    
    // Internal handle (for advanced use)
    void* get_handle() const;
    
private:
    Document();
    
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Page class
class PDFEDITOR_API Page {
public:
    virtual ~Page();
    
    // Page dimensions
    float width() const;
    float height() const;
    Rect get_mediabox() const;
    Rect get_cropbox() const;
    
    void set_mediabox(const Rect& box);
    void set_cropbox(const Rect& box);
    
    // Page properties
    int number() const;  // 1-indexed
    int index() const;   // 0-indexed
    
    // Rotation
    PageRotation rotation() const;
    void set_rotation(PageRotation rotation);
    
    // Text extraction
    std::string get_text() const;
    std::string get_text(const Rect& area) const;
    
    struct TextBlock {
        std::string text;
        Rect bounding_box;
        float font_size;
        std::string font_name;
        Color color;
    };
    
    std::vector<TextBlock> get_text_blocks() const;
    
    // Image extraction
    struct ImageInfo {
        int width;
        int height;
        int bits_per_component;
        ColorSpace color_space;
        Rect bounding_box;
        std::vector<uint8_t> data;
    };
    
    std::vector<ImageInfo> get_images() const;
    
    // Links
    struct Link {
        Rect area;
        std::string uri;
        int destination_page;  // -1 if external link
    };
    
    std::vector<Link> get_links() const;
    
    // Annotations
    int get_annotation_count() const;
    
    // Content manipulation
    bool insert_text(const std::string& text, const Point& position);
    bool insert_image(const std::string& image_path, const Rect& rect);
    
    // Internal handle
    void* get_handle() const;
    
private:
    friend class Document;
    Page();
    
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Outline (bookmarks) - detailed in bookmarks.h
class PDFEDITOR_API Outline {
public:
    virtual ~Outline();
    
    struct Item {
        std::string title;
        int page_index;
        Point position;
        bool is_open;
        std::vector<Item> children;
    };
    
    std::vector<Item> get_items() const;
    bool add_item(const std::string& title, int page_index);
    bool remove_item(int index);
    
private:
    Outline();
    
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace pdfeditor
