target_sources(${LIB_FSFW_NAME} PRIVATE CfdpHandler.cpp CfdpMessage.cpp)

add_subdirectory(pdu)
add_subdirectory(tlv)
