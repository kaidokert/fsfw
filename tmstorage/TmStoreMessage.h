/*
 * TmStoreMessage.h
 *
 *  Created on: 23.02.2015
 *      Author: baetz
 */

#ifndef FRAMEWORK_TMSTORAGE_TMSTOREMESSAGE_H_
#define FRAMEWORK_TMSTORAGE_TMSTOREMESSAGE_H_

#include <framework/ipc/CommandMessage.h>
#include <framework/storagemanager/StorageManagerIF.h>
#include <framework/tmstorage/TmStoreFrontendIF.h>

class TmStoreMessage: public CommandMessage {
public:
	static ReturnValue_t setEnableStoringMessage(CommandMessage* cmd, bool setEnabled);
	static ReturnValue_t setDeleteContentMessage(CommandMessage* cmd, TmStoreFrontendIF::ApidSsc upTo);
	static ReturnValue_t setDownlinkContentMessage(CommandMessage* cmd, TmStoreFrontendIF::ApidSsc fromPacket, TmStoreFrontendIF::ApidSsc toPacket );
	static void setChangeSelectionDefinitionMessage(CommandMessage* cmd, bool addDefinition, store_address_t store_id);
	static ReturnValue_t setReportSelectionDefinitionMessage(CommandMessage* cmd);
	static void setSelectionDefinitionReportMessage(CommandMessage* cmd, store_address_t storeId);
	static ReturnValue_t setReportStoreCatalogueMessage(CommandMessage* cmd);
	static void setStoreCatalogueReportMessage(CommandMessage* cmd, store_address_t storeId);
	static void clear(CommandMessage* cmd);
	static TmStoreFrontendIF::ApidSsc getPacketId1(CommandMessage* cmd);
	static TmStoreFrontendIF::ApidSsc getPacketId2(CommandMessage* cmd);
	static bool getEnableStoring(CommandMessage* cmd);
	static bool getAddToSelection(CommandMessage* cmd);
	static store_address_t getStoreId(const CommandMessage* cmd);
	virtual ~TmStoreMessage();
	static const uint8_t MESSAGE_ID = TM_STORE_MESSAGE_ID;
	static const Command_t ENABLE_STORING = MAKE_COMMAND_ID(1);
	static const Command_t DELETE_STORE_CONTENT = MAKE_COMMAND_ID(2);
	static const Command_t DOWNLINK_STORE_CONTENT = MAKE_COMMAND_ID(3);
	static const Command_t CHANGE_SELECTION_DEFINITION = MAKE_COMMAND_ID(4);
	static const Command_t REPORT_SELECTION_DEFINITION = MAKE_COMMAND_ID(5);
	static const Command_t SELECTION_DEFINITION_REPORT = MAKE_COMMAND_ID(6);
	static const Command_t REPORT_STORE_CATALOGUE = MAKE_COMMAND_ID(7);
	static const Command_t STORE_CATALOGUE_REPORT = MAKE_COMMAND_ID(8);
private:
	TmStoreMessage();
};

#endif /* FRAMEWORK_TMSTORAGE_TMSTOREMESSAGE_H_ */
