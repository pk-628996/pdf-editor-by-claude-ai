#include "commands.h"
#include "pdfeditor/core.h"
#include <iostream>
#include <cstdlib>

using namespace pdfeditor;
using namespace pdfeditor::cli;

void print_version() {
    std::cout << "PDFEditor CLI v" << Library::get_version() << std::endl;
    std::cout << "A production-ready PDF editor" << std::endl;
    std::cout << "Copyright (C) 2025 PDFEditor Contributors" << std::endl;
}

void print_usage() {
    std::cout << "Usage: pdfeditor-cli <command> [options] <file>" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  info              Show PDF document information" << std::endl;
    std::cout << "  bookmarks         Manage PDF bookmarks/outline" << std::endl;
    std::cout << "  metadata          Manage PDF metadata" << std::endl;
    std::cout << "  pages             Manage PDF pages" << std::endl;
    std::cout << "  merge             Merge multiple PDF files" << std::endl;
    std::cout << "  split             Split PDF into multiple files" << std::endl;
    std::cout << "  render            Render PDF pages to images" << std::endl;
    std::cout << "  thumbnail         Generate page thumbnails" << std::endl;
    std::cout << "  annotations       Manage annotations" << std::endl;
    std::cout << "  encrypt           Encrypt PDF with password" << std::endl;
    std::cout << "  decrypt           Remove PDF password" << std::endl;
    std::cout << "  ocr               Perform OCR on PDF" << std::endl;
    std::cout << "  optimize          Optimize PDF file size" << std::endl;
    std::cout << "  linearize         Linearize PDF for web" << std::endl;
    std::cout << "  validate          Validate PDF structure" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help        Show this help message" << std::endl;
    std::cout << "  -v, --version     Show version information" << std::endl;
    std::cout << "  -o, --out <file>  Output file path" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  pdfeditor-cli info document.pdf" << std::endl;
    std::cout << "  pdfeditor-cli bookmarks list document.pdf --json" << std::endl;
    std::cout << "  pdfeditor-cli metadata set document.pdf --Title \"My Doc\" -o output.pdf" << std::endl;
    std::cout << "  pdfeditor-cli merge file1.pdf file2.pdf -o merged.pdf" << std::endl;
    std::cout << "  pdfeditor-cli render document.pdf --dpi 300 -o page.png" << std::endl;
    std::cout << std::endl;
    std::cout << "For detailed help on a command, use:" << std::endl;
    std::cout << "  pdfeditor-cli <command> --help" << std::endl;
}

int main(int argc, char* argv[]) {
    // Initialize library
    if (!Library::initialize()) {
        std::cerr << "Error: Failed to initialize PDFEditor library" << std::endl;
        return EXIT_FAILURE;
    }
    
    // Parse arguments
    if (argc < 2) {
        print_usage();
        Library::shutdown();
        return EXIT_SUCCESS;
    }
    
    Arguments args = ArgumentParser::parse(argc, argv);
    
    // Handle global flags
    if (args.get_flag("help") || args.get_flag("h")) {
        if (args.command.empty()) {
            print_usage();
        } else {
            CommandRegistry::instance().print_command_help(args.command);
        }
        Library::shutdown();
        return EXIT_SUCCESS;
    }
    
    if (args.get_flag("version") || args.get_flag("v")) {
        print_version();
        Library::shutdown();
        return EXIT_SUCCESS;
    }
    
    // Register all commands
    register_all_commands();
    
    // Execute command
    int result = EXIT_SUCCESS;
    
    try {
        if (args.command.empty()) {
            std::cerr << "Error: No command specified" << std::endl;
            print_usage();
            result = EXIT_FAILURE;
        } else {
            result = CommandRegistry::instance().execute(args.command, args);
            
            if (result == -1) {
                std::cerr << "Error: Unknown command '" << args.command << "'" << std::endl;
                std::cerr << "Use 'pdfeditor-cli --help' for usage information" << std::endl;
                result = EXIT_FAILURE;
            }
        }
    } catch (const Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        result = EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        result = EXIT_FAILURE;
    }
    
    // Cleanup
    Library::shutdown();
    
    return result;
}
