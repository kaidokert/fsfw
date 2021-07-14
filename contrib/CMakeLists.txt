if(FSFW_ADD_SPG4_PROPAGATOR)
    target_sources(${LIB_FSFW_NAME} PRIVATE
        spg4unit.cpp
    )
    target_include_directories(${LIB_FSFW_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/spg4
    )
    target_include_directories(${LIB_FSFW_NAME} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/spg4
    )
endif()
