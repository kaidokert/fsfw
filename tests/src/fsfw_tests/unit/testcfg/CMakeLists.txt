target_sources(${FSFW_TEST_TGT} PRIVATE
    ipc/MissionMessageTypes.cpp
    pollingsequence/PollingSequenceFactory.cpp
)

# Add include paths for the executable
target_include_directories(${FSFW_TEST_TGT} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# These translation files are actually not that relevant for the tests. However, the FSFW tests
# compile against a user-configured variant of the FSFW, which might be configured to include
# translation information. Therefore, empty dummy translation files are compiled here
# so the tests compile in any case.

# If a special translation file for object IDs exists, compile it.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/objects/translateObjects.cpp")
    target_sources(${FSFW_TEST_TGT} PRIVATE
        objects/translateObjects.cpp
    )
endif()

# If a special translation file for events exists, compile it.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/objects/translateObjects.cpp")
    target_sources(${FSFW_TEST_TGT} PRIVATE
        events/translateEvents.cpp
    )
endif()