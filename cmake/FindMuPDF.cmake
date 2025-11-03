# FindMuPDF.cmake
# Finds the MuPDF library
#
# This will define the following variables:
#   MuPDF_FOUND         - System has MuPDF
#   MuPDF_INCLUDE_DIRS  - The MuPDF include directories
#   MuPDF_LIBRARIES     - The libraries needed to use MuPDF
#   MuPDF_VERSION       - The version of MuPDF found
#
# and the following imported targets:
#   MuPDF::MuPDF        - The MuPDF library

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_MuPDF QUIET mupdf)
endif()

# Check for MuPDF_ROOT hint
if(MuPDF_ROOT)
    set(MuPDF_SEARCH_PATHS ${MuPDF_ROOT})
else()
    set(MuPDF_SEARCH_PATHS
        ${PC_MuPDF_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/third_party/mupdf
        ${CMAKE_SOURCE_DIR}/mupdf
        $ENV{MUPDF_DIR}
        /usr
        /usr/local
        /opt/local
    )
endif()

# Find the include directory
find_path(MuPDF_INCLUDE_DIR
    NAMES mupdf/fitz.h
    HINTS ${MuPDF_SEARCH_PATHS}
    PATH_SUFFIXES include
    DOC "MuPDF include directory"
)

# Find the main library
find_library(MuPDF_LIBRARY
    NAMES mupdf libmupdf
    HINTS ${MuPDF_SEARCH_PATHS}
    PATH_SUFFIXES lib lib64 platform/win32/x64/Release platform/win32/x64/Debug
    DOC "MuPDF library"
)

# Find third-party library (contains dependencies like zlib, jpeg, etc.)
find_library(MuPDF_THIRD_LIBRARY
    NAMES mupdf-third libmupdf-third mupdfthird thirdparty libthirdparty
    HINTS ${MuPDF_SEARCH_PATHS}
    PATH_SUFFIXES lib lib64 platform/win32/x64/Release platform/win32/x64/Debug
    DOC "MuPDF third-party library"
)

# Try to extract version from header
if(MuPDF_INCLUDE_DIR)
    if(EXISTS "${MuPDF_INCLUDE_DIR}/mupdf/fitz/version.h")
        file(STRINGS "${MuPDF_INCLUDE_DIR}/mupdf/fitz/version.h" 
             MuPDF_VERSION_LINE 
             REGEX "^#define[\t ]+FZ_VERSION[\t ]+\".*\"")
        
        if(MuPDF_VERSION_LINE)
            string(REGEX REPLACE "^#define[\t ]+FZ_VERSION[\t ]+\"([^\"]*)\".*" "\\1"
                   MuPDF_VERSION "${MuPDF_VERSION_LINE}")
        endif()
    endif()
endif()

# Handle the QUIETLY and REQUIRED arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MuPDF
    REQUIRED_VARS
        MuPDF_LIBRARY
        MuPDF_INCLUDE_DIR
    VERSION_VAR
        MuPDF_VERSION
    FAIL_MESSAGE
        "Could not find MuPDF library. Set MuPDF_ROOT to the installation directory."
)

if(MuPDF_FOUND)
    set(MuPDF_LIBRARIES ${MuPDF_LIBRARY})
    set(MuPDF_INCLUDE_DIRS ${MuPDF_INCLUDE_DIR})
    
    # Add third-party library if found
    if(MuPDF_THIRD_LIBRARY)
        list(APPEND MuPDF_LIBRARIES ${MuPDF_THIRD_LIBRARY})
    endif()
    
    # Create imported target
    if(NOT TARGET MuPDF::MuPDF)
        add_library(MuPDF::MuPDF UNKNOWN IMPORTED)
        set_target_properties(MuPDF::MuPDF PROPERTIES
            IMPORTED_LOCATION "${MuPDF_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${MuPDF_INCLUDE_DIR}"
        )
        
        # Link third-party library if available
        if(MuPDF_THIRD_LIBRARY)
            set_target_properties(MuPDF::MuPDF PROPERTIES
                INTERFACE_LINK_LIBRARIES "${MuPDF_THIRD_LIBRARY}"
            )
        endif()
        
        # Windows-specific libraries
        if(WIN32)
            set_target_properties(MuPDF::MuPDF PROPERTIES
                INTERFACE_LINK_LIBRARIES "${MuPDF_THIRD_LIBRARY};advapi32;comdlg32;gdi32;user32;shell32"
            )
        # Unix-specific libraries
        elseif(UNIX)
            set_target_properties(MuPDF::MuPDF PROPERTIES
                INTERFACE_LINK_LIBRARIES "${MuPDF_THIRD_LIBRARY};m;pthread"
            )
        endif()
    endif()
    
    # Debug output
    if(NOT MuPDF_FIND_QUIETLY)
        message(STATUS "Found MuPDF: ${MuPDF_LIBRARY}")
        message(STATUS "  Version: ${MuPDF_VERSION}")
        message(STATUS "  Include: ${MuPDF_INCLUDE_DIR}")
        if(MuPDF_THIRD_LIBRARY)
            message(STATUS "  Third-party lib: ${MuPDF_THIRD_LIBRARY}")
        endif()
    endif()
endif()

mark_as_advanced(
    MuPDF_INCLUDE_DIR
    MuPDF_LIBRARY
    MuPDF_THIRD_LIBRARY
)
