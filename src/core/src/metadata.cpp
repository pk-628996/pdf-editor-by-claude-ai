#include "pdfeditor/metadata.h"
#include "pdfeditor/core.h"
#include <sstream>
#include <algorithm>
#include <ctime>
#include <iomanip>

#ifdef USE_MUPDF
#include <mupdf/fitz.h>
#endif

namespace pdfeditor {

namespace {
    // Helper to get PDF object as string
    std::string pdf_obj_to_string(void* ctx, void* obj) {
#ifdef USE_MUPDF
        if (!obj) return "";
        pdf_obj* pdf_o = static_cast<pdf_obj*>(obj);
        const char* str = pdf_to_text_string(static_cast<fz_context*>(ctx), pdf_o);
        return str ? str : "";
#else
        return "";
#endif
    }
    
    // Format current time as PDF date string
    std::string current_pdf_date() {
        std::time_t t = std::time(nullptr);
        std::tm* tm = std::localtime(&t);
        
        std::ostringstream oss;
        oss << "D:"
            << std::setfill('0')
            << std::setw(4) << (tm->tm_year + 1900)
            << std::setw(2) << (tm->tm_mon + 1)
            << std::setw(2) << tm->tm_mday
            << std::setw(2) << tm->tm_hour
            << std::setw(2) << tm->tm_min
            << std::setw(2) << tm->tm_sec;
        
        return oss.str();
    }
}

// XMPMetadata helper implementations
std::string XMPMetadata::get_value(
    const std::string& namespace_uri,
    const std::string& name
) const {
    for (const auto& prop : properties) {
        if (prop.namespace_uri == namespace_uri && prop.name == name) {
            return prop.value;
        }
    }
    return "";
}

void XMPMetadata::set_value(
    const std::string& namespace_uri,
    const std::string& name,
    const std::string& value
) {
    // Find existing property
    for (auto& prop : properties) {
        if (prop.namespace_uri == namespace_uri && prop.name == name) {
            prop.value = value;
            return;
        }
    }
    
    // Add new property
    properties.push_back(XMPProperty(namespace_uri, name, value));
}

bool XMPMetadata::has_property(
    const std::string& namespace_uri,
    const std::string& name
) const {
    for (const auto& prop : properties) {
        if (prop.namespace_uri == namespace_uri && prop.name == name) {
            return true;
        }
    }
    return false;
}

// Metadata class implementation
MetadataMap Metadata::read_info(Document* doc) {
    if (!doc) return {};
    
    MetadataMap info;
    
#ifdef USE_MUPDF
    void* handle = doc->get_handle();
    if (!handle) return {};
    
    fz_document* fz_doc = static_cast<fz_document*>(handle);
    fz_context* ctx = nullptr; // TODO: Get context from document
    
    if (!ctx) return {};
    
    pdf_document* pdf = pdf_specifics(ctx, fz_doc);
    if (!pdf) return {};
    
    fz_try(ctx) {
        pdf_obj* info_dict = pdf_dict_get(ctx, pdf_trailer(ctx, pdf), PDF_NAME(Info));
        if (info_dict) {
            // Read standard fields
            auto read_field = [&](const char* key) -> std::string {
                pdf_obj* obj = pdf_dict_gets(ctx, info_dict, key);
                return pdf_obj_to_string(ctx, obj);
            };
            
            info[InfoKeys::Title] = read_field("Title");
            info[InfoKeys::Author] = read_field("Author");
            info[InfoKeys::Subject] = read_field("Subject");
            info[InfoKeys::Keywords] = read_field("Keywords");
            info[InfoKeys::Creator] = read_field("Creator");
            info[InfoKeys::Producer] = read_field("Producer");
            info[InfoKeys::CreationDate] = read_field("CreationDate");
            info[InfoKeys::ModDate] = read_field("ModDate");
            
            // Read custom fields
            int n = pdf_dict_len(ctx, info_dict);
            for (int i = 0; i < n; ++i) {
                pdf_obj* key = pdf_dict_get_key(ctx, info_dict, i);
                const char* key_str = pdf_to_name(ctx, key);
                
                // Skip standard keys
                if (std::string(key_str) != "Title" &&
                    std::string(key_str) != "Author" &&
                    std::string(key_str) != "Subject" &&
                    std::string(key_str) != "Keywords" &&
                    std::string(key_str) != "Creator" &&
                    std::string(key_str) != "Producer" &&
                    std::string(key_str) != "CreationDate" &&
                    std::string(key_str) != "ModDate") {
                    
                    pdf_obj* val = pdf_dict_get_val(ctx, info_dict, i);
                    info[key_str] = pdf_obj_to_string(ctx, val);
                }
            }
        }
    }
    fz_catch(ctx) {
        // Error reading info
    }
#endif
    
    return info;
}

bool Metadata::write_info(Document* doc, const MetadataMap& info) {
    if (!doc) return false;
    
#ifdef USE_MUPDF
    void* handle = doc->get_handle();
    if (!handle) return false;
    
    fz_document* fz_doc = static_cast<fz_document*>(handle);
    fz_context* ctx = nullptr; // TODO: Get context
    
    if (!ctx) return false;
    
    pdf_document* pdf = pdf_specifics(ctx, fz_doc);
    if (!pdf) return false;
    
    fz_try(ctx) {
        pdf_obj* trailer = pdf_trailer(ctx, pdf);
        pdf_obj* info_dict = pdf_dict_get(ctx, trailer, PDF_NAME(Info));
        
        if (!info_dict) {
            // Create new info dictionary
            info_dict = pdf_new_dict(ctx, pdf, 8);
            pdf_dict_put(ctx, trailer, PDF_NAME(Info), info_dict);
        }
        
        // Write all fields
        for (const auto& [key, value] : info) {
            pdf_obj* key_obj = pdf_new_name(ctx, key.c_str());
            pdf_obj* val_obj = pdf_new_text_string(ctx, value.c_str());
            pdf_dict_put(ctx, info_dict, key_obj, val_obj);
            pdf_drop_obj(ctx, key_obj);
            pdf_drop_obj(ctx, val_obj);
        }
    }
    fz_catch(ctx) {
        return false;
    }
    
    return true;
#else
    return false;
#endif
}

std::string Metadata::get_title(Document* doc) {
    auto info = read_info(doc);
    return info[InfoKeys::Title];
}

std::string Metadata::get_author(Document* doc) {
    auto info = read_info(doc);
    return info[InfoKeys::Author];
}

std::string Metadata::get_subject(Document* doc) {
    auto info = read_info(doc);
    return info[InfoKeys::Subject];
}

std::string Metadata::get_keywords(Document* doc) {
    auto info = read_info(doc);
    return info[InfoKeys::Keywords];
}

std::string Metadata::get_creator(Document* doc) {
    auto info = read_info(doc);
    return info[InfoKeys::Creator];
}

std::string Metadata::get_producer(Document* doc) {
    auto info = read_info(doc);
    return info[InfoKeys::Producer];
}

std::string Metadata::get_creation_date(Document* doc) {
    auto info = read_info(doc);
    return info[InfoKeys::CreationDate];
}

std::string Metadata::get_modification_date(Document* doc) {
    auto info = read_info(doc);
    return info[InfoKeys::ModDate];
}

bool Metadata::set_title(Document* doc, const std::string& title) {
    auto info = read_info(doc);
    info[InfoKeys::Title] = title;
    return write_info(doc, info);
}

bool Metadata::set_author(Document* doc, const std::string& author) {
    auto info = read_info(doc);
    info[InfoKeys::Author] = author;
    return write_info(doc, info);
}

bool Metadata::set_subject(Document* doc, const std::string& subject) {
    auto info = read_info(doc);
    info[InfoKeys::Subject] = subject;
    return write_info(doc, info);
}

bool Metadata::set_keywords(Document* doc, const std::string& keywords) {
    auto info = read_info(doc);
    info[InfoKeys::Keywords] = keywords;
    return write_info(doc, info);
}

bool Metadata::set_creator(Document* doc, const std::string& creator) {
    auto info = read_info(doc);
    info[InfoKeys::Creator] = creator;
    return write_info(doc, info);
}

bool Metadata::set_producer(Document* doc, const std::string& producer) {
    auto info = read_info(doc);
    info[InfoKeys::Producer] = producer;
    return write_info(doc, info);
}

bool Metadata::update_modification_date(Document* doc) {
    auto info = read_info(doc);
    info[InfoKeys::ModDate] = current_pdf_date();
    return write_info(doc, info);
}

bool Metadata::has_xmp(Document* doc) {
    if (!doc) return false;
    
#ifdef USE_MUPDF
    void* handle = doc->get_handle();
    if (!handle) return false;
    
    fz_document* fz_doc = static_cast<fz_document*>(handle);
    fz_context* ctx = nullptr; // TODO: Get context
    
    if (!ctx) return false;
    
    pdf_document* pdf = pdf_specifics(ctx, fz_doc);
    if (!pdf) return false;
    
    bool has_metadata = false;
    fz_try(ctx) {
        pdf_obj* catalog = pdf_dict_get(ctx, pdf_trailer(ctx, pdf), PDF_NAME(Root));
        pdf_obj* metadata = pdf_dict_get(ctx, catalog, PDF_NAME(Metadata));
        has_metadata = (metadata != nullptr);
    }
    fz_catch(ctx) {
        has_metadata = false;
    }
    
    return has_metadata;
#else
    return false;
#endif
}

std::string Metadata::read_xmp_raw(Document* doc) {
    if (!doc) return "";
    
#ifdef USE_MUPDF
    void* handle = doc->get_handle();
    if (!handle) return "";
    
    fz_document* fz_doc = static_cast<fz_document*>(handle);
    fz_context* ctx = nullptr; // TODO: Get context
    
    if (!ctx) return "";
    
    pdf_document* pdf = pdf_specifics(ctx, fz_doc);
    if (!pdf) return "";
    
    std::string xmp;
    fz_try(ctx) {
        pdf_obj* catalog = pdf_dict_get(ctx, pdf_trailer(ctx, pdf), PDF_NAME(Root));
        pdf_obj* metadata = pdf_dict_get(ctx, catalog, PDF_NAME(Metadata));
        
        if (metadata) {
            fz_buffer* buf = pdf_load_stream(ctx, metadata);
            size_t len = fz_buffer_storage(ctx, buf, nullptr);
            const unsigned char* data = fz_buffer_storage(ctx, buf, nullptr);
            xmp = std::string(reinterpret_cast<const char*>(data), len);
            fz_drop_buffer(ctx, buf);
        }
    }
    fz_catch(ctx) {
        // Error reading XMP
    }
    
    return xmp;
#else
    return "";
#endif
}

bool Metadata::write_xmp_raw(Document* doc, const std::string& xmp_xml) {
    if (!doc) return false;
    
#ifdef USE_MUPDF
    void* handle = doc->get_handle();
    if (!handle) return false;
    
    fz_document* fz_doc = static_cast<fz_document*>(handle);
    fz_context* ctx = nullptr; // TODO: Get context
    
    if (!ctx) return false;
    
    pdf_document* pdf = pdf_specifics(ctx, fz_doc);
    if (!pdf) return false;
    
    fz_try(ctx) {
        pdf_obj* catalog = pdf_dict_get(ctx, pdf_trailer(ctx, pdf), PDF_NAME(Root));
        
        // Create metadata stream
        fz_buffer* buf = fz_new_buffer_from_copied_data(
            ctx,
            reinterpret_cast<const unsigned char*>(xmp_xml.c_str()),
            xmp_xml.length()
        );
        
        pdf_obj* metadata = pdf_add_stream(ctx, pdf, buf, nullptr, 0);
        pdf_dict_put(ctx, metadata, PDF_NAME(Type), PDF_NAME(Metadata));
        pdf_dict_put(ctx, metadata, PDF_NAME(Subtype), PDF_NAME(XML));
        
        pdf_dict_put(ctx, catalog, PDF_NAME(Metadata), metadata);
        
        fz_drop_buffer(ctx, buf);
    }
    fz_catch(ctx) {
        return false;
    }
    
    return true;
#else
    return false;
#endif
}

XMPMetadata Metadata::read_xmp(Document* doc) {
    XMPMetadata xmp;
    xmp.raw_xml = read_xmp_raw(doc);
    
    // TODO: Parse XMP XML and populate properties
    // For now, just store raw XML
    
    return xmp;
}

bool Metadata::write_xmp(Document* doc, const XMPMetadata& xmp) {
    // TODO: Generate XMP XML from properties
    return write_xmp_raw(doc, xmp.raw_xml);
}

std::string Metadata::get_xmp_property(
    Document* doc,
    const std::string& namespace_uri,
    const std::string& property_name
) {
    auto xmp = read_xmp(doc);
    return xmp.get_value(namespace_uri, property_name);
}

bool Metadata::set_xmp_property(
    Document* doc,
    const std::string& namespace_uri,
    const std::string& property_name,
    const std::string& value
) {
    auto xmp = read_xmp(doc);
    xmp.set_value(namespace_uri, property_name, value);
    return write_xmp(doc, xmp);
}

bool Metadata::remove_xmp_property(
    Document* doc,
    const std::string& namespace_uri,
    const std::string& property_name
) {
    auto xmp = read_xmp(doc);
    
    auto it = std::remove_if(xmp.properties.begin(), xmp.properties.end(),
        [&](const XMPProperty& prop) {
            return prop.namespace_uri == namespace_uri && 
                   prop.name == property_name;
        });
    
    xmp.properties.erase(it, xmp.properties.end());
    
    return write_xmp(doc, xmp);
}

bool Metadata::sync_info_xmp(Document* doc, bool prefer_xmp) {
    auto info = read_info(doc);
    auto xmp = read_xmp(doc);
    
    if (prefer_xmp) {
        // Copy XMP to Info
        info[InfoKeys::Title] = xmp.get_value(XMPNamespaces::DC, "title");
        info[InfoKeys::Author] = xmp.get_value(XMPNamespaces::DC, "creator");
        info[InfoKeys::Subject] = xmp.get_value(XMPNamespaces::DC, "description");
        info[InfoKeys::Keywords] = xmp.get_value(XMPNamespaces::DC, "subject");
        
        return write_info(doc, info);
    } else {
        // Copy Info to XMP
        xmp.set_value(XMPNamespaces::DC, "title", info[InfoKeys::Title]);
        xmp.set_value(XMPNamespaces::DC, "creator", info[InfoKeys::Author]);
        xmp.set_value(XMPNamespaces::DC, "description", info[InfoKeys::Subject]);
        xmp.set_value(XMPNamespaces::DC, "subject", info[InfoKeys::Keywords]);
        
        return write_xmp(doc, xmp);
    }
}

bool Metadata::copy_info_to_xmp(Document* doc) {
    return sync_info_xmp(doc, false);
}

bool Metadata::copy_xmp_to_info(Document* doc) {
    return sync_info_xmp(doc, true);
}

bool Metadata::add_custom_field(
    Document* doc,
    const std::string& key,
    const std::string& value
) {
    auto info = read_info(doc);
    info[key] = value;
    return write_info(doc, info);
}

std::string Metadata::get_custom_field(Document* doc, const std::string& key) {
    auto info = read_info(doc);
    return info[key];
}

bool Metadata::remove_custom_field(Document* doc, const std::string& key) {
    auto info = read_info(doc);
    info.erase(key);
    return write_info(doc, info);
}

std::vector<std::string> Metadata::list_custom_fields(Document* doc) {
    auto info = read_info(doc);
    std::vector<std::string> custom;
    
    for (const auto& [key, value] : info) {
        // Skip standard keys
        if (key != InfoKeys::Title &&
            key != InfoKeys::Author &&
            key != InfoKeys::Subject &&
            key != InfoKeys::Keywords &&
            key != InfoKeys::Creator &&
            key != InfoKeys::Producer &&
            key != InfoKeys::CreationDate &&
            key != InfoKeys::ModDate &&
            key != InfoKeys::Trapped) {
            custom.push_back(key);
        }
    }
    
    return custom;
}

bool Metadata::sanitize(Document* doc) {
    return sanitize_info(doc) && sanitize_xmp(doc);
}

bool Metadata::sanitize_xmp(Document* doc) {
    if (!doc) return false;
    
#ifdef USE_MUPDF
    void* handle = doc->get_handle();
    if (!handle) return false;
    
    fz_document* fz_doc = static_cast<fz_document*>(handle);
    fz_context* ctx = nullptr; // TODO: Get context
    
    if (!ctx) return false;
    
    pdf_document* pdf = pdf_specifics(ctx, fz_doc);
    if (!pdf) return false;
    
    fz_try(ctx) {
        pdf_obj* catalog = pdf_dict_get(ctx, pdf_trailer(ctx, pdf), PDF_NAME(Root));
        pdf_dict_del(ctx, catalog, PDF_NAME(Metadata));
    }
    fz_catch(ctx) {
        return false;
    }
    
    return true;
#else
    return false;
#endif
}

bool Metadata::sanitize_info(Document* doc) {
    if (!doc) return false;
    
#ifdef USE_MUPDF
    void* handle = doc->get_handle();
    if (!handle) return false;
    
    fz_document* fz_doc = static_cast<fz_document*>(handle);
    fz_context* ctx = nullptr; // TODO: Get context
    
    if (!ctx) return false;
    
    pdf_document* pdf = pdf_specifics(ctx, fz_doc);
    if (!pdf) return false;
    
    fz_try(ctx) {
        pdf_obj* trailer = pdf_trailer(ctx, pdf);
        pdf_dict_del(ctx, trailer, PDF_NAME(Info));
    }
    fz_catch(ctx) {
        return false;
    }
    
    return true;
#else
    return false;
#endif
}

bool Metadata::sanitize_fields(
    Document* doc,
    const std::vector<std::string>& fields_to_remove
) {
    auto info = read_info(doc);
    
    for (const auto& field : fields_to_remove) {
        info.erase(field);
    }
    
    return write_info(doc, info);
}

bool Metadata::validate_xmp(Document* doc) {
    // TODO: Validate XMP XML structure
    return true;
}

std::vector<std::string> Metadata::get_xmp_errors(Document* doc) {
    // TODO: Parse XMP and collect errors
    return {};
}

bool Metadata::repair_xmp(Document* doc) {
    // TODO: Attempt to repair malformed XMP
    return true;
}

std::string Metadata::export_json(Document* doc) {
    auto info = read_info(doc);
    
    std::ostringstream json;
    json << "{\n";
    
    bool first = true;
    for (const auto& [key, value] : info) {
        if (!first) json << ",\n";
        json << "  \"" << key << "\": \"" << value << "\"";
        first = false;
    }
    
    json << "\n}\n";
    return json.str();
}

bool Metadata::import_json(Document* doc, const std::string& json) {
    // TODO: Parse JSON and import metadata
    return true;
}

bool Metadata::export_xmp(Document* doc, const std::string& file_path) {
    std::string xmp = read_xmp_raw(doc);
    
    std::ofstream file(file_path);
    if (!file) return false;
    
    file << xmp;
    return true;
}

bool Metadata::import_xmp(Document* doc, const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file) return false;
    
    std::string xmp((std::istreambuf_iterator<char>(file)),
                    std::istreambuf_iterator<char>());
    
    return write_xmp_raw(doc, xmp);
}

bool Metadata::has_pdfa_metadata(Document* doc) {
    auto xmp = read_xmp(doc);
    return xmp.has_property(XMPNamespaces::PDFAID, "part");
}

std::string Metadata::get_pdfa_conformance(Document* doc) {
    auto xmp = read_xmp(doc);
    std::string part = xmp.get_value(XMPNamespaces::PDFAID, "part");
    std::string conformance = xmp.get_value(XMPNamespaces::PDFAID, "conformance");
    
    if (!part.empty() && !conformance.empty()) {
        return "PDF/A-" + part + conformance;
    }
    
    return "";
}

bool Metadata::set_pdfa_conformance(
    Document* doc,
    const std::string& part,
    const std::string& conformance
) {
    auto xmp = read_xmp(doc);
    xmp.set_value(XMPNamespaces::PDFAID, "part", part);
    xmp.set_value(XMPNamespaces::PDFAID, "conformance", conformance);
    
    return write_xmp(doc, xmp);
}

std::string Metadata::format_pdf_date(const std::string& iso_date) {
    // Simple conversion - in production use proper date parsing
    return "D:" + iso_date;
}

std::string Metadata::parse_pdf_date(const std::string& pdf_date) {
    // Simple conversion - in production use proper date parsing
    if (pdf_date.length() > 2 && pdf_date.substr(0, 2) == "D:") {
        return pdf_date.substr(2);
    }
    return pdf_date;
}

std::string Metadata::get_current_pdf_date() {
    return current_pdf_date();
}

std::string Metadata::generate_xmp_uuid() {
    return util::generate_uuid();
}

std::string Metadata::create_minimal_xmp(
    const std::string& title,
    const std::string& author,
    const std::string& subject
) {
    std::ostringstream xmp;
    
    xmp << "<?xpacket begin='' id='W5M0MpCehiHzreSzNTczkc9d'?>\n";
    xmp << "<x:xmpmeta xmlns:x='adobe:ns:meta/'>\n";
    xmp << "  <rdf:RDF xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#'>\n";
    xmp << "    <rdf:Description rdf:about=''\n";
    xmp << "        xmlns:dc='http://purl.org/dc/elements/1.1/'>\n";
    
    if (!title.empty()) {
        xmp << "      <dc:title><rdf:Alt><rdf:li xml:lang='x-default'>"
            << title << "</rdf:li></rdf:Alt></dc:title>\n";
    }
    
    if (!author.empty()) {
        xmp << "      <dc:creator><rdf:Seq><rdf:li>"
            << author << "</rdf:li></rdf:Seq></dc:creator>\n";
    }
    
    if (!subject.empty()) {
        xmp << "      <dc:description><rdf:Alt><rdf:li xml:lang='x-default'>"
            << subject << "</rdf:li></rdf:Alt></dc:description>\n";
    }
    
    xmp << "    </rdf:Description>\n";
    xmp << "  </rdf:RDF>\n";
    xmp << "</x:xmpmeta>\n";
    xmp << "<?xpacket end='w'?>\n";
    
    return xmp.str();
}

// XMP Schema Registry implementation
namespace {
    std::map<std::string, std::string> schema_registry = {
        {XMPNamespaces::DC, "dc"},
        {XMPNamespaces::XMP, "xmp"},
        {XMPNamespaces::PDF, "pdf"},
        {XMPNamespaces::PDFAID, "pdfaid"},
        {XMPNamespaces::PDFX, "pdfx"},
        {XMPNamespaces::XMPRights, "xmpRights"},
        {XMPNamespaces::EXIF, "exif"}
    };
}

bool XMPSchemaRegistry::register_namespace(
    const std::string& namespace_uri,
    const std::string& prefix
) {
    schema_registry[namespace_uri] = prefix;
    return true;
}

bool XMPSchemaRegistry::unregister_namespace(const std::string& namespace_uri) {
    schema_registry.erase(namespace_uri);
    return true;
}

std::string XMPSchemaRegistry::get_prefix(const std::string& namespace_uri) {
    auto it = schema_registry.find(namespace_uri);
    return (it != schema_registry.end()) ? it->second : "";
}

std::string XMPSchemaRegistry::get_namespace(const std::string& prefix) {
    for (const auto& [uri, pfx] : schema_registry) {
        if (pfx == prefix) return uri;
    }
    return "";
}

bool XMPSchemaRegistry::is_registered(const std::string& namespace_uri) {
    return schema_registry.find(namespace_uri) != schema_registry.end();
}

std::map<std::string, std::string> XMPSchemaRegistry::list_namespaces() {
    return schema_registry;
}

} // namespace pdfeditor
