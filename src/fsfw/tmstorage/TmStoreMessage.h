#ifndef FSFW_TMSTORAGE_TMSTOREMESSAGE_H_
#define FSFW_TMSTORAGE_TMSTOREMESSAGE_H_

#include "TmStorePackets.h"
#include "fsfw/ipc/CommandMessage.h"
#include "fsfw/objectmanager/SystemObjectIF.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
#include "tmStorageConf.h"

class TmStoreMessage {
 public:
  static ReturnValue_t setEnableStoringMessage(CommandMessage* cmd, bool setEnabled);
  static ReturnValue_t setDeleteContentMessage(CommandMessage* cmd, ApidSsc upTo);
  static ReturnValue_t setDownlinkContentMessage(CommandMessage* cmd, ApidSsc fromPacket,
                                                 ApidSsc toPacket);
  static void setChangeSelectionDefinitionMessage(CommandMessage* cmd, bool addDefinition,
                                                  store_address_t store_id);
  static ReturnValue_t setReportSelectionDefinitionMessage(CommandMessage* cmd);
  static void setSelectionDefinitionReportMessage(CommandMessage* cmd, store_address_t storeId);
  static ReturnValue_t setReportStoreCatalogueMessage(CommandMessage* cmd);
  static void setStoreCatalogueReportMessage(CommandMessage* cmd, object_id_t objectId,
                                             store_address_t storeId);
  static void setDownlinkContentTimeMessage(CommandMessage* cmd, store_address_t storeId);
  static void setIndexReportMessage(CommandMessage* cmd, store_address_t storeId);
  static ReturnValue_t setDeleteBlocksMessage(CommandMessage* cmd, uint32_t addressLow,
                                              uint32_t addressHigh);
  static ReturnValue_t setDownlinkBlocksMessage(CommandMessage* cmd, uint32_t addressLow,
                                                uint32_t addressHigh);
  static ReturnValue_t setIndexRequestMessage(CommandMessage* cmd);
  static void setDeleteContentTimeMessage(CommandMessage* cmd, store_address_t storeId);
  static void clear(CommandMessage* cmd);
  static object_id_t getObjectId(CommandMessage* cmd);
  static ApidSsc getPacketId1(CommandMessage* cmd);
  static ApidSsc getPacketId2(CommandMessage* cmd);
  static bool getEnableStoring(CommandMessage* cmd);
  static bool getAddToSelection(CommandMessage* cmd);
  static uint32_t getAddressLow(CommandMessage* cmd);
  static uint32_t getAddressHigh(CommandMessage* cmd);

  static store_address_t getStoreId(const CommandMessage* cmd);
  virtual ~TmStoreMessage();
  static const uint8_t MESSAGE_ID = messagetypes::TM_STORE;
  static const Command_t ENABLE_STORING = MAKE_COMMAND_ID(1);
  static const Command_t DELETE_STORE_CONTENT = MAKE_COMMAND_ID(2);
  static const Command_t DOWNLINK_STORE_CONTENT = MAKE_COMMAND_ID(3);
  static const Command_t CHANGE_SELECTION_DEFINITION = MAKE_COMMAND_ID(4);
  static const Command_t REPORT_SELECTION_DEFINITION = MAKE_COMMAND_ID(5);
  static const Command_t SELECTION_DEFINITION_REPORT = MAKE_COMMAND_ID(6);
  static const Command_t REPORT_STORE_CATALOGUE = MAKE_COMMAND_ID(7);
  static const Command_t STORE_CATALOGUE_REPORT = MAKE_COMMAND_ID(8);
  static const Command_t DOWNLINK_STORE_CONTENT_TIME = MAKE_COMMAND_ID(9);
  static const Command_t DELETE_STORE_CONTENT_TIME = MAKE_COMMAND_ID(10);
  static const Command_t DELETE_STORE_CONTENT_BLOCKS = MAKE_COMMAND_ID(11);
  static const Command_t DOWNLINK_STORE_CONTENT_BLOCKS = MAKE_COMMAND_ID(12);
  static const Command_t REPORT_INDEX_REQUEST = MAKE_COMMAND_ID(13);
  static const Command_t INDEX_REPORT = MAKE_COMMAND_ID(14);

 private:
  TmStoreMessage();
};

#endif /* FSFW_TMSTORAGE_TMSTOREMESSAGE_H_ */
