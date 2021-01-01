target_sources(${TARGET_NAME} 
	PRIVATE
)

# Add include paths for the executable
target_include_directories(${TARGET_NAME} 
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}
)

if(NOT DEFINED FSFW_CONFIG_DIRECTORY)
	set(FSFW_CONFIG_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()