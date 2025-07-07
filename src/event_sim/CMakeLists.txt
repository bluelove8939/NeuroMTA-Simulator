add_library(eventsim SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/command.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/context.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/module.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/core.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/debugger.cc
)

target_include_directories(eventsim 
PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(eventsim PRIVATE -Wall)

set_target_properties(eventsim 
PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)