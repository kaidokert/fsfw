target_sources(
  ${LIB_FSFW_NAME}
  PRIVATE spiCore.cpp
          spiDefinitions.cpp
          spiInterrupts.cpp
          mspInit.cpp
          SpiCookie.cpp
          SpiComIF.cpp
          stm32h743zi.cpp)
