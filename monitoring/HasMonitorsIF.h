/**
 * @file	HasMonitorsIF.h
 * @brief	This file defines the HasMonitorsIF class.
 * @date	28.07.2014
 * @author	baetz
 */
#ifndef HASMONITORSIF_H_
#define HASMONITORSIF_H_

#include "../events/EventReportingProxyIF.h"
#include "../objectmanager/ObjectManagerIF.h"
#include "../ipc/MessageQueueSenderIF.h"

class HasMonitorsIF {
public:
	static const uint8_t MAX_N_PARAMETER = 10;
//	static const uint8_t MAX_N_LIMIT_ID = 10;
	virtual ReturnValue_t setCheckingOfParameters(uint8_t checkingStrategy,
			bool forOnePid = false, uint32_t parameterId = 0) = 0;
	virtual ReturnValue_t modifyParameterMonitor(uint8_t limitType,
			uint32_t parameterId, const uint8_t* data, uint32_t size) = 0;
	virtual ReturnValue_t modifyObjectMonitor(uint32_t objectId,
			const uint8_t* data, const uint32_t size) = 0;
	virtual void setAllMonitorsToUnchecked() = 0;
	virtual MessageQueueId_t getCommandQueue() const = 0;
	virtual ~HasMonitorsIF() {
	}
};

#endif /* HASMONITORSIF_H_ */
