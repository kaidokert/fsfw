target_sources(${FSFW_TEST_TGT} PRIVATE testCfdp.cpp testOtherTlvs.cpp
                                        testTlv.cpp testLvs.cpp)

add_subdirectory(handler)
add_subdirectory(pdu)
