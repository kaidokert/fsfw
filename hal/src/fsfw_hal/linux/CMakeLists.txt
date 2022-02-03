if(FSFW_HAL_ADD_RASPBERRY_PI)
    add_subdirectory(rpi)
endif()

target_sources(${LIB_FSFW_NAME} PRIVATE
	UnixFileGuard.cpp
	utility.cpp
)

add_subdirectory(gpio)
add_subdirectory(spi)
add_subdirectory(i2c)
add_subdirectory(uart)
add_subdirectory(uio)
