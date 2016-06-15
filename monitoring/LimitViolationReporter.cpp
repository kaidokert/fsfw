/**
 * @file	LimitViolationReporter.cpp
 * @brief	This file defines the LimitViolationReporter class.
 * @date	17.07.2014
 * @author	baetz
 */
#include <framework/monitoring/LimitViolationReporter.h>
#include <framework/monitoring/MonitoringIF.h>
#include <framework/monitoring/ReceivesMonitoringReportsIF.h>
#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/serialize/SerializeAdapter.h>

ReturnValue_t LimitViolationReporter::sendLimitViolationReport(const SerializeIF* data) {
	ReturnValue_t result = checkClassLoaded();
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	store_address_t storeId;
	uint8_t* dataTarget = NULL;
	uint32_t maxSize = data->getSerializedSize();
	if (maxSize > MonitoringIF::VIOLATION_REPORT_MAX_SIZE) {
		return MonitoringIF::INVALID_SIZE;
	}
	result = ipcStore->getFreeElement(&storeId, maxSize,
			&dataTarget);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	uint32_t size = 0;
	result = data->serialize(&dataTarget, &size, maxSize, true);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	CommandMessage report;
	MonitoringMessage::setLimitViolationReport(&report, storeId);
	return reportQueue.sendToDefault(&report);
}

ReturnValue_t LimitViolationReporter::checkClassLoaded() {
	if (reportQueue.getDefaultDestination() == 0) {
		ReceivesMonitoringReportsIF* receiver = objectManager->get<
				ReceivesMonitoringReportsIF>(objects::PUS_MONITORING_SERVICE);
		if (receiver == NULL) {
			return ObjectManagerIF::NOT_FOUND;
		}
		reportQueue.setDefaultDestination(receiver->getCommandQueue());
	}
	if (ipcStore == NULL) {
		ipcStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
		if (ipcStore == NULL) {
			return HasReturnvaluesIF::RETURN_FAILED;
		}
	}
	return HasReturnvaluesIF::RETURN_OK;
}

//Lazy initialization.
MessageQueueSender LimitViolationReporter::reportQueue;
StorageManagerIF* LimitViolationReporter::ipcStore = NULL;
