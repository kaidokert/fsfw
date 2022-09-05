target_sources(
  ${FSFW_TEST_TGT}
  PRIVATE PowerSwitcherMock.cpp
          DeviceHandlerMock.cpp
          DeviceFdirMock.cpp
          CookieIFMock.cpp
          ComIFMock.cpp
          MessageQueueMock.cpp
          InternalErrorReporterMock.cpp
          LocalPoolOwnerBase.cpp
          PusVerificationReporterMock.cpp
          PusServiceBaseMock.cpp
          AcceptsTmMock.cpp
          PusDistributorMock.cpp
          CcsdsCheckerMock.cpp
          AcceptsTcMock.cpp
          StorageManagerMock.cpp
          FilesystemMock.cpp
          EventReportingProxyMock.cpp)

add_subdirectory(cfdp)
