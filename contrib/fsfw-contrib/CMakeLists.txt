if(FSFW_ADD_SGP4_PROPAGATOR)
    target_sources(${LIB_FSFW_NAME} PRIVATE
        sgp4/sgp4unit.cpp
    )
    target_include_directories(${LIB_FSFW_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/sgp4
    )
    target_include_directories(${LIB_FSFW_NAME} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/sgp4
    )
endif()
