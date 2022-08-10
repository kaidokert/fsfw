add_subdirectory(devicehandlers)
add_subdirectory(common)
add_subdirectory(host)

if(UNIX)
  add_subdirectory(linux)
endif()

if(FSFW_HAL_ADD_STM32H7)
  add_subdirectory(stm32h7)
endif()
