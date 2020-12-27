target_sources(${TARGET_NAME} PRIVATE
	InternalUnitTester.cpp
	UnittDefinitions.cpp
)

add_subdirectory(osal)
add_subdirectory(serialize)