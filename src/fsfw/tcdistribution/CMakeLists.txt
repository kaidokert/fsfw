target_sources(
  ${LIB_FSFW_NAME}
  PRIVATE CcsdsDistributor.cpp
          PusDistributor.cpp
          TcDistributorBase.cpp
          PusPacketChecker.cpp
          TcPacketCheckCFDP.cpp
          CfdpDistributor.cpp
          CcsdsPacketChecker.cpp
          CcsdsUnpacker.cpp)
