target_sources(${LIB_FSFW_NAME} PRIVATE SpacePacket.cpp SpacePacketBase.cpp)

add_subdirectory(cfdp)
add_subdirectory(packetmatcher)
add_subdirectory(pus)
