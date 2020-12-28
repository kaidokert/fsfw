################################################################################
# CMake support for the Flight Software Framework Tests
# 
# Developed in an effort to replace Make with a modern build system.
# 
# Author: R. Mueller
################################################################################

################################################################################
# Pre-Project preparation
################################################################################
cmake_minimum_required(VERSION 3.13)

# set(CMAKE_VERBOSE TRUE)

set(CMAKE_SCRIPT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/buildsystem/cmake")

# Tests can be built with the Host OSAL or with the Linux OSAL.
if(NOT OS_FSFW)
	set(OS_FSFW host CACHE STRING "OS for the FSFW.")
endif()

option(CUSTOM_UNITTEST_RUNNER 
	"Specify whether custom main or Catch2 main is used" TRUE
)

# Perform steps like loading toolchain files where applicable.
#include(${CMAKE_SCRIPT_PATH}/PreProjectConfig.cmake)
#pre_project_config()

# Project Name
project(fsfw_tests C CXX)

################################################################################
# Pre-Sources preparation
################################################################################

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set names and variables
set(TARGET_NAME ${CMAKE_PROJECT_NAME}) 
if(CUSTOM_UNITTEST_RUNNER)
	set(CATCH2_TARGET Catch2)
else()
	set(CATCH2_TARGET Catch2WithMain)
endif()
set(LIB_FSFW_NAME fsfw)

# Set path names
set(FSFW_PATH fsfw)
set(CATCH2_PATH Catch2)
set(FSFW_TESTS_PATH fsfw/unittest)
set(TEST_SETUP_PATH unittest)

# Analyse different OS and architecture/target options and
# determine BSP_PATH

# FreeRTOS
if(${OS_FSFW} STREQUAL linux)
	add_definitions(-DUNIX -DLINUX)	
	find_package(Threads REQUIRED)
# Hosted
else()
	if(WIN32)
    	add_definitions(-DWIN32)
	elseif(UNIX)
		find_package(Threads REQUIRED)
		add_definitions(-DUNIX -DLINUX)	
	endif()
endif()

set(FSFW_CONFIG_PATH testcfg)

################################################################################
# Executable and Sources
################################################################################

# Add executable
add_executable(${TARGET_NAME})

# Add subdirectories
add_subdirectory(${FSFW_PATH})
add_subdirectory(${CATCH2_PATH})
add_subdirectory(${FSFW_CONFIG_PATH})
add_subdirectory(${FSFW_TESTS_PATH})
add_subdirectory(${TEST_SETUP_PATH})

################################################################################
# Post-Sources preparation
################################################################################

# Add libraries for all sources.
target_link_libraries(${TARGET_NAME} PRIVATE
	${LIB_FSFW_NAME} 
	${CATCH2_TARGET}
)

# Add include paths for all sources.
target_include_directories(${TARGET_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${FSFW_CONFIG_PATH}
)	

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(WARNING_FLAGS
		-Wall
		-Wextra
		-Wshadow=local
		-Wimplicit-fallthrough=1
		-Wno-unused-parameter
		-Wno-psabi
	)
	
	# Remove unused sections.
	target_compile_options(${TARGET_NAME} PRIVATE
		"-ffunction-sections"
		"-fdata-sections"
	)

	# Removed unused sections.
	target_link_options(${TARGET_NAME} PRIVATE
		"-Wl,--gc-sections"
	)
	
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(COMPILER_FLAGS "/permissive-")
endif()

if(CMAKE_VERBOSE)
	message(STATUS "Warning flags: ${WARNING_FLAGS}")
endif()
		 

# Compile options for all sources.
target_compile_options(${TARGET_NAME} PRIVATE
	$<$<COMPILE_LANGUAGE:C>:${WARNING_FLAGS} ${COMPILER_FLAGS}>
	$<$<COMPILE_LANGUAGE:CXX>:${WARNING_FLAGS} ${COMPILER_FLAGS}>
	${ABI_FLAGS}
)

if(NOT CMAKE_SIZE)
	set(CMAKE_SIZE size)
	if(WIN32)
		set(FILE_SUFFIX ".exe")
	endif()
endif()

add_custom_command(
   TARGET ${TARGET_NAME}
   POST_BUILD
   COMMAND echo "Build directory: ${CMAKE_BINARY_DIR}"
   COMMAND echo "Target OSAL: ${OS_FSFW}"
   COMMAND echo "Target Build Type: ${CMAKE_BUILD_TYPE}"
   COMMAND ${CMAKE_SIZE} ${TARGET_NAME}${FILE_SUFFIX}
)

include (${CMAKE_CURRENT_SOURCE_DIR}/buildsystem/cmake/BuildType.cmake)
set_build_type()





