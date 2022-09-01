target_sources(${FSFW_TEST_TGT} PRIVATE CatchDefinitions.cpp CatchFactory.cpp
                                        printChar.cpp testVersion.cpp)

target_sources(${FSFW_TEST_TGT} PRIVATE CatchRunner.cpp CatchSetup.cpp)

add_subdirectory(testcfg)
add_subdirectory(mocks)

add_subdirectory(tcdistributor)
add_subdirectory(action)
add_subdirectory(power)
add_subdirectory(util)
add_subdirectory(container)
add_subdirectory(osal)
add_subdirectory(pus)
add_subdirectory(serialize)
add_subdirectory(datapoollocal)
add_subdirectory(storagemanager)
add_subdirectory(globalfunctions)
add_subdirectory(timemanager)
add_subdirectory(tmtcpacket)
add_subdirectory(cfdp)
add_subdirectory(hal)
add_subdirectory(internalerror)
add_subdirectory(devicehandler)
add_subdirectory(tmtcservices)

target_include_directories(${FSFW_TEST_TGT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
