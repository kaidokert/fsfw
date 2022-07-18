target_sources(${LIB_FSFW_NAME} PRIVATE SpacePacketReader.cpp)

add_subdirectory(cfdp)
add_subdirectory(packetmatcher)
add_subdirectory(pus)
