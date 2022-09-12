target_sources(${FSFW_TEST_TGT} PRIVATE
	PowerSwitcherMock.cpp
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
)
