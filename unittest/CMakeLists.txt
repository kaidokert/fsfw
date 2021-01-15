add_subdirectory(internal)

if(LINK_CATCH2)
    add_subdirectory(tests)
endif()