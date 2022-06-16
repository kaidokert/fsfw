if(DEFINED WIN32 OR DEFINED UNIX)
  target_sources(
    ${LIB_FSFW_NAME}
    PRIVATE tcpipCommon.cpp TcpIpBase.cpp UdpTcPollingTask.cpp
            UdpTmTcBridge.cpp TcpTmTcServer.cpp TcpTmTcBridge.cpp)
endif()

if(WIN32)
  target_link_libraries(${LIB_FSFW_NAME} PRIVATE wsock32 ws2_32)
endif()
