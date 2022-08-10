target_sources(
  ${LIB_FSFW_NAME}
  PRIVATE EntityIdTlv.cpp
          FilestoreRequestTlv.cpp
          FilestoreResponseTlv.cpp
          Lv.cpp
          Tlv.cpp
          StringLv.cpp
          FlowLabelTlv.cpp
          MessageToUserTlv.cpp
          FaultHandlerOverrideTlv.cpp)
