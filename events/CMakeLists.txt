target_sources(${LIB_FSFW_NAME} 
	PRIVATE 
		Event.cpp
		EventManager.cpp
		EventMessage.cpp
)

add_subdirectory(eventmatching)