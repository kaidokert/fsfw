target_sources(
  ${LIB_FSFW_NAME}
  PRIVATE Service1TelecommandVerification.cpp
          Service2DeviceAccess.cpp
          Service3Housekeeping.cpp
          Service5EventReporting.cpp
          Service8FunctionManagement.cpp
          Service9TimeManagement.cpp
          Service17Test.cpp
          Service20ParameterManagement.cpp
          CService200ModeCommanding.cpp
          CService201HealthCommanding.cpp)
