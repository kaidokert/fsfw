target_sources(${TARGET_NAME} 
	PRIVATE
)

# Add include paths for the executable
target_include_directories(${TARGET_NAME} 
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}
)

# Add include paths for the FSFW library
target_include_directories(${LIB_FSFW_NAME} 
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)