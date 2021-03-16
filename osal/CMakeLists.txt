# Check the OS_FSFW variable
if(${OS_FSFW} STREQUAL "freertos")
	add_subdirectory(FreeRTOS)
elseif(${OS_FSFW} STREQUAL "rtems")
	add_subdirectory(rtems)
elseif(${OS_FSFW} STREQUAL "linux")
	add_subdirectory(linux)
elseif(${OS_FSFW} STREQUAL "host")
	add_subdirectory(host)
	if (WIN32)
		add_subdirectory(windows)
	elseif(UNIX)
		target_sources(${LIB_FSFW_NAME}
			PUBLIC
				linux/TcUnixUdpPollingTask.cpp
				linux/TmTcUnixUdpBridge.cpp
		)
	endif ()

else()

	message(WARNING "The OS_FSFW variable was not set. Assuming host OS..")
	# Not set. Assumuing this is a host build, try to determine host OS
	if (WIN32)
		add_subdirectory(host)
		add_subdirectory(windows)
	elseif (UNIX)
		add_subdirectory(linux)
	else ()
		# MacOS or other OSes have not been tested yet / are not supported.
		message(FATAL_ERROR "The host OS could not be determined! Aborting.")
	endif()

endif()

add_subdirectory(common)