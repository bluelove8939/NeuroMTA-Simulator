add_library(neuromta SHARED 
    # Common
    common/command.cc
    common/context.cc
    common/module.cc
    common/core.cc
    common/debugger.cc
    # Modules
    dma_module.cc
    icnt_module.cc
    saos_module.cc
    # Cores
)

target_include_directories(neuromta PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/src/externals/dramsim3/src
    ${PROJECT_SOURCE_DIR}/src/externals/dramsim3/ext/headers
)

target_compile_options(neuromta PRIVATE -Wall)

set_target_properties(neuromta PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)