cmake_minimum_required(VERSION 3.13)

set(FSFW_VERSION 4)
set(FSFW_SUBVERSION 0)
set(FSFW_REVISION 0)

# Add the cmake folder so the FindSphinx module is found
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(FSFW_ETL_LIB_MAJOR_VERSION 20 CACHE STRING
    "ETL library major version requirement"
)
set(FSFW_ETL_LIB_VERSION ${FSFW_ETL_LIB_MAJOR_VERSION}.27.3 CACHE STRING
    "ETL library exact version requirement"
)
set(FSFW_ETL_LINK_TARGET etl::etl)

set(FSFW_CATCH2_LIB_MAJOR_VERSION 3 CACHE STRING
    "Catch2 library major version requirement"
)
set(FSFW_CATCH2_LIB_VERSION v${FSFW_CATCH2_LIB_MAJOR_VERSION}.0.0-preview5 CACHE STRING
    "Catch2 library exact version requirement"
)

set(FSFW_ETL_LIB_NAME etl)

option(FSFW_GENERATE_SECTIONS
    "Generate function and data sections. Required to remove unused code" ON
)
if(FSFW_GENERATE_SECTIONS)
    option(FSFW_REMOVE_UNUSED_CODE "Remove unused code" ON)
endif()

option(FSFW_BUILD_UNITTESTS "Build unittest binary in addition to static library" OFF)
option(FSFW_BUILD_DOCS "Build documentation with Sphinx and Doxygen" OFF)
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
set(FSFW_DUMMY_TGT fsfw-dummy)

project(${LIB_FSFW_NAME})
add_library(${LIB_FSFW_NAME})

if(FSFW_BUILD_UNITTESTS)
    message(STATUS "Building the FSFW unittests in addition to the static library")
    # Check whether the user has already installed Catch2 first
    find_package(Catch2 ${FSFW_CATCH2_LIB_MAJOR_VERSION})
    # Not installed, so use FetchContent to download and provide Catch2
    if(NOT Catch2_FOUND)
        message(STATUS "Catch2 installation not found. Downloading Catch2 library with FetchContent")
        include(FetchContent)

        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG        ${FSFW_CATCH2_LIB_VERSION}
        )

        FetchContent_MakeAvailable(Catch2)
        # fixes regression -preview4, to be confirmed in later releases
        # Related GitHub issue: https://github.com/catchorg/Catch2/issues/2417
        set_target_properties(Catch2 PROPERTIES DEBUG_POSTFIX "")
    endif()

    set(FSFW_CONFIG_PATH tests/src/fsfw_tests/unit/testcfg)
    configure_file(tests/src/fsfw_tests/unit/testcfg/FSFWConfig.h.in FSFWConfig.h)
    configure_file(tests/src/fsfw_tests/unit/testcfg/TestsConfig.h.in tests/TestsConfig.h)

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

message(STATUS "Finding and/or providing ETL library")

# Check whether the user has already installed ETL first
find_package(${FSFW_ETL_LIB_NAME} ${FSFW_ETL_LIB_MAJOR_VERSION} QUIET)
# Not installed, so use FetchContent to download and provide etl
if(NOT ${FSFW_ETL_LIB_NAME}_FOUND)
    message(STATUS
        "No ETL installation was found with find_package. Installing and providing "
        "etl with FindPackage"
    )
    include(FetchContent)

    FetchContent_Declare(
        ${FSFW_ETL_LIB_NAME}
        GIT_REPOSITORY https://github.com/ETLCPP/etl
        GIT_TAG        ${FSFW_ETL_LIB_VERSION}
    )

    FetchContent_MakeAvailable(${FSFW_ETL_LIB_NAME})
    add_library(${FSFW_ETL_LINK_TARGET} ALIAS ${FSFW_ETL_LIB_NAME})
endif()

set(FSFW_CORE_INC_PATH "inc")

set_property(CACHE FSFW_OSAL PROPERTY STRINGS host linux rtems freertos)

# For configure files
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

configure_file(src/fsfw/FSFW.h.in fsfw/FSFW.h)
configure_file(src/fsfw/FSFWVersion.h.in fsfw/FSFWVersion.h)

message(STATUS "Compiling FSFW for the ${FSFW_OS_NAME} operating system.")

add_subdirectory(src)
add_subdirectory(tests)
if(FSFW_ADD_HAL)
    add_subdirectory(hal)
endif()
add_subdirectory(contrib)
if(FSFW_BUILD_DOCS)
    add_subdirectory(docs)
endif()

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
    set(DEF_CONF_PATH misc/defaultcfg/fsfwconfig)
    if(NOT FSFW_BUILD_DOCS)
        message(WARNING "Flight Software Framework configuration path not set!")
        message(WARNING "Setting default configuration from ${DEF_CONF_PATH} ..")
    endif()
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
          -Wduplicated-cond # check for duplicate conditions
          -Wduplicated-branches # check for duplicate branches
          -Wlogical-op # Search for bitwise operations instead of logical 
          -Wnull-dereference # Search for NULL dereference
          -Wundef # Warn if undefind marcos are used
          -Wformat=2 # Format string problem detection
          -Wformat-overflow=2 # Formatting issues in printf
          -Wformat-truncation=2 # Formatting issues in printf
          -Wformat-security # Search for dangerous printf operations
          -Wstrict-overflow=3 # Warn if integer overflows might happen
          -Warray-bounds=2 # Some array bounds violations will be found
          -Wshift-overflow=2 # Search for bit left shift overflows (<c++14)
          -Wcast-qual # Warn if the constness is cast away
          -Wstringop-overflow=4
          # -Wstack-protector # Emits a few false positives for low level access
          # -Wconversion # Creates many false positives
          # -Warith-conversion # Use with Wconversion to find more implicit conversions
          # -fanalyzer # Should be used to look through problems
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
    ${FSFW_ETL_LINK_TARGET}
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
