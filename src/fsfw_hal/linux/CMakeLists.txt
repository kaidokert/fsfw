if(FSFW_HAL_ADD_RASPBERRY_PI)
  add_subdirectory(rpi)
endif()

target_sources(${LIB_FSFW_NAME} PRIVATE UnixFileGuard.cpp CommandExecutor.cpp
                                        utility.cpp)

if(FSFW_HAL_LINUX_ADD_PERIPHERAL_DRIVERS)
  if(FSFW_HAL_LINUX_ADD_LIBGPIOD)
    add_subdirectory(gpio)
  endif()
  add_subdirectory(uart)
  # Adding those does not really make sense on Apple systems which are generally
  # host systems. It won't even compile as the headers are missing
  if(NOT APPLE)
    add_subdirectory(i2c)
    add_subdirectory(spi)
  endif()
endif()

add_subdirectory(uio)
