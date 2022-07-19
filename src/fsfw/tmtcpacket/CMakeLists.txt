target_sources(
  ${LIB_FSFW_NAME} PRIVATE SpacePacketReader.cpp SpacePacketCreator.cpp
                           PacketStorageHelper.cpp)

add_subdirectory(cfdp)
add_subdirectory(packetmatcher)
add_subdirectory(pus)
