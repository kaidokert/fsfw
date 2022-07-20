target_sources(
  ${LIB_FSFW_NAME}
  PRIVATE CommandingServiceBase.cpp
          PusServiceBase.cpp
          PusVerificationReport.cpp
          TmTcBridge.cpp
          TmTcMessage.cpp
          VerificationReporter.cpp
          SpacePacketParser.cpp
          TmStoreHelper.cpp)
