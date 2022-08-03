target_sources(${LIB_FSFW_NAME} PRIVATE CfdpMessage.cpp CfdpRouter.cpp)

# CfdpDistributor.cpp CfdpHandler.cpp
add_subdirectory(pdu)
add_subdirectory(tlv)
