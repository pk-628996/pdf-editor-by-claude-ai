# Core library CMakeLists.txt

# Collect source files
set(CORE_SOURCES
    src/document.cpp
    src/renderer.cpp
    src/editor.cpp
    src/annotations.cpp
    src/bookmarks.cpp
    src/metadata.cpp
    src/signing.cpp
    src/forms.cpp
    src/ocr.cpp
    src/security.cpp
    src/optimizer.cpp
)

# Collect header files
set(CORE_HEADERS
    include/pdfeditor/core.h
    include/pdfeditor/document.h
    include/pdfeditor/renderer.h
    include/pdfeditor/editor.h
    include/pdfeditor/annotations.h
    include/pdfeditor/bookmarks.h
    include/pdfeditor/metadata.h
    include/pdfeditor/signing.h
    include/pdfeditor/forms.h
    include/pdfeditor/ocr.h
    include/pdfeditor/security.h
    include/pdfeditor/optimizer.h
)

# Create the shared library
add_library(pdfeditor_core SHARED
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

# Set library properties
set_target_properties(pdfeditor_core PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME pdfeditor
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Include directories
target_include_directories(pdfeditor_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link Qt libraries
target_link_libraries(pdfeditor_core
    PUBLIC
        Qt6::Core
        Qt6::Xml
    PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
)

# Link MuPDF if available
if(TARGET MuPDF::MuPDF)
    target_link_libraries(pdfeditor_core PRIVATE MuPDF::MuPDF)
    target_compile_definitions(pdfeditor_core PRIVATE USE_MUPDF)
endif()

# Link Tesseract if available
if(TARGET Tesseract::Tesseract)
    target_link_libraries(pdfeditor_core PRIVATE Tesseract::Tesseract)
    target_compile_definitions(pdfeditor_core PRIVATE USE_TESSERACT)
endif()

# Platform-specific settings
if(WIN32)
    target_compile_definitions(pdfeditor_core PRIVATE
        PDFEDITOR_EXPORTS
        _CRT_SECURE_NO_WARNINGS
    )
    
    # Export symbols
    set_target_properties(pdfeditor_core PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
elseif(APPLE)
    set_target_properties(pdfeditor_core PROPERTIES
        MACOSX_RPATH ON
        INSTALL_RPATH "@loader_path"
    )
else() # Linux
    set_target_properties(pdfeditor_core PROPERTIES
        INSTALL_RPATH "$ORIGIN"
    )
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(pdfeditor_core PRIVATE /W4)
else()
    target_compile_options(pdfeditor_core PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter
    )
endif()

# Add position independent code for static linking
set_target_properties(pdfeditor_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Installation
install(TARGETS pdfeditor_core
    EXPORT PDFEditorTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/pdfeditor
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# Export targets
install(EXPORT PDFEditorTargets
    FILE PDFEditorTargets.cmake
    NAMESPACE PDFEditor::
    DESTINATION lib/cmake/PDFEditor
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/PDFEditorConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/PDFEditorConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/PDFEditorConfig.cmake"
    INSTALL_DESTINATION lib/cmake/PDFEditor
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/PDFEditorConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/PDFEditorConfigVersion.cmake"
    DESTINATION lib/cmake/PDFEditor
)
