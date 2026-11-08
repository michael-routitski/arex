set(TEST_FILES
    test_file_previewer.cpp
    test_directory_scanner.cpp
    test_process_runner.cpp
)

foreach(TEST_SRC ${TEST_FILES})
    # Get filename without extension (e.g. test_file_previewer)
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    
    add_executable(${TEST_NAME} ${TEST_SRC})
    target_link_libraries(${TEST_NAME} PRIVATE arex::core)
    target_apply_warnings(${TEST_NAME})
    target_enable_sanitizers(${TEST_NAME})
    
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
