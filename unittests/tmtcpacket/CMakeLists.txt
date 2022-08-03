target_sources(
  ${FSFW_TEST_TGT}
  PRIVATE testCcsdsCreator.cpp
          testCcsdsReader.cpp
          testPusTcCreator.cpp
          testPusTcReader.cpp
          testPusTmCreator.cpp
          testPusTmReader.cpp
          testCcsds.cpp
          testZcTmWriter.cpp)
