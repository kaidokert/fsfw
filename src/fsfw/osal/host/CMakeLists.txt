target_sources(
  ${LIB_FSFW_NAME}
  PRIVATE Clock.cpp
          FixedTimeslotTask.cpp
          MessageQueue.cpp
          Mutex.cpp
          MutexFactory.cpp
          PeriodicTask.cpp
          QueueFactory.cpp
          QueueMapManager.cpp
          SemaphoreFactory.cpp
          TaskFactory.cpp
          taskHelpers.cpp)

if(UNIX)
  find_package(Threads REQUIRED)

  target_link_libraries(${LIB_FSFW_NAME} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
  if(NOT APPLE)
    target_link_libraries(${LIB_FSFW_NAME} PRIVATE rt)
  endif()

endif()
