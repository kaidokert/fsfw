add_subdirectory(spi)
add_subdirectory(gpio)
add_subdirectory(devicetest)

target_sources(${LIB_FSFW_HAL_NAME} PRIVATE
    dma.cpp
)
