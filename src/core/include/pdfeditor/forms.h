#pragma once

#include "core.h"
#include "document.h"
#include <string>
#include <vector>
#include <memory>
#include <variant>

namespace pdfeditor {

// Form field types
enum class FieldType {
    Text,
    Button,
    CheckBox,
    RadioButton,
    ComboBox,
    ListBox,
    Signature
};

// Form field flags
enum class FieldFlag : uint32_t {
    None = 0,
    ReadOnly = 1 << 0,
    Required = 1 << 1,
    NoExport = 1 << 2,
    Multiline = 1 << 12,
    Password = 1 << 13,
    FileSelect = 1 << 20,
    DoNotSpellCheck = 1 << 22,
    DoNotScroll = 1 << 23,
    Comb = 1 << 24,
    RichText = 1 << 25
};

// Form field value types
using FieldValue = std::variant<
    std::string,           // Text value
    bool,                  // Checkbox/radio state
    int,                   // List selection index
    std::vector<int>,      // Multiple selections
    std::vector<uint8_t>   // Signature data
>;

// Base form field
struct FormField {
    std::string id;
    std::string name;
    std::string partial_name;
    std::string alternate_name;  // Tooltip
    std::string mapping_name;    // Export name
    FieldType type;
    Rect rect;
    int page_index;
    uint32_t flags;
    FieldValue value;
    FieldValue default_value;
    
    bool is_read_only() const {
        return (flags & static_cast<uint32_t>(FieldFlag::ReadOnly)) != 0;
    }
    
    bool is_required() const {
        return (flags & static_cast<uint32_t>(FieldFlag::Required)) != 0;
    }
};

// Text field
struct TextField : public FormField {
    int max_length;
    bool multiline;
    bool password;
    bool file_select;
    bool rich_text;
    std::string font_name;
    float font_size;
    Color text_color;
    
    TextField() 
        : max_length(0)
        , multiline(false)
        , password(false)
        , file_select(false)
        , rich_text(false)
        , font_size(12.0f)
        , text_color(Color::black()) {
        type = FieldType::Text;
    }
};

// Button field
struct ButtonField : public FormField {
    std::string caption;
    std::string icon_path;
    bool push_button;  // If false, it's a radio/checkbox
    
    ButtonField() : push_button(true) {
        type = FieldType::Button;
    }
};

// Checkbox field
struct CheckBoxField : public FormField {
    std::string export_value;
    bool checked;
    
    CheckBoxField() : checked(false) {
        type = FieldType::CheckBox;
    }
};

// Radio button field
struct RadioButtonField : public FormField {
    std::string export_value;
    std::string group_name;
    bool selected;
    
    RadioButtonField() : selected(false) {
        type = FieldType::RadioButton;
    }
};

// Choice field (ComboBox/ListBox)
struct ChoiceField : public FormField {
    std::vector<std::string> options;
    std::vector<std::string> export_values;
    std::vector<int> selected_indices;
    bool editable;     // ComboBox only
    bool multi_select; // ListBox only
    bool sort;
    
    ChoiceField() 
        : editable(false)
        , multi_select(false)
        , sort(false) {}
};

// Signature field
struct SignatureField : public FormField {
    bool signed;
    std::string signer_name;
    std::string signing_date;
    std::string reason;
    std::string location;
    std::vector<uint8_t> signature_data;
    
    SignatureField() : signed(false) {
        type = FieldType::Signature;
    }
};

// Form management class
class PDFEDITOR_API Forms {
public:
    // ===== Querying =====
    
    // Check if document has forms
    static bool has_forms(Document* doc);
    
    // Get all form fields
    static std::vector<std::shared_ptr<FormField>> get_fields(Document* doc);
    
    // Get fields on specific page
    static std::vector<std::shared_ptr<FormField>> get_page_fields(
        Document* doc,
        int page_index
    );
    
    // Get field by name
    static Result<std::shared_ptr<FormField>> get_field(
        Document* doc,
        const std::string& name
    );
    
    // Get field by ID
    static Result<std::shared_ptr<FormField>> get_field_by_id(
        Document* doc,
        const std::string& id
    );
    
    // Count form fields
    static int count_fields(Document* doc);
    
    // ===== Field Values =====
    
    // Get field value
    static FieldValue get_value(Document* doc, const std::string& field_name);
    
    // Set field value
    static bool set_value(
        Document* doc,
        const std::string& field_name,
        const FieldValue& value
    );
    
    // Reset field to default
    static bool reset_field(Document* doc, const std::string& field_name);
    
    // Reset all fields
    static bool reset_all(Document* doc);
    
    // ===== Text Fields =====
    
    // Set text field value
    static bool set_text(
        Document* doc,
        const std::string& field_name,
        const std::string& text
    );
    
    // Get text field value
    static std::string get_text(Document* doc, const std::string& field_name);
    
    // ===== Checkbox/Radio =====
    
    // Check checkbox
    static bool check(Document* doc, const std::string& field_name);
    
    // Uncheck checkbox
    static bool uncheck(Document* doc, const std::string& field_name);
    
    // Toggle checkbox
    static bool toggle(Document* doc, const std::string& field_name);
    
    // Is checkbox checked
    static bool is_checked(Document* doc, const std::string& field_name);
    
    // Select radio button
    static bool select_radio(
        Document* doc,
        const std::string& group_name,
        const std::string& value
    );
    
    // ===== Choice Fields =====
    
    // Select option by index
    static bool select_option(
        Document* doc,
        const std::string& field_name,
        int index
    );
    
    // Select option by value
    static bool select_option_by_value(
        Document* doc,
        const std::string& field_name,
        const std::string& value
    );
    
    // Select multiple options
    static bool select_options(
        Document* doc,
        const std::string& field_name,
        const std::vector<int>& indices
    );
    
    // Get selected options
    static std::vector<int> get_selected_options(
        Document* doc,
        const std::string& field_name
    );
    
    // ===== Field Creation =====
    
    // Add text field
    static Result<std::string> add_text_field(
        Document* doc,
        int page_index,
        const Rect& rect,
        const std::string& name,
        const std::string& default_value = ""
    );
    
    // Add checkbox
    static Result<std::string> add_checkbox(
        Document* doc,
        int page_index,
        const Rect& rect,
        const std::string& name,
        bool default_checked = false
    );
    
    // Add radio button
    static Result<std::string> add_radio_button(
        Document* doc,
        int page_index,
        const Rect& rect,
        const std::string& group_name,
        const std::string& value
    );
    
    // Add combo box
    static Result<std::string> add_combo_box(
        Document* doc,
        int page_index,
        const Rect& rect,
        const std::string& name,
        const std::vector<std::string>& options
    );
    
    // Add list box
    static Result<std::string> add_list_box(
        Document* doc,
        int page_index,
        const Rect& rect,
        const std::string& name,
        const std::vector<std::string>& options,
        bool multi_select = false
    );
    
    // Add button
    static Result<std::string> add_button(
        Document* doc,
        int page_index,
        const Rect& rect,
        const std::string& name,
        const std::string& caption
    );
    
    // Add signature field
    static Result<std::string> add_signature_field(
        Document* doc,
        int page_index,
        const Rect& rect,
        const std::string& name
    );
    
    // ===== Field Modification =====
    
    // Update field properties
    static bool update_field(
        Document* doc,
        const std::string& field_name,
        const std::shared_ptr<FormField>& field
    );
    
    // Set field rect
    static bool set_field_rect(
        Document* doc,
        const std::string& field_name,
        const Rect& rect
    );
    
    // Set field flags
    static bool set_field_flags(
        Document* doc,
        const std::string& field_name,
        uint32_t flags
    );
    
    // Set field read-only
    static bool set_read_only(
        Document* doc,
        const std::string& field_name,
        bool read_only
    );
    
    // Set field required
    static bool set_required(
        Document* doc,
        const std::string& field_name,
        bool required
    );
    
    // ===== Field Deletion =====
    
    // Remove field
    static bool remove_field(Document* doc, const std::string& field_name);
    
    // Remove all fields
    static bool remove_all_fields(Document* doc);
    
    // ===== Validation =====
    
    // Validate field value
    static bool validate_field(Document* doc, const std::string& field_name);
    
    // Validate all fields
    static bool validate_all(Document* doc);
    
    // Get validation errors
    static std::vector<std::string> get_validation_errors(Document* doc);
    
    // ===== Import/Export =====
    
    // Export form data to FDF
    static std::string export_fdf(Document* doc);
    
    // Import form data from FDF
    static bool import_fdf(Document* doc, const std::string& fdf);
    
    // Export form data to XFDF
    static std::string export_xfdf(Document* doc);
    
    // Import form data from XFDF
    static bool import_xfdf(Document* doc, const std::string& xfdf);
    
    // Export to JSON
    static std::string export_json(Document* doc);
    
    // Import from JSON
    static bool import_json(Document* doc, const std::string& json);
    
    // ===== Flattening =====
    
    // Flatten form (convert to page content)
    static bool flatten(Document* doc);
    
    // Flatten specific field
    static bool flatten_field(Document* doc, const std::string& field_name);
    
    // ===== JavaScript Actions =====
    
    // Set field action (JavaScript)
    static bool set_field_action(
        Document* doc,
        const std::string& field_name,
        const std::string& action_type,  // e.g., "Calculate", "Validate", "Format"
        const std::string& javascript
    );
    
    // Get field action
    static std::string get_field_action(
        Document* doc,
        const std::string& field_name,
        const std::string& action_type
    );
    
    // Remove field action
    static bool remove_field_action(
        Document* doc,
        const std::string& field_name,
        const std::string& action_type
    );
    
    // ===== Calculation Order =====
    
    // Set field calculation order
    static bool set_calculation_order(
        Document* doc,
        const std::vector<std::string>& field_names
    );
    
    // Get calculation order
    static std::vector<std::string> get_calculation_order(Document* doc);
    
    // ===== Tab Order =====
    
    // Set tab order for page
    static bool set_tab_order(
        Document* doc,
        int page_index,
        const std::vector<std::string>& field_names
    );
    
    // Get tab order for page
    static std::vector<std::string> get_tab_order(Document* doc, int page_index);
};

} // namespace pdfeditor
