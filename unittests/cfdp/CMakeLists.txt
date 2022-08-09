target_sources(${FSFW_TEST_TGT} PRIVATE testCfdp.cpp testTlvsLvs.cpp)

add_subdirectory(handler)
add_subdirectory(pdu)
