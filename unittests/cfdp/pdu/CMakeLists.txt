target_sources(
  ${FSFW_TEST_TGT}
  PRIVATE testAckPdu.cpp
          testAckPdu.cpp
          testEofPdu.cpp
          testNakPdu.cpp
          testFinishedPdu.cpp
          testPromptPdu.cpp
          testKeepAlivePdu.cpp
          testMetadataPdu.cpp
          testFileData.cpp
          testCfdpHeader.cpp
          testFileDirective.cpp)
