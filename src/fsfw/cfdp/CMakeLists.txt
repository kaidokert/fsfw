target_sources(${LIB_FSFW_NAME} PRIVATE
    CFDPHandler.cpp
    CFDPMessage.cpp
)

add_subdirectory(pdu)
add_subdirectory(tlv)
