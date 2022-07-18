target_sources(${LIB_FSFW_NAME} PRIVATE InternalUnitTester.cpp
                                        UnittDefinitions.cpp)

add_subdirectory(osal)
add_subdirectory(serialize)
add_subdirectory(globalfunctions)
