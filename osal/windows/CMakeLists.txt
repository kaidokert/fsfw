target_sources(${LIB_FSFW_NAME} PRIVATE 
    TcWinUdpPollingTask.cpp
    TmTcWinUdpBridge.cpp
    TcWinTcpServer.cpp
)

target_link_libraries(${LIB_FSFW_NAME} PRIVATE
    wsock32 
    ws2_32
)