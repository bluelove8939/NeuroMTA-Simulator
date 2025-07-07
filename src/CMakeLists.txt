add_subdirectory(externals)
add_subdirectory(event_sim)
add_subdirectory(event_sim_ip)

# set(DRAMSIM3_ROOT_DIR ${PROJECT_SOURCE_DIR}/src/externals/dramsim3)
# add_library(dramsim3 SHARED IMPORTED)
# set_target_properties(dramsim3 PROPERTIES 
#     IMPORTED_LOCATION ${DRAMSIM3_ROOT_DIR}/libdramsim3.so 
# )

function(single_source_test src_name)
    add_executable(${src_name} ${CMAKE_CURRENT_SOURCE_DIR}/tests/${src_name}.cc)
    target_include_directories(${src_name} PUBLIC ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries(${src_name} PRIVATE eventsim eventsim_ip dramsim3)
    target_compile_options(${src_name} PRIVATE -Wall)
    set_target_properties(${src_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )
endfunction()

single_source_test(test_saos_simpledma)
single_source_test(test_saos_dramsim3)
