target_sources(
  ${LIB_FSFW_NAME}
  PRIVATE Clcw.cpp
          DataLinkLayer.cpp
          Farm1StateLockout.cpp
          Farm1StateOpen.cpp
          Farm1StateWait.cpp
          MapPacketExtraction.cpp
          TcTransferFrame.cpp
          TcTransferFrameLocal.cpp
          VirtualChannelReception.cpp)
