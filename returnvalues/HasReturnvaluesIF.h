/*
 * HasReturnvaluesIF.h
 *
 *  Created on: 05.11.2012
 *      Author: mohr
 */

#ifndef HASRETURNVALUESIF_H_
#define HASRETURNVALUESIF_H_

#include <stdint.h>

#define MAKE_RETURN_CODE( number )	((INTERFACE_ID << 8) + (number))
typedef uint16_t ReturnValue_t;

//TODO: make one storageIF containing all possible returnvalues for storing things

//A list of current interface id's. May be an enum. May be put in a separate header file.
#define OPERATING_SYSTEM_ABSTRACTION	0x01 //OS
#define OBJECT_MANAGER_IF				0x02 //OM
#define DEVICE_HANDLER_BASE				0x03 //DHB
#define RMAP_CHANNEL					0x04 //RMP
#define POWER_SWITCH_IF					0x05 //PS
#define HAS_MEMORY_IF					0x06 //PP
#define DEVICE_STATE_MACHINE_BASE		0x07 //DSMB
#define DATA_SET_CLASS					0x08 //DPS
#define POOL_RAW_ACCESS_CLASS			0x09 //DPR
#define CONTROLLER_BASE					0x0A //CTR
#define SUBSYSTEM_BASE					0x0B //SB
#define MODE_STORE_IF					0x0C //MS
#define SUBSYSTEM						0x0D //SS
#define HAS_MODES_IF					0x0E //HM
#define COMMAND_MESSAGE					0x0F //CM
#define CCSDS_TIME_HELPER_CLASS			0x10 //TIM
#define STAR_TRACKER_HANDLER			0x11 //STR
#define ARRAY_LIST						0x12 //AL
#define ASSEMBLY_BASE					0x13 //AB
#define MEMORY_HELPER					0x14 //MH
#define PCDU_HANDLER_CLASS				0x15 //PHC
#define RW_HANDLER_CLASS				0x16 //RWC
#define PLOC_CC_CLASS					0x17 //PCC
#define SERIALIZE_IF					0x18 //SE
#define FIXED_MAP						0x19 //FM
#define HAS_HEALTH_IF					0x1A //HHI
#define FRAME_FINDER_IF					0x1B //FFI
#define TM_VG_GENERATOR_IF				0x1C //VCG
#define FIFO_CLASS						0x1D //FF
#define MESSAGE_PROXY					0x1E //MQP
#define TRIPLE_REDUNDACY_CHECK			0x1F //TRC
#define TC_PACKET_CHECK					0x20 //TCC
#define PACKET_DISTRIBUTION				0x21 //TCD
#define ACCEPTS_TELECOMMANDS_IF			0x22 //PUS
#define DEVICE_SERVICE_BASE				0x23 //DSB
#define FUNCTION_MANAGEMENT_SERVICE		0x24 //FM
#define HOUSEKEEPING_SERVICE			0x25 //HK
#define MEMORY_MANAGEMENT_SERVICE		0x26 //MM
#define COMMAND_SERVICE_BASE			0x27 //CSB
#define OPERATIONS_SCHEDULING_SERVICE	0x28 //SCH
#define TM_STORE_BACKEND_IF				0x29 //TMB
#define TM_STORE_FRONTEND_IF			0x2A //TMF
#define STORAGE_AND_RETRIEVAL_SERVICE	0x2B //SR
#define MATCH_TREE_CLASS				0x2C //MT
#define EVENT_MANAGER_IF				0x2D //EV
#define HANDLES_FAILURES_IF				0x2E //FDI
#define DEVICE_HANDLER_IF				0x2F //DHI
#define STORAGE_MANAGER_IF				0x30 //SM
#define THERMAL_COMPONENT_IF			0x31 //TC
#define THERMAL_CONTROLLER_CLASS		0x33 //TCC
#define FOG_HANDLER_CLASS				0x34 //FHC
#define INTERNAL_ERROR_CODES			0x35 //IEC
#define TRAP							0x36 //TRP
#define PAYLOAD_HANDLING				0x37 //PLH
#define GPS_MONITORING					0x3A //GPSM
#define SUS_MONITORING					0x3B //SUSM
#define MGMMGT_MONITORING				0x3C //MGMT
#define FOG_MONITORING					0x3D //FOGM
#define STR_MONITORING					0x3E //STRM
#define DEPLOYMENT_CONTROLLER_CLASS		0x3F //DPLC
#define CCSDS_HANDLER_IF				0x40 //CCS
#define ACS_CONTROLLER_CLASS			0x41 //ACS
#define SGP4PROPAGATOR_CLASS			0x42 //SGP
#define PARAMETER_WRAPPER				0x43 //PAW
#define HAS_PARAMETERS_IF				0x44 //HPA
#define ASCII_CONVERTER					0x50 //ASC
#define POWER_SWITCHER					0x51 //POS
#define CCSDS_BOARD_HANDLER				0x52 //CBH
#define LIMITS_IF						0x60 //LIM
#define COMMANDS_ACTIONS_IF				0x80 //CF
#define HAS_ACTIONS_IF					0x81 //HF
#define DEVICE_COMMUNICATION_IF			0x90 //DC
#define BSP								0xF0 //BSP




class HasReturnvaluesIF {
public:
	static const ReturnValue_t RETURN_OK = 0;
	static const ReturnValue_t RETURN_FAILED = 1;
	virtual ~HasReturnvaluesIF() {
	}

};



#endif /* HASRETURNVALUESIF_H_ */
