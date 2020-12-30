target_sources(${LIB_FSFW_NAME} PRIVATE
	ipc/missionMessageTypes.cpp
	objects/FsfwFactory.cpp
	pollingsequence/PollingSequenceFactory.cpp
)

# Should be added to include path
target_include_directories(${TARGET_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

if(NOT FSFW_CONFIG_PATH)
	set(FSFW_CONFIG_PATH ${CMAKE_CURRENT_SOURCE_DIR})
endif()
