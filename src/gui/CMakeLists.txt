# GUI application CMakeLists.txt

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Collect source files
set(GUI_SOURCES
    main.cpp
    mainwindow.cpp
    pdfviewer.cpp
    bookmarkspanel.cpp
    thumbnailspanel.cpp
    metadatadialog.cpp
    annotationtoolbar.cpp
    searchdialog.cpp
    preferencesdialog.cpp
)

# Collect header files
set(GUI_HEADERS
    mainwindow.h
    pdfviewer.h
    bookmarkspanel.h
    thumbnailspanel.h
    metadatadialog.h
    annotationtoolbar.h
    searchdialog.h
    preferencesdialog.h
)

# Collect UI files
set(GUI_UI_FILES
    mainwindow.ui
    metadatadialog.ui
    searchdialog.ui
    preferencesdialog.ui
)

# Resource files
set(GUI_RESOURCES
    resources.qrc
)

# Create executable
add_executable(pdfeditor
    ${GUI_SOURCES}
    ${GUI_HEADERS}
    ${GUI_UI_FILES}
    ${GUI_RESOURCES}
)

# Link libraries
target_link_libraries(pdfeditor
    PRIVATE
        pdfeditor_core
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
        Qt6::PrintSupport
)

target_include_directories(pdfeditor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Set properties
set_target_properties(pdfeditor PROPERTIES
    OUTPUT_NAME pdfeditor
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Platform-specific settings
if(WIN32)
    # Windows application (not console)
    set_target_properties(pdfeditor PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    # Add application icon
    target_sources(pdfeditor PRIVATE resources/app.rc)
    
elseif(APPLE)
    # macOS bundle
    set_target_properties(pdfeditor PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/Info.plist
        MACOSX_BUNDLE_ICON_FILE app_icon.icns
        MACOSX_RPATH ON
        INSTALL_RPATH "@executable_path/../Frameworks"
    )
    
    # Add application icon
    set(MACOSX_BUNDLE_ICON_FILE app_icon.icns)
    set_source_files_properties(resources/app_icon.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(pdfeditor PRIVATE resources/app_icon.icns)
    
else()
    # Linux application
    set_target_properties(pdfeditor PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
    )
endif()

# Installation
install(TARGETS pdfeditor
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

# Install desktop file and icon on Linux
if(UNIX AND NOT APPLE)
    install(FILES resources/pdfeditor.desktop
        DESTINATION share/applications
    )
    install(FILES resources/icons/app_icon.png
        DESTINATION share/icons/hicolor/256x256/apps
        RENAME pdfeditor.png
    )
endif()

# macOS bundle fixup
if(APPLE)
    install(CODE "
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/pdfeditor.app\" \"\" \"\")
    ")
endif()
