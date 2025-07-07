set(DRAMSIM3_ROOT_DIR       ${PROJECT_SOURCE_DIR}/src/externals/dramsim3)
set(DRAMSIM3_INC_DIR        ${DRAMSIM3_ROOT_DIR}/src)
set(DRAMSIM3_EXT_INC_DIR    ${DRAMSIM3_ROOT_DIR}/ext/headers)

add_library(eventsim_ip SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/dma_engine.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/saos.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/saos_core.cc
)

target_include_directories(eventsim_ip
PUBLIC 
    ${PROJECT_SOURCE_DIR}/src/event_sim
    ${DRAMSIM3_INC_DIR}
    ${DRAMSIM3_EXT_INC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(eventsim_ip PRIVATE dramsim3)

target_compile_options(eventsim_ip PRIVATE -Wall)

set_target_properties(eventsim_ip
PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)