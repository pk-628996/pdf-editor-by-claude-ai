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

# Find the include directory
find_path(MuPDF_INCLUDE_DIR
    NAMES mupdf/fitz.h
    PATHS
        ${PC_MuPDF_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/third_party/mupdf/include
        /usr/include
        /usr/local/include
        /opt/local/include
    PATH_SUFFIXES mupdf
)

# Find the main library
find_library(MuPDF_LIBRARY
    NAMES mupdf libmupdf
    PATHS
        ${PC_MuPDF_LIBRARY_DIRS}
        ${CMAKE_SOURCE_DIR}/third_party/mupdf/build/release
        ${CMAKE_SOURCE_DIR}/third_party/mupdf/build
        /usr/lib
        /usr/local/lib
        /opt/local/lib
    PATH_SUFFIXES lib lib64
)

# Find additional MuPDF libraries
find_library(MuPDF_THIRD_LIBRARY
    NAMES mupdf-third libmupdf-third mupdfthird
    PATHS
        ${PC_MuPDF_LIBRARY_DIRS}
        ${CMAKE_SOURCE_DIR}/third_party/mupdf/build/release
        ${CMAKE_SOURCE_DIR}/third_party/mupdf/build
        /usr/lib
        /usr/local/lib
        /opt/local/lib
    PATH_SUFFIXES lib lib64
)

# Try to extract version from header
if(MuPDF_INCLUDE_DIR)
    file(STRINGS "${MuPDF_INCLUDE_DIR}/mupdf/fitz/version.h" 
         MuPDF_VERSION_LINE 
         REGEX "^#define[\t ]+FZ_VERSION[\t ]+\".*\"")
    
    if(MuPDF_VERSION_LINE)
        string(REGEX REPLACE "^#define[\t ]+FZ_VERSION[\t ]+\"([^\"]*)\".*" "\\1"
               MuPDF_VERSION "${MuPDF_VERSION_LINE}")
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
        
        # MuPDF requires these system libraries
        if(UNIX AND NOT APPLE)
            set_target_properties(MuPDF::MuPDF PROPERTIES
                INTERFACE_LINK_LIBRARIES "m;pthread"
            )
        endif()
    endif()
endif()

mark_as_advanced(
    MuPDF_INCLUDE_DIR
    MuPDF_LIBRARY
    MuPDF_THIRD_LIBRARY
)
