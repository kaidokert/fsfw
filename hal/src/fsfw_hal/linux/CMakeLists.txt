if(FSFW_HAL_ADD_RASPBERRY_PI)
    add_subdirectory(rpi)
endif()

target_sources(${LIB_FSFW_NAME} PRIVATE
	UnixFileGuard.cpp
	CommandExecutor.cpp
	utility.cpp
)

if(FSFW_HAL_LINUX_ADD_PERIPHERAL_DRIVERS)
if(FSFW_HAL_LINUX_ADD_LIBGPIOD)
	add_subdirectory(gpio)
endif()
	add_subdirectory(spi)
	add_subdirectory(i2c)
	add_subdirectory(uart)
endif()

add_subdirectory(uio)
