target_sources(${LIB_FSFW_NAME} PRIVATE CfdpMessage.cpp CfdpDistributor.cpp)

add_subdirectory(pdu)
add_subdirectory(tlv)
add_subdirectory(handler)
