target_sources(
  ${LIB_FSFW_NAME}
  PRIVATE Clock.cpp
          CpuUsage.cpp
          InitTask.cpp
          InternalErrorCodes.cpp
          MessageQueue.cpp
          PeriodicTask.cpp
          Mutex.cpp
          MutexFactory.cpp
          FixedTimeslotTask.cpp
          QueueFactory.cpp
          RtemsBasic.cpp
          RTEMSTaskBase.cpp
          TaskFactory.cpp
          BinarySemaphore.cpp
          SemaphoreFactory.cpp)
