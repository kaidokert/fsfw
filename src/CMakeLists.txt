target_include_directories(${LIB_FSFW_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(${LIB_FSFW_NAME}
                           INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(fsfw)
add_subdirectory(fsfw_hal)
add_subdirectory(fsfw_tests)
