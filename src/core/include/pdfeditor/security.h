#pragma once

#include "core.h"
#include "document.h"
#include <string>
#include <vector>

namespace pdfeditor {

// Encryption algorithm
enum class EncryptionAlgorithm {
    RC4_40,       // 40-bit RC4
    RC4_128,      // 128-bit RC4
    AES_128,      // 128-bit AES
    AES_256,      // 256-bit AES
    AES_256_R6    // 256-bit AES (PDF 2.0)
};

// Security handler
enum class SecurityHandler {
    Standard,     // Standard security handler
    Public_Key    // Public-key security handler
};

// Encryption info
struct EncryptionInfo {
    SecurityHandler handler;
    EncryptionAlgorithm algorithm;
    int key_length_bits;
    bool has_user_password;
    bool has_owner_password;
    uint32_t permissions;
    bool encrypt_metadata;
    std::string filter;
    int version;
    int revision;
};

// Security class
class PDFEDITOR_API Security {
public:
    // ===== Query Security Status =====
    
    // Check if document is encrypted
    static bool is_encrypted(Document* doc);
    
    // Get encryption info
    static EncryptionInfo get_encryption_info(Document* doc);
    
    // Check if user password is set
    static bool has_user_password(Document* doc);
    
    // Check if owner password is set
    static bool has_owner_password(Document* doc);
    
    // Get permissions
    static uint32_t get_permissions(Document* doc);
    
    // Check specific permission
    static bool has_permission(Document* doc, Permission perm);
    
    // ===== Encryption =====
    
    // Encrypt with password
    static bool encrypt(
        Document* doc,
        const std::string& user_password,
        const std::string& owner_password,
        uint32_t permissions = static_cast<uint32_t>(Permission::All),
        EncryptionAlgorithm algorithm = EncryptionAlgorithm::AES_256
    );
    
    // Encrypt with user password only
    static bool encrypt_user(
        Document* doc,
        const std::string& password,
        uint32_t permissions = static_cast<uint32_t>(Permission::All),
        EncryptionAlgorithm algorithm = EncryptionAlgorithm::AES_256
    );
    
    // Encrypt with owner password only (no user password)
    static bool encrypt_owner(
        Document* doc,
        const std::string& owner_password,
        uint32_t permissions = static_cast<uint32_t>(Permission::All),
        EncryptionAlgorithm algorithm = EncryptionAlgorithm::AES_256
    );
    
    // ===== Decryption =====
    
    // Remove encryption
    static bool decrypt(Document* doc, const std::string& password = "");
    
    // Remove user password (keep owner)
    static bool remove_user_password(Document* doc, const std::string& owner_password);
    
    // Remove owner password (keep user)
    static bool remove_owner_password(Document* doc, const std::string& user_password);
    
    // ===== Password Management =====
    
    // Change user password
    static bool change_user_password(
        Document* doc,
        const std::string& old_password,
        const std::string& new_password
    );
    
    // Change owner password
    static bool change_owner_password(
        Document* doc,
        const std::string& old_password,
        const std::string& new_password
    );
    
    // Verify password
    static bool verify_password(Document* doc, const std::string& password);
    
    // Check password strength
    enum class PasswordStrength {
        VeryWeak,
        Weak,
        Medium,
        Strong,
        VeryStrong
    };
    static PasswordStrength check_password_strength(const std::string& password);
    
    // ===== Permissions =====
    
    // Set permissions
    static bool set_permissions(
        Document* doc,
        uint32_t permissions,
        const std::string& owner_password
    );
    
    // Add permission
    static bool add_permission(
        Document* doc,
        Permission perm,
        const std::string& owner_password
    );
    
    // Remove permission
    static bool remove_permission(
        Document* doc,
        Permission perm,
        const std::string& owner_password
    );
    
    // Check if permission is allowed
    static bool is_allowed(Document* doc, Permission perm);
    
    // ===== Certificate-Based Encryption =====
    
    // Encrypt with certificate
    static bool encrypt_with_certificate(
        Document* doc,
        const std::string& certificate_path,
        uint32_t permissions = static_cast<uint32_t>(Permission::All)
    );
    
    // Add recipient certificate
    static bool add_recipient(
        Document* doc,
        const std::string& certificate_path,
        uint32_t permissions
    );
    
    // Remove recipient
    static bool remove_recipient(
        Document* doc,
        const std::string& certificate_path
    );
    
    // List recipients
    static std::vector<std::string> list_recipients(Document* doc);
    
    // ===== Digital Rights Management (DRM) =====
    
    // Set document ID
    static bool set_document_id(Document* doc, const std::string& id);
    
    // Get document ID
    static std::string get_document_id(Document* doc);
    
    // Set expiration date
    static bool set_expiration_date(Document* doc, const std::string& date);
    
    // Get expiration date
    static std::string get_expiration_date(Document* doc);
    
    // Check if expired
    static bool is_expired(Document* doc);
    
    // ===== Redaction Security =====
    
    // Mark area for redaction
    static bool mark_redaction(
        Page* page,
        const Rect& area,
        const Color& color = Color::black()
    );
    
    // Apply all redactions (permanent removal)
    static bool apply_redactions(Document* doc);
    
    // Remove redaction marks
    static bool remove_redaction_marks(Page* page);
    
    // Search and redact text
    static bool search_and_redact(
        Document* doc,
        const std::string& text,
        bool case_sensitive = false
    );
    
    // Redact by pattern (regex)
    static bool redact_by_pattern(
        Document* doc,
        const std::string& pattern
    );
    
    // ===== Sanitization =====
    
    // Remove hidden data
    static bool remove_hidden_data(Document* doc);
    
    // Remove comments
    static bool remove_comments(Document* doc);
    
    // Remove metadata
    static bool remove_metadata(Document* doc);
    
    // Remove attachments
    static bool remove_attachments(Document* doc);
    
    // Remove bookmarks
    static bool remove_bookmarks(Document* doc);
    
    // Remove JavaScript
    static bool remove_javascript(Document* doc);
    
    // Remove links
    static bool remove_links(Document* doc);
    
    // Full sanitization (all of above)
    static bool sanitize_document(Document* doc);
    
    // ===== Security Analysis =====
    
    // Analyze document security
    struct SecurityReport {
        bool is_encrypted;
        bool has_passwords;
        uint32_t permissions;
        bool has_javascript;
        bool has_attachments;
        bool has_external_links;
        bool metadata_present;
        std::vector<std::string> security_warnings;
        std::vector<std::string> recommendations;
    };
    
    static SecurityReport analyze_security(Document* doc);
    
    // Check for vulnerabilities
    static std::vector<std::string> check_vulnerabilities(Document* doc);
    
    // ===== Watermarking for Security =====
    
    // Add security watermark (visible)
    static bool add_visible_watermark(
        Document* doc,
        const std::string& text,
        float opacity = 0.3f
    );
    
    // Add invisible watermark (steganography)
    static bool add_invisible_watermark(
        Document* doc,
        const std::string& data
    );
    
    // Extract invisible watermark
    static std::string extract_invisible_watermark(Document* doc);
    
    // ===== Access Control =====
    
    // Set document open action
    static bool set_open_action(
        Document* doc,
        const std::string& javascript
    );
    
    // Require password to open
    static bool require_password_to_open(
        Document* doc,
        const std::string& password
    );
    
    // Set usage rights
    static bool set_usage_rights(
        Document* doc,
        bool enable_commenting,
        bool enable_form_fill,
        bool enable_digital_signatures,
        bool enable_assembly
    );
    
    // ===== Compliance =====
    
    // Check PDF/A compliance (no encryption allowed)
    static bool is_pdfa_compatible(Document* doc);
    
    // Make compatible with PDF/A (remove encryption)
    static bool make_pdfa_compatible(Document* doc);
    
    // Check if document meets security policy
    struct SecurityPolicy {
        bool require_encryption;
        EncryptionAlgorithm min_algorithm;
        bool require_owner_password;
        uint32_t required_permissions;
        bool allow_javascript;
        bool allow_attachments;
        bool require_metadata_encryption;
    };
    
    static bool meets_policy(Document* doc, const SecurityPolicy& policy);
    
    // Apply security policy
    static bool apply_policy(Document* doc, const SecurityPolicy& policy);
};

} // namespace pdfeditor
