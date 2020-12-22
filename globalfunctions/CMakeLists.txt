target_sources(${LIB_FSFW_NAME} 
	PRIVATE 
		arrayprinter.cpp
		AsciiConverter.cpp
		CRC.cpp
		DleEncoder.cpp
		PeriodicOperationDivider.cpp
		timevalOperations.cpp
		Type.cpp
)

add_subdirectory(math)