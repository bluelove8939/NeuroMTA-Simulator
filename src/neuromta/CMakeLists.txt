add_library(neuromta SHARED 
    # Common
    common/command.cc
    common/context.cc
    common/module.cc
    common/core.cc
    common/debugger.cc
    # Modules
    modules/dma_module.cc
    modules/icnt_module.cc
    modules/saos_module.cc
    # Cores
)

target_include_directories(neuromta PUBLIC 
    # NeuroMTA indlude directories    
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/modules
    # DRAMSim3 indlude directories
    ${PROJECT_SOURCE_DIR}/src/externals/dramsim3/src
    ${PROJECT_SOURCE_DIR}/src/externals/dramsim3/ext/headers
    # Booksim2 indlude directories
    ${PROJECT_SOURCE_DIR}/src/externals/booksim2/src
    ${PROJECT_SOURCE_DIR}/src/externals/booksim2/src/networks
    ${PROJECT_SOURCE_DIR}/src/externals/booksim2/src/allocators
    ${PROJECT_SOURCE_DIR}/src/externals/booksim2/src/arbiters
    ${PROJECT_SOURCE_DIR}/src/externals/booksim2/src/power
    ${PROJECT_SOURCE_DIR}/src/externals/booksim2/src/routers
)

target_compile_options(neuromta PRIVATE -Wall)

set_target_properties(neuromta PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)