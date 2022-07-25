# This abstraction layer requires the gpiod library. You can install this
# library with "sudo apt-get install -y libgpiod-dev". If you are
# cross-compiling, you need to install the package before syncing the sysroot to
# your host computer.
find_library(LIB_GPIO gpiod)

if(${LIB_GPIO} MATCHES LIB_GPIO-NOTFOUND)
  message(STATUS "gpiod library not found, not linking against it")
else()
  target_sources(${LIB_FSFW_NAME} PRIVATE LinuxLibgpioIF.cpp)
  target_link_libraries(${LIB_FSFW_NAME} PRIVATE ${LIB_GPIO})
endif()
