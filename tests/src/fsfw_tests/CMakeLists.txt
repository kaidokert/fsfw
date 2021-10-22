
if(FSFW_ADD_INTERNAL_TESTS)
    add_subdirectory(internal)
endif()

if(FSFW_BUILD_UNITTESTS)
    add_subdirectory(unit)
endif()
