#ifndef FSFW_RETURNVALUES_FWCLASSIDS_H_
#define FSFW_RETURNVALUES_FWCLASSIDS_H_

#include <cstdint>

// The comment block at the end is used by the returnvalue exporter.
// It is recommended to add it as well for mission returnvalues
namespace CLASS_ID {
enum : uint8_t {
  FW_CLASS_ID_START = 0,          // [EXPORT] : [START]
  OPERATING_SYSTEM_ABSTRACTION,   // OS
  OBJECT_MANAGER_IF,              // OM
  DEVICE_HANDLER_BASE,            // DHB
  RMAP_CHANNEL,                   // RMP
  POWER_SWITCH_IF,                // PS
  HAS_MEMORY_IF,                  // PP
  DEVICE_STATE_MACHINE_BASE,      // DSMB
  DATA_SET_CLASS,                 // DPS
  POOL_RAW_ACCESS_CLASS,          // DPR
  CONTROLLER_BASE,                // CTR
  SUBSYSTEM_BASE,                 // SB
  MODE_STORE_IF,                  // MS
  SUBSYSTEM,                      // SS
  HAS_MODES_IF,                   // HM
  COMMAND_MESSAGE,                // CM
  CCSDS_TIME_HELPER_CLASS,        // TIM
  ARRAY_LIST,                     // AL
  ASSEMBLY_BASE,                  // AB
  MEMORY_HELPER,                  // MH
  SERIALIZE_IF,                   // SE
  FIXED_MAP,                      // FM
  FIXED_MULTIMAP,                 // FMM
  HAS_HEALTH_IF,                  // HHI
  FIFO_CLASS,                     // FF
  MESSAGE_PROXY,                  // MQP
  TRIPLE_REDUNDACY_CHECK,         // TRC
  TMTC_DISTRIBUTION,              // TCC
  PACKET_DISTRIBUTION,            // TCD
  ACCEPTS_TELECOMMANDS_IF,        // ATC
  PUS_IF,                         // PUS
  DEVICE_SERVICE_BASE,            // DSB
  COMMAND_SERVICE_BASE,           // CSB
  TM_STORE_BACKEND_IF,            // TMB
  TM_STORE_FRONTEND_IF,           // TMF
  STORAGE_AND_RETRIEVAL_SERVICE,  // SR
  MATCH_TREE_CLASS,               // MT
  EVENT_MANAGER_IF,               // EV
  HANDLES_FAILURES_IF,            // FDI
  DEVICE_HANDLER_IF,              // DHI
  STORAGE_MANAGER_IF,             // SM
  THERMAL_COMPONENT_IF,           // TC
  INTERNAL_ERROR_CODES,           // IEC
  TRAP,                           // TRP
  CCSDS_HANDLER_IF,               // CCS
  PARAMETER_WRAPPER,              // PAW
  HAS_PARAMETERS_IF,              // HPA
  ASCII_CONVERTER,                // ASC
  POWER_SWITCHER,                 // POS
  LIMITS_IF,                      // LIM
  COMMANDS_ACTIONS_IF,            // CF
  HAS_ACTIONS_IF,                 // HF
  DEVICE_COMMUNICATION_IF,        // DC
  BSP,                            // BSP
  CFDP,                           // CFDP
  TIME_STAMPER_IF,                // TSI
  SGP4PROPAGATOR_CLASS,           // SGP4
  MUTEX_IF,                       // MUX
  MESSAGE_QUEUE_IF,               // MQI
  SEMAPHORE_IF,                   // SPH
  LOCAL_POOL_OWNER_IF,            // LPIF
  POOL_VARIABLE_IF,               // PVA
  HOUSEKEEPING_MANAGER,           // HKM
  DLE_ENCODER,                    // DLEE
  PUS_SERVICE_3,                  // PUS3
  PUS_SERVICE_9,                  // PUS9
  PUS_SERVICE_11,                 // PUS11
  FILE_SYSTEM,                    // FILS
  LINUX_OSAL,                     // UXOS
  HAL_SPI,                        // HSPI
  HAL_UART,                       // HURT
  HAL_I2C,                        // HI2C
  HAL_GPIO,                       // HGIO
  FIXED_SLOT_TASK_IF,             // FTIF
  MGM_LIS3MDL,                    // MGMLIS3
  MGM_RM3100,                     // MGMRM3100
  SPACE_PACKET_PARSER,            // SPPA
  FW_CLASS_ID_COUNT               // [EXPORT] : [END]

};
}

#endif /* FSFW_RETURNVALUES_FWCLASSIDS_H_ */
