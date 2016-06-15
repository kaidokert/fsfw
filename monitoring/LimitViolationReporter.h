/**
 * @file	LimitViolationReporter.h
 * @brief	This file defines the LimitViolationReporter class.
 * @date	17.07.2014
 * @author	baetz
 */
#ifndef LIMITVIOLATIONREPORTER_H_
#define LIMITVIOLATIONREPORTER_H_

#include <framework/ipc/MessageQueueSender.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/serialize/SerializeIF.h>
#include <framework/storagemanager/StorageManagerIF.h>

class LimitViolationReporter {
public:
	static ReturnValue_t sendLimitViolationReport(const SerializeIF* data);
private:
	static MessageQueueSender reportQueue;
	static StorageManagerIF* ipcStore;
	static ReturnValue_t checkClassLoaded();
	LimitViolationReporter();
};

#endif /* LIMITVIOLATIONREPORTER_H_ */
