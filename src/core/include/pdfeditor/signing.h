#pragma once

#include "core.h"
#include "document.h"
#include <string>
#include <vector>
#include <ctime>

namespace pdfeditor {

// Signature type
enum class SignatureType {
    Digital,      // Digital signature (cryptographic)
    Visual,       // Visual signature (image only)
    Both          // Both digital and visual
};

// Signature standard
enum class SignatureStandard {
    PKCS7,        // PKCS#7 (CMS)
    PAdES,        // PAdES (PDF Advanced Electronic Signatures)
    CAdES         // CAdES (CMS Advanced Electronic Signatures)
};

// Hash algorithm
enum class HashAlgorithm {
    SHA1,
    SHA256,
    SHA384,
    SHA512
};

// Signature appearance
struct SignatureAppearance {
    Rect rect;
    int page_index;
    std::string name;
    std::string reason;
    std::string location;
    std::string contact_info;
    std::string image_path;     // Custom signature image
    bool show_labels;
    bool show_date;
    bool show_logo;
    Color background_color;
    Color text_color;
    
    SignatureAppearance()
        : page_index(0)
        , show_labels(true)
        , show_date(true)
        , show_logo(false)
        , background_color(Color::white())
        , text_color(Color::black()) {}
};

// Certificate info
struct CertificateInfo {
    std::string subject;
    std::string issuer;
    std::string serial_number;
    std::string valid_from;
    std::string valid_to;
    std::string key_usage;
    std::string email;
    std::string organization;
    std::string common_name;
    int key_size_bits;
    bool is_valid;
    bool is_self_signed;
    std::string fingerprint_sha1;
    std::string fingerprint_sha256;
};

// Signature info
struct SignatureInfo {
    std::string field_name;
    SignatureType type;
    SignatureStandard standard;
    bool is_signed;
    bool is_valid;
    std::string signer_name;
    std::string signing_time;
    std::string reason;
    std::string location;
    std::string contact_info;
    CertificateInfo certificate;
    HashAlgorithm hash_algorithm;
    bool covers_whole_document;
    std::vector<std::string> byte_ranges;  // Signed byte ranges
    std::vector<std::string> validation_errors;
    
    // Timestamp info
    bool has_timestamp;
    std::string timestamp_time;
    std::string timestamp_authority;
};

// Signing options
struct SigningOptions {
    SignatureType type;
    SignatureStandard standard;
    HashAlgorithm hash_algorithm;
    std::string certificate_path;
    std::string private_key_path;
    std::string password;              // For encrypted private key
    std::string reason;
    std::string location;
    std::string contact_info;
    bool add_timestamp;
    std::string timestamp_server_url;
    bool lock_document_after_signing;
    SignatureAppearance appearance;
    
    SigningOptions()
        : type(SignatureType::Both)
        , standard(SignatureStandard::PAdES)
        , hash_algorithm(HashAlgorithm::SHA256)
        , add_timestamp(false)
        , lock_document_after_signing(false) {}
};

// Signature validation result
struct ValidationResult {
    bool is_valid;
    bool certificate_valid;
    bool signature_intact;
    bool document_unmodified;
    bool timestamp_valid;
    bool trusted_certificate;
    std::string validation_time;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    CertificateInfo certificate;
};

// Digital signing class
class PDFEDITOR_API Signing {
public:
    // ===== Signing =====
    
    // Sign document
    static bool sign(
        Document* doc,
        const SigningOptions& options
    );
    
    // Sign with PKCS#12 file (contains both certificate and private key)
    static bool sign_with_p12(
        Document* doc,
        const std::string& p12_path,
        const std::string& password,
        const SignatureAppearance& appearance = SignatureAppearance()
    );
    
    // Sign with smartcard/token
    static bool sign_with_token(
        Document* doc,
        const std::string& token_name,
        const std::string& pin,
        const SignatureAppearance& appearance = SignatureAppearance()
    );
    
    // Add signature field (empty, to be signed later)
    static bool add_signature_field(
        Document* doc,
        const std::string& field_name,
        int page_index,
        const Rect& rect
    );
    
    // Sign existing signature field
    static bool sign_field(
        Document* doc,
        const std::string& field_name,
        const SigningOptions& options
    );
    
    // ===== Multiple Signatures =====
    
    // Add additional signature (incremental update)
    static bool add_signature(
        Document* doc,
        const SigningOptions& options
    );
    
    // Get all signatures
    static std::vector<SignatureInfo> get_signatures(Document* doc);
    
    // Get signature by field name
    static Result<SignatureInfo> get_signature(
        Document* doc,
        const std::string& field_name
    );
    
    // Count signatures
    static int count_signatures(Document* doc);
    
    // Remove signature
    static bool remove_signature(
        Document* doc,
        const std::string& field_name
    );
    
    // Clear all signatures
    static bool clear_signatures(Document* doc);
    
    // ===== Validation =====
    
    // Validate signature
    static ValidationResult validate_signature(
        Document* doc,
        const std::string& field_name
    );
    
    // Validate all signatures
    static std::vector<ValidationResult> validate_all_signatures(Document* doc);
    
    // Quick validation check
    static bool is_signature_valid(
        Document* doc,
        const std::string& field_name
    );
    
    // Check if document has been modified after signing
    static bool is_document_modified(
        Document* doc,
        const std::string& field_name
    );
    
    // ===== Certificate Management =====
    
    // Load certificate from file
    static Result<CertificateInfo> load_certificate(
        const std::string& cert_path
    );
    
    // Load certificate from PKCS#12
    static Result<CertificateInfo> load_certificate_from_p12(
        const std::string& p12_path,
        const std::string& password
    );
    
    // Get certificate from signature
    static Result<CertificateInfo> get_signature_certificate(
        Document* doc,
        const std::string& field_name
    );
    
    // Validate certificate
    static bool validate_certificate(const CertificateInfo& cert);
    
    // Check certificate expiration
    static bool is_certificate_expired(const CertificateInfo& cert);
    
    // Get certificate chain
    static std::vector<CertificateInfo> get_certificate_chain(
        const CertificateInfo& cert
    );
    
    // ===== Trust Management =====
    
    // Add trusted certificate
    static bool add_trusted_certificate(const std::string& cert_path);
    
    // Remove trusted certificate
    static bool remove_trusted_certificate(const std::string& fingerprint);
    
    // List trusted certificates
    static std::vector<CertificateInfo> list_trusted_certificates();
    
    // Check if certificate is trusted
    static bool is_certificate_trusted(const CertificateInfo& cert);
    
    // ===== Timestamp =====
    
    // Add timestamp to signature
    static bool add_timestamp(
        Document* doc,
        const std::string& field_name,
        const std::string& timestamp_server_url
    );
    
    // Validate timestamp
    static bool validate_timestamp(
        Document* doc,
        const std::string& field_name
    );
    
    // Get timestamp info
    struct TimestampInfo {
        bool present;
        std::string time;
        std::string authority;
        bool valid;
        CertificateInfo tsa_certificate;
    };
    
    static TimestampInfo get_timestamp_info(
        Document* doc,
        const std::string& field_name
    );
    
    // ===== Visual Signatures =====
    
    // Add visual signature (image only, no cryptographic signature)
    static bool add_visual_signature(
        Document* doc,
        const SignatureAppearance& appearance
    );
    
    // Update signature appearance
    static bool update_appearance(
        Document* doc,
        const std::string& field_name,
        const SignatureAppearance& appearance
    );
    
    // ===== Signature Locks =====
    
    // Lock document after signing
    static bool lock_document(
        Document* doc,
        const std::string& field_name
    );
    
    // Lock specific fields
    static bool lock_fields(
        Document* doc,
        const std::string& field_name,
        const std::vector<std::string>& fields_to_lock
    );
    
    // Check if document is locked
    static bool is_document_locked(Document* doc);
    
    // ===== Long-Term Validation (LTV) =====
    
    // Enable LTV (embed revocation info)
    static bool enable_ltv(Document* doc);
    
    // Add validation data (OCSP, CRL)
    static bool add_validation_data(
        Document* doc,
        const std::string& field_name
    );
    
    // Verify LTV signature
    static bool verify_ltv(
        Document* doc,
        const std::string& field_name
    );
    
    // ===== Batch Signing =====
    
    // Sign multiple documents
    struct BatchSigningJob {
        std::string input_path;
        std::string output_path;
        SigningOptions options;
    };
    
    static std::vector<bool> batch_sign(
        const std::vector<BatchSigningJob>& jobs,
        ProgressCallback callback = nullptr
    );
    
    // ===== Utilities =====
    
    // Generate self-signed certificate
    static bool generate_self_signed_certificate(
        const std::string& output_p12_path,
        const std::string& password,
        const std::string& common_name,
        const std::string& organization,
        const std::string& email,
        int validity_days = 365
    );
    
    // Export signature data
    static std::vector<uint8_t> export_signature_data(
        Document* doc,
        const std::string& field_name
    );
    
    // Get signature hash
    static std::string get_signature_hash(
        Document* doc,
        const std::string& field_name
    );
    
    // Verify hash
    static bool verify_hash(
        const std::string& hash,
        const std::vector<uint8_t>& signature,
        const CertificateInfo& cert
    );
};

} // namespace pdfeditor
