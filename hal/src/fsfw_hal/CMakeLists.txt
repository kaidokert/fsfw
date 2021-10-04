add_subdirectory(devicehandlers)
add_subdirectory(common)

if(FSFW_HAL_ADD_LINUX)
    add_subdirectory(linux)
endif()

if(FSFW_HAL_ADD_STM32H7)
    add_subdirectory(stm32h7)
endif()
