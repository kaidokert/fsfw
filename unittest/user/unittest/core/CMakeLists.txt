target_sources(${TARGET_NAME} PRIVATE
	CatchDefinitions.cpp
	CatchFactory.cpp
	CatchRunner.cpp
	CatchSetup.cpp
	printChar.cpp
)

if(CUSTOM_UNITTEST_RUNNER)
	target_sources(${TARGET_NAME} PRIVATE
		CatchRunner.cpp
	)
endif()