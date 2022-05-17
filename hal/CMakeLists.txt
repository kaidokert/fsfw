cmake_minimum_required(VERSION 3.13)

# Can also be changed by upper CMakeLists.txt file
find_library(LIB_FSFW_NAME fsfw REQUIRED)

option(FSFW_HAL_ADD_LINUX "Add the Linux HAL to the sources. Requires gpiod library" OFF)
# On by default for now because I did not have an issue including and compiling those files
# and libraries on a Desktop Linux system and the primary target of the FSFW is still embedded
# Linux. The only exception from this is the gpiod library which requires a dedicated installation,
# but CMake is able to determine whether this library is installed with find_library.
option(FSFW_HAL_LINUX_ADD_PERIPHERAL_DRIVERS "Add peripheral drivers for embedded Linux" ON)
option(FSFW_HAL_LINUX_ADD_LIBGPIOD "Target implements libgpiod" ON)

option(FSFW_HAL_ADD_RASPBERRY_PI "Add Raspberry Pi specific code to the sources" OFF)
option(FSFW_HAL_ADD_STM32H7 "Add the STM32H7 HAL to the sources" OFF)
option(FSFW_HAL_WARNING_SHADOW_LOCAL_GCC "Enable -Wshadow=local warning in GCC" ON)

set(LINUX_HAL_PATH_NAME linux)
set(STM32H7_PATH_NAME stm32h7)

add_subdirectory(src)

foreach(INCLUDE_PATH ${FSFW_HAL_ADDITIONAL_INC_PATHS})
    if(IS_ABSOLUTE ${INCLUDE_PATH})
        set(CURR_ABS_INC_PATH "${INCLUDE_PATH}")
    else()
        get_filename_component(CURR_ABS_INC_PATH
            ${INCLUDE_PATH} REALPATH BASE_DIR ${CMAKE_SOURCE_DIR})
    endif()

    if(CMAKE_VERBOSE)
        message(STATUS "FSFW include path: ${CURR_ABS_INC_PATH}")
    endif()

    list(APPEND FSFW_HAL_ADD_INC_PATHS_ABS ${CURR_ABS_INC_PATH})
endforeach()

target_include_directories(${LIB_FSFW_NAME} PRIVATE
    ${FSFW_HAL_ADD_INC_PATHS_ABS}
)

target_compile_definitions(${LIB_FSFW_NAME} PRIVATE
    ${FSFW_HAL_DEFINES}
)

target_link_libraries(${LIB_FSFW_NAME} PRIVATE
    ${FSFW_HAL_LINK_LIBS}
)
