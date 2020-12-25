target_sources(${LIB_FSFW_NAME} 
	PRIVATE 
		Clock.cpp
		BinarySemaphore.cpp
		CountingSemaphore.cpp
		FixedTimeslotTask.cpp
		InternalErrorCodes.cpp
		MessageQueue.cpp
		Mutex.cpp
		MutexFactory.cpp
		PeriodicPosixTask.cpp
		PosixThread.cpp
		QueueFactory.cpp
		SemaphoreFactory.cpp
		TaskFactory.cpp
		TcUnixUdpPollingTask.cpp
		TmTcUnixUdpBridge.cpp
		Timer.cpp
)

target_link_libraries(${LIB_FSFW_NAME} PRIVATE
	${CMAKE_THREAD_LIBS_INIT}
	rt
)

