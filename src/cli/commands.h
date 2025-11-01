#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>

namespace pdfeditor {
namespace cli {

// Command-line argument structure
struct Arguments {
    std::string command;
    std::vector<std::string> positional;
    std::map<std::string, std::string> options;
    std::map<std::string, bool> flags;
    
    bool has_option(const std::string& key) const {
        return options.find(key) != options.end();
    }
    
    std::string get_option(const std::string& key, const std::string& default_val = "") const {
        auto it = options.find(key);
        return (it != options.end()) ? it->second : default_val;
    }
    
    bool get_flag(const std::string& key) const {
        auto it = flags.find(key);
        return (it != flags.end()) ? it->second : false;
    }
};

// Command handler function type
using CommandHandler = std::function<int(const Arguments&)>;

// Command registry
class CommandRegistry {
public:
    static CommandRegistry& instance();
    
    void register_command(
        const std::string& name,
        const std::string& description,
        const std::string& usage,
        CommandHandler handler
    );
    
    int execute(const std::string& command, const Arguments& args);
    
    void print_help() const;
    void print_command_help(const std::string& command) const;
    
    std::vector<std::string> get_commands() const;
    
private:
    CommandRegistry() = default;
    
    struct CommandInfo {
        std::string description;
        std::string usage;
        CommandHandler handler;
    };
    
    std::map<std::string, CommandInfo> commands_;
};

// Argument parser
class ArgumentParser {
public:
    static Arguments parse(int argc, char* argv[]);
    
private:
    static bool is_option(const std::string& arg);
    static bool is_flag(const std::string& arg);
    static std::string strip_dashes(const std::string& arg);
};

// Utility functions
namespace utils {
    // Print colored output
    void print_error(const std::string& message);
    void print_success(const std::string& message);
    void print_warning(const std::string& message);
    void print_info(const std::string& message);
    
    // Progress bar
    class ProgressBar {
    public:
        ProgressBar(int total, const std::string& label = "");
        void update(int current);
        void finish();
        
    private:
        int total_;
        int current_;
        std::string label_;
    };
    
    // File operations
    bool file_exists(const std::string& path);
    std::string get_output_path(const std::string& input, const std::string& suffix);
}

// Command implementations
namespace commands {
    // Document info
    int cmd_info(const Arguments& args);
    
    // Bookmarks
    int cmd_bookmarks_list(const Arguments& args);
    int cmd_bookmarks_add(const Arguments& args);
    int cmd_bookmarks_remove(const Arguments& args);
    int cmd_bookmarks_export(const Arguments& args);
    int cmd_bookmarks_import(const Arguments& args);
    
    // Metadata
    int cmd_metadata_show(const Arguments& args);
    int cmd_metadata_set(const Arguments& args);
    int cmd_metadata_sanitize(const Arguments& args);
    int cmd_metadata_xmp(const Arguments& args);
    
    // Pages
    int cmd_pages_count(const Arguments& args);
    int cmd_pages_extract(const Arguments& args);
    int cmd_pages_delete(const Arguments& args);
    int cmd_pages_rotate(const Arguments& args);
    
    // Merge/Split
    int cmd_merge(const Arguments& args);
    int cmd_split(const Arguments& args);
    
    // Rendering
    int cmd_render(const Arguments& args);
    int cmd_thumbnail(const Arguments& args);
    
    // Annotations
    int cmd_annotations_list(const Arguments& args);
    int cmd_annotations_flatten(const Arguments& args);
    int cmd_annotations_remove(const Arguments& args);
    
    // Security
    int cmd_encrypt(const Arguments& args);
    int cmd_decrypt(const Arguments& args);
    
    // OCR
    int cmd_ocr(const Arguments& args);
    
    // Optimization
    int cmd_optimize(const Arguments& args);
    int cmd_linearize(const Arguments& args);
    
    // Validation
    int cmd_validate(const Arguments& args);
}

// Register all commands
void register_all_commands();

} // namespace cli
} // namespace pdfeditor
