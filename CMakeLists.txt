cmake_minimum_required(VERSION 3.13)

set(FSFW_VERSION 2)
set(FSFW_SUBVERSION 0)
set(FSFW_REVISION 0)

option(FSFW_GENERATE_SECTIONS
    "Generate function and data sections. Required to remove unused code" ON
)
if(FSFW_GENERATE_SECTIONS)
    option(FSFW_REMOVE_UNUSED_CODE "Remove unused code" ON)
endif()

option(FSFW_BUILD_UNITTESTS "Build unittest binary in addition to static library" OFF)
if(FSFW_BUILD_UNITTESTS)
    option(FSFW_TESTS_GEN_COV "Generate coverage data for unittests" ON)
endif()

option(FSFW_WARNING_SHADOW_LOCAL_GCC "Enable -Wshadow=local warning in GCC" ON)
# Options to exclude parts of the FSFW from compilation.
option(FSFW_ADD_INTERNAL_TESTS "Add internal unit tests" ON)
option(FSFW_ADD_UNITTESTS "Add regular unittests. Requires Catch2" OFF)
option(FSFW_ADD_HAL "Add Hardware Abstraction Layer" ON)

# Optional sources
option(FSFW_ADD_PUS "Compile with PUS sources" ON)
option(FSFW_ADD_MONITORING "Compile with monitoring components" ON)

option(FSFW_ADD_RMAP "Compile with RMAP" OFF)
option(FSFW_ADD_DATALINKLAYER "Compile with Data Link Layer" OFF)
option(FSFW_ADD_COORDINATES "Compile with coordinate components" OFF)
option(FSFW_ADD_TMSTORAGE "Compile with tm storage components" OFF)

# Contrib sources
option(FSFW_ADD_SGP4_PROPAGATOR "Add SGP4 propagator code" OFF)

set(LIB_FSFW_NAME fsfw)
set(FSFW_TEST_TGT fsfw-tests)

add_library(${LIB_FSFW_NAME})

if(FSFW_BUILD_UNITTESTS)
    message(STATUS "Building the FSFW unittests in addition to the static library")
    # Check whether the user has already installed Catch2 first
    find_package(Catch2 3)
    # Not installed, so use FetchContent to download and provide Catch2
    if(NOT Catch2_FOUND)
        include(FetchContent)

        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG        v3.0.0-preview3
        )

        FetchContent_MakeAvailable(Catch2)
    endif()

    set(FSFW_CONFIG_PATH tests/src/fsfw_tests/unit/testcfg)
    configure_file(tests/src/fsfw_tests/unit/testcfg/FSFWConfig.h.in FSFWConfig.h)
    configure_file(tests/src/fsfw_tests/unit/testcfg/TestsConfig.h.in tests/TestsConfig.h)
    configure_file(tests/src/fsfw_tests/unit/testcfg/OBSWConfig.h.in OBSWConfig.h)

    project(${FSFW_TEST_TGT} CXX C)
    add_executable(${FSFW_TEST_TGT})

    if(FSFW_TESTS_GEN_COV)
        message(STATUS "Generating coverage data for the library")
        message(STATUS "Targets linking against ${LIB_FSFW_NAME} "
            "will be compiled with coverage data as well"
        )
        include(FetchContent)
        FetchContent_Declare(
            cmake-modules
            GIT_REPOSITORY https://github.com/bilke/cmake-modules.git
        )
        FetchContent_MakeAvailable(cmake-modules)
        set(CMAKE_BUILD_TYPE "Debug")
        list(APPEND CMAKE_MODULE_PATH ${cmake-modules_SOURCE_DIR})
        include(CodeCoverage)
    endif()
endif()

set(FSFW_CORE_INC_PATH "inc")

set_property(CACHE FSFW_OSAL PROPERTY STRINGS host linux rtems freertos)

# Configure Files
target_include_directories(${LIB_FSFW_NAME} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_include_directories(${LIB_FSFW_NAME} INTERFACE
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED True)
elseif(${CMAKE_CXX_STANDARD} LESS 11)
    message(FATAL_ERROR "Compiling the FSFW requires a minimum of C++11 support")
endif()

# Backwards comptability
if(OS_FSFW AND NOT FSFW_OSAL)
    message(WARNING "Please pass the FSFW OSAL as FSFW_OSAL instead of OS_FSFW")
    set(FSFW_OSAL OS_FSFW)
endif()

if(NOT FSFW_OSAL)
	message(STATUS "No OS for FSFW via FSFW_OSAL set. Assuming host OS")
	# Assume host OS and autodetermine from OS_FSFW
	if(UNIX)
		set(FSFW_OSAL  "linux" 
			CACHE STRING 
			"OS abstraction layer used in the FSFW"
		)
	elseif(WIN32)
		set(FSFW_OSAL  "host" 
			CACHE STRING "OS abstraction layer used in the FSFW"
		)
	endif()

endif()

set(FSFW_OSAL_DEFINITION FSFW_OSAL_HOST)

if(FSFW_OSAL MATCHES host)
    set(FSFW_OS_NAME "Host")
    set(FSFW_OSAL_HOST ON)
elseif(FSFW_OSAL MATCHES linux)
    set(FSFW_OS_NAME "Linux")
    set(FSFW_OSAL_LINUX ON)
elseif(FSFW_OSAL MATCHES freertos)
    set(FSFW_OS_NAME "FreeRTOS")
    set(FSFW_OSAL_FREERTOS ON)
    target_link_libraries(${LIB_FSFW_NAME} PRIVATE
        ${LIB_OS_NAME}
    )
elseif(FSFW_OSAL STREQUAL rtems)
    set(FSFW_OS_NAME "RTEMS")
    set(FSFW_OSAL_RTEMS ON)
else()
    message(WARNING
        "Invalid operating system for FSFW specified! Setting to host.."
    )
    set(FSFW_OS_NAME "Host")
    set(OS_FSFW "host")
endif()

if(FSFW_BUILD_UNITTESTS)
    configure_file(src/fsfw/FSFW.h.in fsfw/FSFW.h)
    configure_file(src/fsfw/FSFWVersion.h.in fsfw/FSFWVersion.h)
else()
    configure_file(src/fsfw/FSFW.h.in FSFW.h)
    configure_file(src/fsfw/FSFWVersion.h.in FSFWVersion.h)
endif()

message(STATUS "Compiling FSFW for the ${FSFW_OS_NAME} operating system.")

add_subdirectory(src)
add_subdirectory(tests)
if(FSFW_ADD_HAL)
    add_subdirectory(hal)
endif()
add_subdirectory(contrib)

if(FSFW_BUILD_UNITTESTS)
    if(FSFW_TESTS_GEN_COV)
        if(CMAKE_COMPILER_IS_GNUCXX)
            include(CodeCoverage)

            # Remove quotes.
            separate_arguments(COVERAGE_COMPILER_FLAGS
                NATIVE_COMMAND "${COVERAGE_COMPILER_FLAGS}"
            )

            # Add compile options manually, we don't want coverage for Catch2
            target_compile_options(${FSFW_TEST_TGT} PRIVATE
                "${COVERAGE_COMPILER_FLAGS}"
            )
            target_compile_options(${LIB_FSFW_NAME} PRIVATE
                "${COVERAGE_COMPILER_FLAGS}"
            )

            # Exclude directories here
            if(WIN32)
                set(GCOVR_ADDITIONAL_ARGS
                    "--exclude-throw-branches"
                    "--exclude-unreachable-branches"
                )
                set(COVERAGE_EXCLUDES
                    "/c/msys64/mingw64/*" "*/fsfw_hal/*"
                )
            elseif(UNIX)
                set(COVERAGE_EXCLUDES
                    "/usr/include/*" "/usr/bin/*" "Catch2/*"
                    "/usr/local/include/*" "*/fsfw_tests/*"
                    "*/catch2-src/*" "*/fsfw_hal/*"
                )
            endif()

            target_link_options(${FSFW_TEST_TGT} PRIVATE
                -fprofile-arcs
                -ftest-coverage
            )
            target_link_options(${LIB_FSFW_NAME} PRIVATE
                -fprofile-arcs
                -ftest-coverage
            )
            # Need to specify this as an interface, otherwise there will the compile issues
            target_link_options(${LIB_FSFW_NAME} INTERFACE
                -fprofile-arcs
                -ftest-coverage
            )

            if(WIN32)
                setup_target_for_coverage_gcovr_html(
                    NAME ${FSFW_TEST_TGT}_coverage
                    EXECUTABLE ${FSFW_TEST_TGT}
                    DEPENDENCIES ${FSFW_TEST_TGT}
                )
            else()
                setup_target_for_coverage_lcov(
                    NAME ${FSFW_TEST_TGT}_coverage
                    EXECUTABLE ${FSFW_TEST_TGT}
                    DEPENDENCIES ${FSFW_TEST_TGT}
                )
            endif()
        endif()
    endif()
    target_link_libraries(${FSFW_TEST_TGT} PRIVATE Catch2::Catch2 ${LIB_FSFW_NAME})
endif()

# The project CMakeLists file has to set the FSFW_CONFIG_PATH and add it.
# If this is not given, we include the default configuration and emit a warning.
if(NOT FSFW_CONFIG_PATH)
    message(WARNING "Flight Software Framework configuration path not set!")
    set(DEF_CONF_PATH misc/defaultcfg/fsfwconfig)
    message(WARNING "Setting default configuration from ${DEF_CONF_PATH} ..")
    add_subdirectory(${DEF_CONF_PATH})
    set(FSFW_CONFIG_PATH ${DEF_CONF_PATH})
endif()

# FSFW might be part of a possibly complicated folder structure, so we 
# extract the absolute path of the fsfwconfig folder.
if(IS_ABSOLUTE ${FSFW_CONFIG_PATH})
	set(FSFW_CONFIG_PATH_ABSOLUTE ${FSFW_CONFIG_PATH})
else()
	get_filename_component(FSFW_CONFIG_PATH_ABSOLUTE
		${FSFW_CONFIG_PATH} REALPATH BASE_DIR ${CMAKE_SOURCE_DIR}
	)
endif()

foreach(INCLUDE_PATH ${FSFW_ADDITIONAL_INC_PATHS})
    if(IS_ABSOLUTE ${INCLUDE_PATH})
        set(CURR_ABS_INC_PATH "${INCLUDE_PATH}")
    else()
        get_filename_component(CURR_ABS_INC_PATH
            ${INCLUDE_PATH} REALPATH BASE_DIR ${CMAKE_SOURCE_DIR})
    endif()

    if(CMAKE_VERBOSE)
        message(STATUS "FSFW include path: ${CURR_ABS_INC_PATH}")
    endif()

    list(APPEND FSFW_ADD_INC_PATHS_ABS ${CURR_ABS_INC_PATH})
endforeach()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(NOT DEFINED FSFW_WARNING_FLAGS)
	   set(FSFW_WARNING_FLAGS
		  -Wall
		  -Wextra
		  -Wimplicit-fallthrough=1
		  -Wno-unused-parameter
		  -Wno-psabi
        )
    endif()

    if(FSFW_GENERATE_SECTIONS)
        target_compile_options(${LIB_FSFW_NAME} PRIVATE
            "-ffunction-sections"
            "-fdata-sections"
        )
    endif()

    if(FSFW_REMOVE_UNUSED_CODE)
        target_link_options(${LIB_FSFW_NAME} PRIVATE
            "Wl,--gc-sections"
        )
    endif()
  
    if(FSFW_WARNING_SHADOW_LOCAL_GCC)
        list(APPEND WARNING_FLAGS "-Wshadow=local")
    endif()
	
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(COMPILER_FLAGS "/permissive-")
endif()

# Required include paths to compile the FSFW
target_include_directories(${LIB_FSFW_NAME} INTERFACE 
	${CMAKE_SOURCE_DIR}
	${FSFW_CONFIG_PATH_ABSOLUTE}
	${FSFW_CORE_INC_PATH}
	${FSFW_ADD_INC_PATHS_ABS}
)

# Includes path required to compile FSFW itself as well
# We assume that the fsfwconfig folder uses include relative to the project
# root here!
target_include_directories(${LIB_FSFW_NAME} PRIVATE 
	${CMAKE_SOURCE_DIR}
	${FSFW_CONFIG_PATH_ABSOLUTE}
	${FSFW_CORE_INC_PATH}
	${FSFW_ADD_INC_PATHS_ABS}
)

target_compile_options(${LIB_FSFW_NAME} PRIVATE 
	${FSFW_WARNING_FLAGS}
	${COMPILER_FLAGS}
)

target_link_libraries(${LIB_FSFW_NAME} PRIVATE
    ${FSFW_ADDITIONAL_LINK_LIBS}
)

string(CONCAT POST_BUILD_COMMENT
    "######################################################################\n"
    "Built FSFW v${FSFW_VERSION}.${FSFW_SUBVERSION}.${FSFW_REVISION}, "
    "Target OSAL: ${FSFW_OS_NAME}\n"
    "######################################################################\n"
)

add_custom_command(
   TARGET ${LIB_FSFW_NAME}
   POST_BUILD
   COMMENT ${POST_BUILD_COMMENT}
)
