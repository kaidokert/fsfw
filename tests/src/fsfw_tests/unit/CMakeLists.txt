target_sources(${FSFW_TEST_TGT} PRIVATE
    CatchDefinitions.cpp
    CatchFactory.cpp
    printChar.cpp
)

# if(FSFW_CUSTOM_UNITTEST_RUNNER)
    target_sources(${FSFW_TEST_TGT} PRIVATE
        CatchRunner.cpp
        CatchSetup.cpp
    )
# endif()

add_subdirectory(testcfg)
add_subdirectory(action)
add_subdirectory(container)
add_subdirectory(osal)
add_subdirectory(serialize)
add_subdirectory(datapoollocal)
add_subdirectory(storagemanager)
add_subdirectory(globalfunctions)
add_subdirectory(timemanager)
add_subdirectory(tmtcpacket)
add_subdirectory(cfdp)
