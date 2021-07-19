/**
 * @file	LimitViolationReporter.h
 * @brief	This file defines the LimitViolationReporter class.
 * @date	17.07.2014
 * @author	baetz
 */
#ifndef LIMITVIOLATIONREPORTER_H_
#define LIMITVIOLATIONREPORTER_H_

#include "monitoringConf.h"
#include "../returnvalues/HasReturnvaluesIF.h"
#include "../serialize/SerializeIF.h"
#include "../storagemanager/StorageManagerIF.h"
#include "../ipc/MessageQueueSenderIF.h"

namespace Factory{
void setStaticFrameworkObjectIds();
}

class LimitViolationReporter {
	friend void (Factory::setStaticFrameworkObjectIds)();
public:
	static ReturnValue_t sendLimitViolationReport(const SerializeIF* data);
private:
	static object_id_t reportingTarget;
	static MessageQueueId_t reportQueue;
	static StorageManagerIF* ipcStore;
	static ReturnValue_t checkClassLoaded();
	LimitViolationReporter();
};

#endif /* LIMITVIOLATIONREPORTER_H_ */
