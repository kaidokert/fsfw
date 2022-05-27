target_sources(
  ${LIB_FSFW_NAME}
  PRIVATE AbstractTemperatureSensor.cpp
          Heater.cpp
          RedundantHeater.cpp
          ThermalComponentCore.cpp
          ThermalComponent.cpp
          ThermalModule.cpp
          ThermalMonitorReporter.cpp)
