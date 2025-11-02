# Tests CMakeLists.txt

# Find Qt Test module
find_package(Qt6 REQUIRED COMPONENTS Test)

# Enable testing
enable_testing()

# Test helper library
add_library(test_helpers STATIC
    test_helpers.cpp
    test_helpers.h
)

target_link_libraries(test_helpers
    PUBLIC
        pdfeditor_core
        Qt6::Test
)

target_include_directories(test_helpers
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Function to add a test
function(add_pdfeditor_test TEST_NAME)
    add_executable(${TEST_NAME} ${ARGN})
    
    target_link_libraries(${TEST_NAME}
        PRIVATE
            pdfeditor_core
            test_helpers
            Qt6::Test
    )
    
    # Add as CTest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    
    # Set test properties
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 300
        LABELS "unit"
    )
endfunction()

# Unit tests
add_pdfeditor_test(test_document unit/test_document.cpp)
add_pdfeditor_test(test_bookmarks unit/test_bookmarks.cpp)
add_pdfeditor_test(test_metadata unit/test_metadata.cpp)
add_pdfeditor_test(test_renderer unit/test_renderer.cpp)
add_pdfeditor_test(test_annotations unit/test_annotations.cpp)

# Integration tests
add_executable(test_integration
    integration/test_end_to_end.cpp
)

target_link_libraries(test_integration
    PRIVATE
        pdfeditor_core
        test_helpers
        Qt6::Test
)

add_test(NAME test_integration COMMAND test_integration)
set_tests_properties(test_integration PROPERTIES
    TIMEOUT 600
    LABELS "integration"
)

# Copy test data
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/data/)
