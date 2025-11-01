#include "commands.h"
#include "pdfeditor/core.h"
#include "pdfeditor/document.h"
#include "pdfeditor/bookmarks.h"
#include "pdfeditor/metadata.h"
#include "pdfeditor/renderer.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace pdfeditor {
namespace cli {

// CommandRegistry implementation
CommandRegistry& CommandRegistry::instance() {
    static CommandRegistry registry;
    return registry;
}

void CommandRegistry::register_command(
    const std::string& name,
    const std::string& description,
    const std::string& usage,
    CommandHandler handler
) {
    CommandInfo info;
    info.description = description;
    info.usage = usage;
    info.handler = handler;
    commands_[name] = info;
}

int CommandRegistry::execute(const std::string& command, const Arguments& args) {
    auto it = commands_.find(command);
    if (it == commands_.end()) {
        return -1;  // Command not found
    }
    
    return it->second.handler(args);
}

void CommandRegistry::print_help() const {
    std::cout << "Available commands:" << std::endl;
    
    for (const auto& [name, info] : commands_) {
        std::cout << "  " << std::left << std::setw(20) << name
                  << info.description << std::endl;
    }
}

void CommandRegistry::print_command_help(const std::string& command) const {
    auto it = commands_.find(command);
    if (it == commands_.end()) {
        std::cout << "Unknown command: " << command << std::endl;
        return;
    }
    
    std::cout << "Command: " << command << std::endl;
    std::cout << "Description: " << it->second.description << std::endl;
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  " << it->second.usage << std::endl;
}

std::vector<std::string> CommandRegistry::get_commands() const {
    std::vector<std::string> names;
    for (const auto& [name, info] : commands_) {
        names.push_back(name);
    }
    return names;
}

// ArgumentParser implementation
Arguments ArgumentParser::parse(int argc, char* argv[]) {
    Arguments args;
    
    if (argc < 2) {
        return args;
    }
    
    // First argument is the command
    args.command = argv[1];
    
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (is_option(arg)) {
            // Option with value: --key value or --key=value
            std::string key = strip_dashes(arg);
            
            size_t eq_pos = key.find('=');
            if (eq_pos != std::string::npos) {
                // --key=value format
                std::string value = key.substr(eq_pos + 1);
                key = key.substr(0, eq_pos);
                args.options[key] = value;
            } else if (i + 1 < argc && !is_option(argv[i + 1])) {
                // --key value format
                args.options[key] = argv[++i];
            } else {
                // Treat as flag if no value follows
                args.flags[key] = true;
            }
        } else if (is_flag(arg)) {
            // Short flag: -f
            std::string key = strip_dashes(arg);
            
            // Handle multiple short flags: -abc -> -a -b -c
            if (key.length() > 1 && arg[0] == '-' && arg[1] != '-') {
                for (char c : key) {
                    args.flags[std::string(1, c)] = true;
                }
            } else {
                args.flags[key] = true;
            }
        } else {
            // Positional argument
            args.positional.push_back(arg);
        }
    }
    
    return args;
}

bool ArgumentParser::is_option(const std::string& arg) {
    return arg.length() >= 2 && arg[0] == '-' && arg[1] == '-';
}

bool ArgumentParser::is_flag(const std::string& arg) {
    return arg.length() >= 2 && arg[0] == '-' && arg[1] != '-';
}

std::string ArgumentParser::strip_dashes(const std::string& arg) {
    if (arg.length() >= 2 && arg[0] == '-') {
        if (arg[1] == '-') {
            return arg.substr(2);  // Strip --
        } else {
            return arg.substr(1);  // Strip -
        }
    }
    return arg;
}

// Utility functions
namespace utils {
    
#ifdef _WIN32
    // Windows console colors
    void set_color(int color) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
    }
    #define COLOR_RED 12
    #define COLOR_GREEN 10
    #define COLOR_YELLOW 14
    #define COLOR_RESET 7
#else
    // ANSI color codes
    const char* COLOR_RED = "\033[1;31m";
    const char* COLOR_GREEN = "\033[1;32m";
    const char* COLOR_YELLOW = "\033[1;33m";
    const char* COLOR_RESET = "\033[0m";
#endif

    void print_error(const std::string& message) {
#ifdef _WIN32
        set_color(COLOR_RED);
        std::cerr << "Error: " << message << std::endl;
        set_color(COLOR_RESET);
#else
        std::cerr << COLOR_RED << "Error: " << message << COLOR_RESET << std::endl;
#endif
    }
    
    void print_success(const std::string& message) {
#ifdef _WIN32
        set_color(COLOR_GREEN);
        std::cout << message << std::endl;
        set_color(COLOR_RESET);
#else
        std::cout << COLOR_GREEN << message << COLOR_RESET << std::endl;
#endif
    }
    
    void print_warning(const std::string& message) {
#ifdef _WIN32
        set_color(COLOR_YELLOW);
        std::cout << "Warning: " << message << std::endl;
        set_color(COLOR_RESET);
#else
        std::cout << COLOR_YELLOW << "Warning: " << message << COLOR_RESET << std::endl;
#endif
    }
    
    void print_info(const std::string& message) {
        std::cout << message << std::endl;
    }
    
    // ProgressBar implementation
    ProgressBar::ProgressBar(int total, const std::string& label)
        : total_(total), current_(0), label_(label) {
        update(0);
    }
    
    void ProgressBar::update(int current) {
        current_ = current;
        
        int percentage = (total_ > 0) ? (current * 100 / total_) : 0;
        int bar_width = 50;
        int filled = (bar_width * current) / total_;
        
        std::cout << "\r" << label_ << " [";
        for (int i = 0; i < bar_width; ++i) {
            if (i < filled) {
                std::cout << "=";
            } else if (i == filled) {
                std::cout << ">";
            } else {
                std::cout << " ";
            }
        }
        std::cout << "] " << percentage << "% (" << current << "/" << total_ << ")";
        std::cout.flush();
    }
    
    void ProgressBar::finish() {
        update(total_);
        std::cout << std::endl;
    }
    
    bool file_exists(const std::string& path) {
        return fs::exists(path);
    }
    
    std::string get_output_path(const std::string& input, const std::string& suffix) {
        fs::path p(input);
        std::string stem = p.stem().string();
        std::string ext = p.extension().string();
        
        return stem + suffix + ext;
    }

} // namespace utils

// Command implementations
namespace commands {
    
    // Document info command
    int cmd_info(const Arguments& args) {
        if (args.positional.empty()) {
            utils::print_error("No input file specified");
            return EXIT_FAILURE;
        }
        
        std::string input_file = args.positional[0];
        
        if (!utils::file_exists(input_file)) {
            utils::print_error("File not found: " + input_file);
            return EXIT_FAILURE;
        }
        
        auto result = Document::open(input_file);
        if (!result.is_ok()) {
            utils::print_error("Failed to open PDF: " + result.error_message());
            return EXIT_FAILURE;
        }
        
        auto doc = result.value();
        DocumentInfo info = doc->get_info();
        
        std::cout << "PDF Document Information" << std::endl;
        std::cout << "========================" << std::endl;
        std::cout << "File: " << input_file << std::endl;
        std::cout << "Pages: " << info.page_count << std::endl;
        std::cout << "File Size: " << info.file_size << " bytes" << std::endl;
        std::cout << "PDF Version: " << static_cast<int>(info.version) << std::endl;
        std::cout << "Encrypted: " << (info.is_encrypted ? "Yes" : "No") << std::endl;
        std::cout << "Linearized: " << (info.is_linearized ? "Yes" : "No") << std::endl;
        std::cout << std::endl;
        std::cout << "Metadata:" << std::endl;
        std::cout << "  Title: " << info.title << std::endl;
        std::cout << "  Author: " << info.author << std::endl;
        std::cout << "  Subject: " << info.subject << std::endl;
        std::cout << "  Keywords: " << info.keywords << std::endl;
        std::cout << "  Creator: " << info.creator << std::endl;
        std::cout << "  Producer: " << info.producer << std::endl;
        std::cout << "  Creation Date: " << info.creation_date << std::endl;
        std::cout << "  Modification Date: " << info.modification_date << std::endl;
        
        return EXIT_SUCCESS;
    }
    
    // Bookmarks list command
    int cmd_bookmarks_list(const Arguments& args) {
        if (args.positional.empty()) {
            utils::print_error("No input file specified");
            return EXIT_FAILURE;
        }
        
        std::string input_file = args.positional[0];
        auto result = Document::open(input_file);
        if (!result.is_ok()) {
            utils::print_error("Failed to open PDF: " + result.error_message());
            return EXIT_FAILURE;
        }
        
        auto doc = result.value();
        auto bookmarks = Bookmarks::list(doc.get());
        
        bool json_output = args.get_flag("json");
        
        if (json_output) {
            std::cout << Bookmarks::export_json(doc.get()) << std::endl;
        } else {
            std::cout << "Bookmarks (" << bookmarks.size() << " total)" << std::endl;
            std::cout << "================================" << std::endl;
            
            std::function<void(const std::vector<BookmarkEntry>&, int)> print_bookmarks;
            print_bookmarks = [&](const std::vector<BookmarkEntry>& bms, int indent) {
                for (const auto& bm : bms) {
                    std::string indent_str(indent * 2, ' ');
                    std::cout << indent_str << "- " << bm.title 
                              << " (page " << (bm.destination.page_index + 1) << ")" 
                              << std::endl;
                    
                    if (!bm.children.empty()) {
                        print_bookmarks(bm.children, indent + 1);
                    }
                }
            };
            
            print_bookmarks(bookmarks, 0);
        }
        
        return EXIT_SUCCESS;
    }
    
    // Metadata show command
    int cmd_metadata_show(const Arguments& args) {
        if (args.positional.empty()) {
            utils::print_error("No input file specified");
            return EXIT_FAILURE;
        }
        
        std::string input_file = args.positional[0];
        auto result = Document::open(input_file);
        if (!result.is_ok()) {
            utils::print_error("Failed to open PDF: " + result.error_message());
            return EXIT_FAILURE;
        }
        
        auto doc = result.value();
        auto metadata = Metadata::read_info(doc.get());
        
        bool json_output = args.get_flag("json");
        
        if (json_output) {
            std::cout << Metadata::export_json(doc.get()) << std::endl;
        } else {
            std::cout << "PDF Metadata" << std::endl;
            std::cout << "============" << std::endl;
            
            for (const auto& [key, value] : metadata) {
                std::cout << key << ": " << value << std::endl;
            }
            
            if (Metadata::has_xmp(doc.get())) {
                std::cout << std::endl;
                std::cout << "XMP Metadata: Present" << std::endl;
            }
        }
        
        return EXIT_SUCCESS;
    }
    
    // More command implementations will follow in Part 2...
    
} // namespace commands

// Register all commands
void register_all_commands() {
    auto& registry = CommandRegistry::instance();
    
    // Document info
    registry.register_command(
        "info",
        "Show PDF document information",
        "pdfeditor-cli info <file>",
        commands::cmd_info
    );
    
    // Bookmarks
    registry.register_command(
        "bookmarks",
        "List PDF bookmarks",
        "pdfeditor-cli bookmarks list <file> [--json]",
        commands::cmd_bookmarks_list
    );
    
    // Metadata
    registry.register_command(
        "metadata",
        "Show PDF metadata",
        "pdfeditor-cli metadata show <file> [--json]",
        commands::cmd_metadata_show
    );
    
    // More commands will be registered here...
}

} // namespace cli
} // namespace pdfeditor
