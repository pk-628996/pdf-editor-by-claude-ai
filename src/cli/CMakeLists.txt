# CLI application CMakeLists.txt

add_executable(pdfeditor-cli
    main.cpp
    commands.cpp
)

target_link_libraries(pdfeditor-cli
    PRIVATE
        pdfeditor_core
        Qt6::Core
)

target_include_directories(pdfeditor-cli
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set output name
set_target_properties(pdfeditor-cli PROPERTIES
    OUTPUT_NAME pdfeditor-cli
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Platform-specific settings
if(WIN32)
    # Console application on Windows
    set_target_properties(pdfeditor-cli PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
elseif(APPLE)
    set_target_properties(pdfeditor-cli PROPERTIES
        MACOSX_RPATH ON
        INSTALL_RPATH "@executable_path/../lib"
    )
else()
    set_target_properties(pdfeditor-cli PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
    )
endif()

# Installation
install(TARGETS pdfeditor-cli
    RUNTIME DESTINATION bin
)
