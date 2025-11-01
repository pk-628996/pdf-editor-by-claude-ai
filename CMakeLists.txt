cmake_minimum_required(VERSION 3.20)
project(PDFEditor VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(BUILD_GUI "Build GUI application" ON)
option(BUILD_CLI "Build CLI application" ON)
option(BUILD_TESTS "Build unit tests" ON)
option(USE_MUPDF "Use MuPDF as PDF backend" ON)
option(USE_PDFIUM "Use PDFium as PDF backend" OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Xml)

# Add CMake modules path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find MuPDF
if(USE_MUPDF)
    find_package(MuPDF REQUIRED)
endif()

# Find Tesseract for OCR
find_package(Tesseract)

# Find OpenSSL for signatures
find_package(OpenSSL REQUIRED)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include subdirectories
add_subdirectory(src/core)

if(BUILD_CLI)
    add_subdirectory(src/cli)
endif()

if(BUILD_GUI)
    add_subdirectory(src/gui)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation rules
install(TARGETS pdfeditor_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY src/core/include/pdfeditor
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "PDFEditor")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "PDFEditor Contributors")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Production-ready PDF editor")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

if(WIN32)
    set(CPACK_GENERATOR "WIX;ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "DEB;TGZ")
endif()

include(CPack)
