#include "../objectmanager/ObjectManagerIF.h"
#include "DeviceHandlerMessage.h"
#include "../objectmanager/ObjectManagerIF.h"

DeviceHandlerMessage::DeviceHandlerMessage() {
}

store_address_t DeviceHandlerMessage::getStoreAddress(
		const CommandMessage* message) {
	return store_address_t(message->getParameter2());
}

uint32_t DeviceHandlerMessage::getDeviceCommandId(
		const CommandMessage* message) {
	return message->getParameter();
}

object_id_t DeviceHandlerMessage::getIoBoardObjectId(
		const CommandMessage* message) {
	return message->getParameter();
}

uint8_t DeviceHandlerMessage::getWiretappingMode(
		const CommandMessage* message) {
	return message->getParameter();
}

//void DeviceHandlerMessage::setDeviceHandlerDirectCommandMessage(
//		CommandMessage* message, DeviceCommandId_t deviceCommand,
//		store_address_t commandParametersStoreId) {
//	message->setCommand(CMD_DIRECT);
//	message->setParameter(deviceCommand);
//	message->setParameter2(commandParametersStoreId.raw);
//}

void DeviceHandlerMessage::setDeviceHandlerRawCommandMessage(
		CommandMessage* message, store_address_t rawPacketStoreId) {
	message->setCommand(CMD_RAW);
	message->setParameter2(rawPacketStoreId.raw);
}

void DeviceHandlerMessage::setDeviceHandlerWiretappingMessage(
		CommandMessage* message, uint8_t wiretappingMode) {
	message->setCommand(CMD_WIRETAPPING);
	message->setParameter(wiretappingMode);
}

void DeviceHandlerMessage::setDeviceHandlerSwitchIoBoardMessage(
		CommandMessage* message, uint32_t ioBoardIdentifier) {
	message->setCommand(CMD_SWITCH_IOBOARD);
	message->setParameter(ioBoardIdentifier);
}

object_id_t DeviceHandlerMessage::getDeviceObjectId(
		const CommandMessage* message) {
	return message->getParameter();
}

void DeviceHandlerMessage::setDeviceHandlerRawReplyMessage(
		CommandMessage* message, object_id_t deviceObjectid,
		store_address_t rawPacketStoreId, bool isCommand) {
	if (isCommand) {
		message->setCommand(REPLY_RAW_COMMAND);
	} else {
		message->setCommand(REPLY_RAW_REPLY);
	}
	message->setParameter(deviceObjectid);
	message->setParameter2(rawPacketStoreId.raw);
}

void DeviceHandlerMessage::setDeviceHandlerDirectCommandReply(
		CommandMessage* message, object_id_t deviceObjectid,
		store_address_t commandParametersStoreId) {
	message->setCommand(REPLY_DIRECT_COMMAND_DATA);
	message->setParameter(deviceObjectid);
	message->setParameter2(commandParametersStoreId.raw);
}

void DeviceHandlerMessage::clear(CommandMessage* message) {
	switch (message->getCommand()) {
	case CMD_RAW:
//	case CMD_DIRECT:
	case REPLY_RAW_COMMAND:
	case REPLY_RAW_REPLY:
	case REPLY_DIRECT_COMMAND_DATA: {
		StorageManagerIF *ipcStore = objectManager->get<StorageManagerIF>(
				objects::IPC_STORE);
		if (ipcStore != NULL) {
			ipcStore->deleteData(getStoreAddress(message));
		}
	}
		/* NO BREAK falls through*/
	case CMD_SWITCH_IOBOARD:
	case CMD_WIRETAPPING:
		message->setCommand(CommandMessage::CMD_NONE);
		message->setParameter(0);
		message->setParameter2(0);
		break;
	}

}
