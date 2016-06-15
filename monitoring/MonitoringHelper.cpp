/*
 * MonitoringHelper.cpp
 *
 *  Created on: 07.07.2014
 *      Author: baetz
 */

#include <framework/monitoring/MonitoringHelper.h>
#include <framework/monitoring/MonitoringIF.h>
#include <framework/monitoring/MonitoringMessage.h>
#include <framework/monitoring/MonitoringMessageContent.h>
#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/serialize/SerializeAdapter.h>

MonitoringHelper::MonitoringHelper(HasMonitorsIF* limitOwner) :
		owner(limitOwner), ipcStore(NULL) {
}

MonitoringHelper::~MonitoringHelper() {
	
}

ReturnValue_t MonitoringHelper::handleMessage(CommandMessage* message) {
	ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
	switch (message->getCommand()) {
	case MonitoringMessage::CHANGE_REPORTING_STRATEGY:
		result = handleReportingStrategyMessage(MonitoringMessage::getReportingStategy(message),
				MonitoringMessage::getStoreId(message));
		break;
	case MonitoringMessage::UPDATE_PARAMETER_MONITOR:
		result = handleUpdateParameterMonitor(MonitoringMessage::getStoreId(message));
		break;
	case MonitoringMessage::UPDATE_OBJECT_MONITOR:
		result = handleUpdateObjectMonitor(MonitoringMessage::getStoreId(message));
		break;
	default:
		return HasReturnvaluesIF::RETURN_FAILED;
	}
	replyReturnValue(message, result);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t MonitoringHelper::handleReportingStrategyMessage(uint8_t strategy,
		store_address_t storeId) {
	const uint8_t* data = NULL;
	uint32_t size = 0;
	ReturnValue_t result = ipcStore->getData(storeId, &data, &size);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	EnableDisableContent content;
	int32_t tSize = size;
	result = content.deSerialize(&data, &tSize, true);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		ipcStore->deleteData(storeId);
		return result;
	}
	if (content.funkyList.size == 0) {
		result = owner->setCheckingOfParameters(strategy);
	} else {
		for (EnableDisableContent::EnableDisableList::Iterator iter = content.funkyList.begin();
				iter != content.funkyList.end(); iter++) {
				result = owner->setCheckingOfParameters(strategy, true, *iter);
				if (result != HasReturnvaluesIF::RETURN_OK) {
					//TODO: SW event (as stated in pus)
				}
				break;
		}
		result = HasReturnvaluesIF::RETURN_OK;
	}
	ipcStore->deleteData(storeId);
	return result;
}

ReturnValue_t MonitoringHelper::handleUpdateParameterMonitor(store_address_t storeId) {
	const uint8_t* data = NULL;
	uint32_t size = 0;
	ReturnValue_t result = ipcStore->getData(storeId, &data, &size);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	uint8_t limitType = *data;
	data++;
	size -= 1;
	const uint8_t* pData = data;
	int32_t pSize = size;
	uint32_t parameterId = 0;
	result = SerializeAdapter<uint32_t>::deSerialize(&parameterId, &pData, &pSize,
	true);
	if (result == HasReturnvaluesIF::RETURN_OK) {
		result = owner->modifyParameterMonitor(limitType, parameterId, pData, pSize);
	}
	ipcStore->deleteData(storeId);
	return result;
}

ReturnValue_t MonitoringHelper::handleUpdateObjectMonitor(store_address_t storeId) {
	const uint8_t* data = NULL;
	uint32_t size = 0;
	ReturnValue_t result = ipcStore->getData(storeId, &data, &size);
	if (result != HasReturnvaluesIF::RETURN_OK) {
		return result;
	}
	object_id_t objectId = 0;
	const uint8_t* pData = data;
	int32_t tSize = size;
	result = SerializeAdapter<object_id_t>::deSerialize(&objectId, &pData, &tSize, true);
	if (result == HasReturnvaluesIF::RETURN_OK) {
		result = owner->modifyObjectMonitor(objectId, pData, tSize);
	}
	ipcStore->deleteData(storeId);
	return result;
}

void MonitoringHelper::replyReturnValue(CommandMessage* message, ReturnValue_t result) {
	CommandMessage reply;
	if (result == HasReturnvaluesIF::RETURN_OK) {
		reply.setCommand(CommandMessage::REPLY_COMMAND_OK);
	} else {
		reply.setReplyRejected(result, message->getCommand());
	}
	MessageQueueSender sender(message->getSender());
	sender.sendToDefault(&reply, owner->getCommandQueue());
}

ReturnValue_t MonitoringHelper::initialize() {
	ipcStore = objectManager->get<StorageManagerIF>(objects::IPC_STORE);
	if (ipcStore != NULL) {
		return HasReturnvaluesIF::RETURN_OK;
	} else {
		return HasReturnvaluesIF::RETURN_FAILED;
	}
}
