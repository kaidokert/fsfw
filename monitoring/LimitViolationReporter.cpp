/**
 * @file	LimitViolationReporter.cpp
 * @brief	This file defines the LimitViolationReporter class.
 * @date	17.07.2014
 * @author	baetz
 */
#include "LimitViolationReporter.h"
#include "MonitoringIF.h"
#include "ReceivesMonitoringReportsIF.h"
#include "../objectmanager/ObjectManagerIF.h"
#include "../serialize/SerializeAdapter.h"

ReturnValue_t LimitViolationReporter::sendLimitViolationReport(const SerializeIF* data) {
	ReturnValue_t result = checkClassLoaded();
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	store_address_t storeId;
	uint8_t* dataTarget = NULL;
	size_t maxSize = data->getSerializedSize();
	if (maxSize > MonitoringIF::VIOLATION_REPORT_MAX_SIZE) {
		return MonitoringIF::INVALID_SIZE;
	}
	result = ipcStore->getFreeElement(&storeId, maxSize,
			&dataTarget);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	size_t size = 0;
	result = data->serialize(&dataTarget, &size, maxSize, SerializeIF::Endianness::BIG);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	CommandMessage report;
	MonitoringMessage::setLimitViolationReport(&report, storeId);
	return MessageQueueSenderIF::sendMessage(reportQueue, &report);
}

ReturnValue_t LimitViolationReporter::checkClassLoaded() {
	if (reportQueue == 0) {
		ReceivesMonitoringReportsIF* receiver = objectManager->get<
				ReceivesMonitoringReportsIF>(reportingTarget);
		if (receiver == NULL) {
			return ObjectManagerIF::NOT_FOUND;
		}
		reportQueue = receiver->getCommandQueue();
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
MessageQueueId_t LimitViolationReporter::reportQueue = 0;
StorageManagerIF* LimitViolationReporter::ipcStore = NULL;
object_id_t LimitViolationReporter::reportingTarget = 0;
