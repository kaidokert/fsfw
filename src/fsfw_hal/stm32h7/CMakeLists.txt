add_subdirectory(spi)
add_subdirectory(gpio)
add_subdirectory(devicetest)

target_sources(${LIB_FSFW_NAME} PRIVATE dma.cpp)
