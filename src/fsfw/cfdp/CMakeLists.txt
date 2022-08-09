target_sources(
  ${LIB_FSFW_NAME}
  PRIVATE CfdpMessage.cpp CfdpDistributor.cpp CfdpSourceHandler.cpp
          CfdpDestHandler.cpp FaultHandlerBase.cpp)

# CfdpDistributor.cpp CfdpHandler.cpp
add_subdirectory(pdu)
add_subdirectory(tlv)
