cmake_minimum_required(VERSION 3.13)

# Can also be changed by upper CMakeLists.txt file
find_library(LIB_FSFW_NAME fsfw REQUIRED)

option(FSFW_HAL_ADD_LINUX "Add the Linux HAL to the sources. Required gpiod library" OFF)
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
