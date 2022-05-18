target_sources(
  ${LIB_FSFW_NAME}
  PRIVATE Clock.cpp
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
          QueueMapManager.cpp)

# FreeRTOS is required to link the FSFW now. It is recommended to compile
# FreeRTOS as a static library and set LIB_OS_NAME to the target name of the
# library.
if(NOT LIB_OS_NAME)
  message(
    STATUS
      "LIB_OS_NAME is empty. Make sure to include the FreeRTOS header path properly."
  )
else()
  target_link_libraries(${LIB_FSFW_NAME} PRIVATE ${LIB_OS_NAME})
endif()
