#pragma once

#include "core.h"
#include "document.h"
#include <string>
#include <map>
#include <vector>

namespace pdfeditor {

// Metadata key-value map
using MetadataMap = std::map<std::string, std::string>;

// Standard PDF Info Dictionary keys
namespace InfoKeys {
    constexpr const char* Title = "Title";
    constexpr const char* Author = "Author";
    constexpr const char* Subject = "Subject";
    constexpr const char* Keywords = "Keywords";
    constexpr const char* Creator = "Creator";
    constexpr const char* Producer = "Producer";
    constexpr const char* CreationDate = "CreationDate";
    constexpr const char* ModDate = "ModDate";
    constexpr const char* Trapped = "Trapped";
}

// XMP namespace URIs
namespace XMPNamespaces {
    constexpr const char* DC = "http://purl.org/dc/elements/1.1/";
    constexpr const char* XMP = "http://ns.adobe.com/xap/1.0/";
    constexpr const char* PDF = "http://ns.adobe.com/pdf/1.3/";
    constexpr const char* PDFAID = "http://www.aiim.org/pdfa/ns/id/";
    constexpr const char* PDFX = "http://ns.adobe.com/pdfx/1.3/";
    constexpr const char* XMPRights = "http://ns.adobe.com/xap/1.0/rights/";
    constexpr const char* EXIF = "http://ns.adobe.com/exif/1.0/";
}

// XMP property structure
struct XMPProperty {
    std::string namespace_uri;
    std::string name;
    std::string value;
    std::string lang;  // Language tag (e.g., "en-US")
    
    XMPProperty() = default;
    XMPProperty(const std::string& ns, const std::string& n, const std::string& v)
        : namespace_uri(ns), name(n), value(v) {}
};

// XMP metadata container
struct XMPMetadata {
    std::vector<XMPProperty> properties;
    std::string raw_xml;  // Raw XMP packet
    
    // Helper to get property value
    std::string get_value(const std::string& namespace_uri, const std::string& name) const;
    
    // Helper to set property value
    void set_value(const std::string& namespace_uri, const std::string& name, const std::string& value);
    
    // Check if property exists
    bool has_property(const std::string& namespace_uri, const std::string& name) const;
};

// Metadata management class
class PDFEDITOR_API Metadata {
public:
    // ===== Info Dictionary (Legacy) =====
    
    // Read entire Info dictionary
    static MetadataMap read_info(Document* doc);
    
    // Write entire Info dictionary
    static bool write_info(Document* doc, const MetadataMap& info);
    
    // Get individual Info fields
    static std::string get_title(Document* doc);
    static std::string get_author(Document* doc);
    static std::string get_subject(Document* doc);
    static std::string get_keywords(Document* doc);
    static std::string get_creator(Document* doc);
    static std::string get_producer(Document* doc);
    static std::string get_creation_date(Document* doc);
    static std::string get_modification_date(Document* doc);
    
    // Set individual Info fields
    static bool set_title(Document* doc, const std::string& title);
    static bool set_author(Document* doc, const std::string& author);
    static bool set_subject(Document* doc, const std::string& subject);
    static bool set_keywords(Document* doc, const std::string& keywords);
    static bool set_creator(Document* doc, const std::string& creator);
    static bool set_producer(Document* doc, const std::string& producer);
    
    // Update modification date to current time
    static bool update_modification_date(Document* doc);
    
    // ===== XMP Metadata =====
    
    // Check if document has XMP metadata
    static bool has_xmp(Document* doc);
    
    // Read XMP metadata packet (raw XML)
    static std::string read_xmp_raw(Document* doc);
    
    // Write XMP metadata packet (raw XML)
    static bool write_xmp_raw(Document* doc, const std::string& xmp_xml);
    
    // Read parsed XMP metadata
    static XMPMetadata read_xmp(Document* doc);
    
    // Write parsed XMP metadata
    static bool write_xmp(Document* doc, const XMPMetadata& xmp);
    
    // Get XMP property
    static std::string get_xmp_property(
        Document* doc,
        const std::string& namespace_uri,
        const std::string& property_name
    );
    
    // Set XMP property
    static bool set_xmp_property(
        Document* doc,
        const std::string& namespace_uri,
        const std::string& property_name,
        const std::string& value
    );
    
    // Remove XMP property
    static bool remove_xmp_property(
        Document* doc,
        const std::string& namespace_uri,
        const std::string& property_name
    );
    
    // ===== Synchronization =====
    
    // Sync Info dictionary and XMP (ensure consistency)
    // prefer_xmp: if true, XMP values override Info; otherwise Info overrides XMP
    static bool sync_info_xmp(Document* doc, bool prefer_xmp = true);
    
    // Copy Info dictionary to XMP
    static bool copy_info_to_xmp(Document* doc);
    
    // Copy XMP to Info dictionary
    static bool copy_xmp_to_info(Document* doc);
    
    // ===== Custom Metadata =====
    
    // Add custom metadata field (in Info dictionary)
    static bool add_custom_field(
        Document* doc,
        const std::string& key,
        const std::string& value
    );
    
    // Get custom metadata field
    static std::string get_custom_field(Document* doc, const std::string& key);
    
    // Remove custom metadata field
    static bool remove_custom_field(Document* doc, const std::string& key);
    
    // List all custom fields (excluding standard keys)
    static std::vector<std::string> list_custom_fields(Document* doc);
    
    // ===== Sanitization =====
    
    // Remove all metadata (for redaction/privacy)
    static bool sanitize(Document* doc);
    
    // Remove only XMP metadata
    static bool sanitize_xmp(Document* doc);
    
    // Remove only Info dictionary
    static bool sanitize_info(Document* doc);
    
    // Remove specific metadata fields
    static bool sanitize_fields(
        Document* doc,
        const std::vector<std::string>& fields_to_remove
    );
    
    // ===== Validation =====
    
    // Validate XMP packet structure
    static bool validate_xmp(Document* doc);
    
    // Get XMP validation errors
    static std::vector<std::string> get_xmp_errors(Document* doc);
    
    // Repair malformed XMP
    static bool repair_xmp(Document* doc);
    
    // ===== Import/Export =====
    
    // Export metadata to JSON
    static std::string export_json(Document* doc);
    
    // Import metadata from JSON
    static bool import_json(Document* doc, const std::string& json);
    
    // Export XMP to file
    static bool export_xmp(Document* doc, const std::string& file_path);
    
    // Import XMP from file
    static bool import_xmp(Document* doc, const std::string& file_path);
    
    // ===== PDF/A Metadata =====
    
    // Check if document has PDF/A metadata
    static bool has_pdfa_metadata(Document* doc);
    
    // Get PDF/A conformance level
    static std::string get_pdfa_conformance(Document* doc);
    
    // Set PDF/A conformance level
    static bool set_pdfa_conformance(
        Document* doc,
        const std::string& part,      // e.g., "1", "2", "3"
        const std::string& conformance // e.g., "B", "A", "U"
    );
    
    // ===== Utility Functions =====
    
    // Format date string to PDF date format (D:YYYYMMDDHHmmSSOHH'mm')
    static std::string format_pdf_date(const std::string& iso_date);
    
    // Parse PDF date format to ISO 8601
    static std::string parse_pdf_date(const std::string& pdf_date);
    
    // Get current date/time in PDF format
    static std::string get_current_pdf_date();
    
    // Generate XMP UUID
    static std::string generate_xmp_uuid();
    
    // Create minimal XMP packet
    static std::string create_minimal_xmp(
        const std::string& title = "",
        const std::string& author = "",
        const std::string& subject = ""
    );
};

// XMP Schema registry for custom schemas
class PDFEDITOR_API XMPSchemaRegistry {
public:
    // Register custom XMP namespace
    static bool register_namespace(
        const std::string& namespace_uri,
        const std::string& prefix
    );
    
    // Unregister namespace
    static bool unregister_namespace(const std::string& namespace_uri);
    
    // Get prefix for namespace
    static std::string get_prefix(const std::string& namespace_uri);
    
    // Get namespace for prefix
    static std::string get_namespace(const std::string& prefix);
    
    // Check if namespace is registered
    static bool is_registered(const std::string& namespace_uri);
    
    // List all registered namespaces
    static std::map<std::string, std::string> list_namespaces();
};

} // namespace pdfeditor
