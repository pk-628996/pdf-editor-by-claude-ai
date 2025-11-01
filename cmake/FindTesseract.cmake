# FindTesseract.cmake
# Finds the Tesseract OCR library
#
# This will define the following variables:
#   Tesseract_FOUND         - System has Tesseract
#   Tesseract_INCLUDE_DIRS  - The Tesseract include directories
#   Tesseract_LIBRARIES     - The libraries needed to use Tesseract
#   Tesseract_VERSION       - The version of Tesseract found
#
# and the following imported targets:
#   Tesseract::Tesseract    - The Tesseract library

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_Tesseract QUIET tesseract)
    set(Tesseract_VERSION ${PC_Tesseract_VERSION})
endif()

# Find the include directory
find_path(Tesseract_INCLUDE_DIR
    NAMES tesseract/baseapi.h
    PATHS
        ${PC_Tesseract_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/third_party/tesseract/include
        /usr/include
        /usr/local/include
        /opt/local/include
        C:/Program Files/Tesseract-OCR/include
        C:/Program Files (x86)/Tesseract-OCR/include
)

# Find the library
find_library(Tesseract_LIBRARY
    NAMES tesseract libtesseract tesseract50 tesseract51
    PATHS
        ${PC_Tesseract_LIBRARY_DIRS}
        ${CMAKE_SOURCE_DIR}/third_party/tesseract/lib
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        C:/Program Files/Tesseract-OCR/lib
        C:/Program Files (x86)/Tesseract-OCR/lib
    PATH_SUFFIXES lib lib64
)

# Find Leptonica (required by Tesseract)
find_path(Leptonica_INCLUDE_DIR
    NAMES leptonica/allheaders.h
    PATHS
        ${PC_Leptonica_INCLUDE_DIRS}
        /usr/include
        /usr/local/include
        /opt/local/include
        C:/Program Files/Leptonica/include
)

find_library(Leptonica_LIBRARY
    NAMES leptonica libleptonica lept175 lept180
    PATHS
        ${PC_Leptonica_LIBRARY_DIRS}
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        C:/Program Files/Leptonica/lib
    PATH_SUFFIXES lib lib64
)

# Try to get version from header if pkg-config didn't provide it
if(NOT Tesseract_VERSION AND Tesseract_INCLUDE_DIR)
    file(STRINGS "${Tesseract_INCLUDE_DIR}/tesseract/version.h" 
         Tesseract_VERSION_LINE 
         REGEX "^#define[\t ]+TESSERACT_VERSION_STR[\t ]+\".*\"")
    
    if(Tesseract_VERSION_LINE)
        string(REGEX REPLACE "^#define[\t ]+TESSERACT_VERSION_STR[\t ]+\"([^\"]*)\".*" "\\1"
               Tesseract_VERSION "${Tesseract_VERSION_LINE}")
    endif()
endif()

# Handle the QUIETLY and REQUIRED arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Tesseract
    REQUIRED_VARS
        Tesseract_LIBRARY
        Tesseract_INCLUDE_DIR
        Leptonica_LIBRARY
        Leptonica_INCLUDE_DIR
    VERSION_VAR
        Tesseract_VERSION
)

if(Tesseract_FOUND)
    set(Tesseract_LIBRARIES ${Tesseract_LIBRARY} ${Leptonica_LIBRARY})
    set(Tesseract_INCLUDE_DIRS ${Tesseract_INCLUDE_DIR} ${Leptonica_INCLUDE_DIR})
    
    # Create imported target
    if(NOT TARGET Tesseract::Tesseract)
        add_library(Tesseract::Tesseract UNKNOWN IMPORTED)
        set_target_properties(Tesseract::Tesseract PROPERTIES
            IMPORTED_LOCATION "${Tesseract_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Tesseract_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${Leptonica_LIBRARY}"
        )
        
        # On Windows, may need additional libraries
        if(WIN32)
            find_library(WS2_32_LIBRARY ws2_32)
            if(WS2_32_LIBRARY)
                set_property(TARGET Tesseract::Tesseract APPEND PROPERTY
                    INTERFACE_LINK_LIBRARIES "${WS2_32_LIBRARY}"
                )
            endif()
        endif()
    endif()
endif()

mark_as_advanced(
    Tesseract_INCLUDE_DIR
    Tesseract_LIBRARY
    Leptonica_INCLUDE_DIR
    Leptonica_LIBRARY
)
