/*
 * TmStoreMessage.cpp
 *
 *  Created on: 23.02.2015
 *      Author: baetz
 */

#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/tmstorage/TmStoreMessage.h>

TmStoreMessage::~TmStoreMessage() {
	
}

TmStoreMessage::TmStoreMessage() {
}

ReturnValue_t TmStoreMessage::setEnableStoringMessage(CommandMessage* cmd,
		bool setEnabled) {
	cmd->setCommand(ENABLE_STORING);
	cmd->setParameter(setEnabled);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TmStoreMessage::setDeleteContentMessage(CommandMessage* cmd,
		TmStoreFrontendIF::ApidSsc upTo) {
	cmd->setCommand(DELETE_STORE_CONTENT);
	cmd->setParameter((upTo.apid<<16) + upTo.ssc);
	return HasReturnvaluesIF::RETURN_OK;
}

ReturnValue_t TmStoreMessage::setDownlinkContentMessage(CommandMessage* cmd,
		TmStoreFrontendIF::ApidSsc fromPacket,
		TmStoreFrontendIF::ApidSsc toPacket) {
	cmd->setCommand(DOWNLINK_STORE_CONTENT);
	cmd->setParameter((fromPacket.apid<<16) + fromPacket.ssc);
	cmd->setParameter2((toPacket.apid<<16) + toPacket.ssc);
	return HasReturnvaluesIF::RETURN_OK;
}

TmStoreFrontendIF::ApidSsc TmStoreMessage::getPacketId1(CommandMessage* cmd) {
	TmStoreFrontendIF::ApidSsc temp;
	temp.apid = (cmd->getParameter() >> 16) & 0xFFFF;
	temp.ssc = cmd->getParameter() & 0xFFFF;
	return temp;
}

TmStoreFrontendIF::ApidSsc TmStoreMessage::getPacketId2(CommandMessage* cmd) {
	TmStoreFrontendIF::ApidSsc temp;
	temp.apid = (cmd->getParameter2() >> 16) & 0xFFFF;
	temp.ssc = cmd->getParameter2() & 0xFFFF;
	return temp;
}

bool TmStoreMessage::getEnableStoring(CommandMessage* cmd) {
	return (bool)cmd->getParameter();
}

void TmStoreMessage::setChangeSelectionDefinitionMessage(
		CommandMessage* cmd, bool addDefinition, store_address_t store_id) {
	cmd->setCommand(CHANGE_SELECTION_DEFINITION);
	cmd->setParameter(addDefinition);
	cmd->setParameter2(store_id.raw);
}

void TmStoreMessage::clear(CommandMessage* cmd) {
	switch(cmd->getCommand()) {
	case SELECTION_DEFINITION_REPORT:
	case STORE_CATALOGUE_REPORT:
	case CHANGE_SELECTION_DEFINITION: {
		StorageManagerIF *ipcStore = objectManager->get<StorageManagerIF>(
				objects::IPC_STORE);
		if (ipcStore != NULL) {
			ipcStore->deleteData(getStoreId(cmd));
		}
	}
		break;
	default:
		break;
	}
}

store_address_t TmStoreMessage::getStoreId(const CommandMessage* cmd) {
	store_address_t temp;
	temp.raw = cmd->getParameter2();
	return temp;
}

bool TmStoreMessage::getAddToSelection(CommandMessage* cmd) {
	return (bool)cmd->getParameter();
}

ReturnValue_t TmStoreMessage::setReportSelectionDefinitionMessage(
		CommandMessage* cmd) {
	cmd->setCommand(REPORT_SELECTION_DEFINITION);
	return HasReturnvaluesIF::RETURN_OK;
}

void TmStoreMessage::setSelectionDefinitionReportMessage(
		CommandMessage* cmd, store_address_t storeId) {
	cmd->setCommand(SELECTION_DEFINITION_REPORT);
	cmd->setParameter2(storeId.raw);
}

ReturnValue_t TmStoreMessage::setReportStoreCatalogueMessage(
		CommandMessage* cmd) {
	cmd->setCommand(REPORT_STORE_CATALOGUE);
	return HasReturnvaluesIF::RETURN_OK;
}

void TmStoreMessage::setStoreCatalogueReportMessage(CommandMessage* cmd,
		store_address_t storeId) {
	cmd->setCommand(STORE_CATALOGUE_REPORT);
	cmd->setParameter2(storeId.raw);
}
