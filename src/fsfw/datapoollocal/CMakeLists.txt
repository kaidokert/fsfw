target_sources(
  ${LIB_FSFW_NAME}
  PRIVATE LocalDataPoolManager.cpp LocalDataSet.cpp LocalPoolDataSetBase.cpp
          LocalPoolObjectBase.cpp SharedLocalDataSet.cpp)

add_subdirectory(internal)
