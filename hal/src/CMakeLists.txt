add_subdirectory(devicehandlers)
add_subdirectory(common)

if(FSFW_HAL_ADD_LINUX)
    add_subdirectory(${LINUX_HAL_PATH_NAME})
endif()

if(FSFW_HAL_ADD_STM32H7)
    add_subdirectory(${STM32H7_PATH_NAME})
endif()
