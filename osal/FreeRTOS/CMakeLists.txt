target_sources(${LIB_FSFW_NAME} 
	PRIVATE 
		Clock.cpp
		FixedTimeslotTask.cpp
		BinarySemaphore.cpp
		BinSemaphUsingTask.cpp
		CountingSemaphore.cpp
		CountingSemaphUsingTask.cpp
		MessageQueue.cpp
		Mutex.cpp
		MutexFactory.cpp
		PeriodicTask.cpp
		QueueFactory.cpp
		SemaphoreFactory.cpp
		TaskFactory.cpp
		Timekeeper.cpp
		TaskManagement.cpp
)

# FreeRTOS is required to link the FSFW now. It is recommended to compile 
# FreeRTOS as a static library and set LIB_OS_NAME to the target name of the 
# library.
if(NOT LIB_OS_NAME)
	message(FATAL_ERROR 
		"FreeRTOS needs to be linked as a target and "
		"LIB_OS_NAME needs to be set to the target"
	)
endif()

target_link_libraries(${LIB_FSWFW_NAME} ${LIB_OS_NAME})