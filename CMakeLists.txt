cmake_minimum_required(VERSION 3.13)

set(LIB_FSFW_NAME fsfw)
add_library(${LIB_FSFW_NAME})

option(FSFW_USE_RMAP "Compile with RMAP" ON)
option(FSFW_USE_DATALINKLAYER "Compile with Data Link Layer" ON)

add_subdirectory(action)
add_subdirectory(container)
add_subdirectory(controller)
add_subdirectory(coordinates)
add_subdirectory(datalinklayer)
add_subdirectory(datapool)
add_subdirectory(devicehandlers)
add_subdirectory(events)
add_subdirectory(fdir)
add_subdirectory(globalfunctions)
add_subdirectory(health)
add_subdirectory(internalError)
add_subdirectory(ipc)
add_subdirectory(memory)
add_subdirectory(modes)
add_subdirectory(monitoring)
add_subdirectory(objectmanager)
add_subdirectory(osal)
add_subdirectory(parameters)
add_subdirectory(power)
add_subdirectory(pus)

if(FSFW_USE_RMAP)
add_subdirectory(rmap)
endif()

add_subdirectory(serialize)
add_subdirectory(serviceinterface)
add_subdirectory(storagemanager)
add_subdirectory(subsystem)
add_subdirectory(tasks)
add_subdirectory(tcdistribution)
add_subdirectory(thermal)
add_subdirectory(timemanager)
add_subdirectory(tmstorage)
add_subdirectory(tmtcpacket)
add_subdirectory(tmtcservices)

# The project CMakeLists file has to set the FSFW_CONFIG_PATH and add it.
# If this is not given, we include the default configuration and emit a warning.
if(NOT FSFW_CONFIG_PATH)
message(WARNING "Flight Software Framework configuration path not set!")
message(WARNING "Setting default configuration!")
add_subdirectory(defaultcfg/fsfwconfig)
endif()

# Required include paths to compile the FSFW
target_include_directories(${LIB_FSFW_NAME} 
	INTERFACE 
		${FSFW_CONFIG_PATH}
)
