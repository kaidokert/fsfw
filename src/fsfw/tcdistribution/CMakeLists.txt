target_sources(
  ${LIB_FSFW_NAME}
  PRIVATE CCSDSDistributor.cpp
          PusDistributor.cpp
          TcDistributor.cpp
          PusPacketChecker.cpp
          TcPacketCheckCFDP.cpp
          CFDPDistributor.cpp
          CcsdsPacketChecker.cpp)
