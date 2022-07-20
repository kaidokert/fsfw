target_sources(${LIB_FSFW_NAME} PRIVATE PacketStorageHelper.cpp)

add_subdirectory(ccsds)
add_subdirectory(pus)
add_subdirectory(cfdp)
add_subdirectory(packetmatcher)
