function(single_core_test src_name)
    add_executable(${src_name} ${CMAKE_CURRENT_SOURCE_DIR}/${src_name}.cc)
    target_include_directories(${src_name} PUBLIC ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries(${src_name} PRIVATE neuromta dramsim3)
    target_compile_options(${src_name} PRIVATE -Wall)
    set_target_properties(${src_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endfunction()

single_core_test(test_single_core_saos_dramsim3)
single_core_test(test_single_core_saos_simpledma)
