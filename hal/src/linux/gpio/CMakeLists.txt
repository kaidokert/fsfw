target_sources(${LIB_FSFW_HAL_NAME} PRIVATE
    LinuxLibgpioIF.cpp
)

# This abstraction layer requires the gpiod library. You can install this library
# with "sudo apt-get install -y libgpiod-dev". If you are cross-compiling, you need
# to install the package before syncing the sysroot to your host computer.
find_library(LIB_GPIO gpiod REQUIRED)

target_link_libraries(${LIB_FSFW_HAL_NAME} PRIVATE
    ${LIB_GPIO}
)