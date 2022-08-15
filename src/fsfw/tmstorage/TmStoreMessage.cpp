#include "TmStoreMessage.h"

#include "fsfw/objectmanager/ObjectManager.h"

TmStoreMessage::~TmStoreMessage() {}

TmStoreMessage::TmStoreMessage() {}

ReturnValue_t TmStoreMessage::setEnableStoringMessage(CommandMessage* cmd, bool setEnabled) {
  cmd->setCommand(ENABLE_STORING);
  cmd->setParameter(setEnabled);
  return returnvalue::OK;
}

ReturnValue_t TmStoreMessage::setDeleteContentMessage(CommandMessage* cmd, ApidSsc upTo) {
  cmd->setCommand(DELETE_STORE_CONTENT);
  cmd->setParameter((upTo.apid << 16) + upTo.ssc);
  return returnvalue::OK;
}

ReturnValue_t TmStoreMessage::setDownlinkContentMessage(CommandMessage* cmd, ApidSsc fromPacket,
                                                        ApidSsc toPacket) {
  cmd->setCommand(DOWNLINK_STORE_CONTENT);
  cmd->setParameter((fromPacket.apid << 16) + fromPacket.ssc);
  cmd->setParameter2((toPacket.apid << 16) + toPacket.ssc);
  return returnvalue::OK;
}

ApidSsc TmStoreMessage::getPacketId1(CommandMessage* cmd) {
  ApidSsc temp;
  temp.apid = (cmd->getParameter() >> 16) & 0xFFFF;
  temp.ssc = cmd->getParameter() & 0xFFFF;
  return temp;
}

ApidSsc TmStoreMessage::getPacketId2(CommandMessage* cmd) {
  ApidSsc temp;
  temp.apid = (cmd->getParameter2() >> 16) & 0xFFFF;
  temp.ssc = cmd->getParameter2() & 0xFFFF;
  return temp;
}

bool TmStoreMessage::getEnableStoring(CommandMessage* cmd) { return (bool)cmd->getParameter(); }

void TmStoreMessage::setChangeSelectionDefinitionMessage(CommandMessage* cmd, bool addDefinition,
                                                         store_address_t store_id) {
  cmd->setCommand(CHANGE_SELECTION_DEFINITION);
  cmd->setParameter(addDefinition);
  cmd->setParameter2(store_id.raw);
}

void TmStoreMessage::clear(CommandMessage* cmd) {
  switch (cmd->getCommand()) {
    case SELECTION_DEFINITION_REPORT:
    case STORE_CATALOGUE_REPORT:
    case CHANGE_SELECTION_DEFINITION:
    case INDEX_REPORT:
    case DELETE_STORE_CONTENT_TIME:
    case DOWNLINK_STORE_CONTENT_TIME: {
      StorageManagerIF* ipcStore =
          ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
      if (ipcStore != NULL) {
        ipcStore->deleteData(getStoreId(cmd));
      }
    }
    /* NO BREAK falls through*/
    case DELETE_STORE_CONTENT_BLOCKS:
    case DOWNLINK_STORE_CONTENT_BLOCKS:
    case REPORT_INDEX_REQUEST:
      cmd->setCommand(CommandMessage::UNKNOWN_COMMAND);
      cmd->setParameter(0);
      cmd->setParameter2(0);
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

bool TmStoreMessage::getAddToSelection(CommandMessage* cmd) { return (bool)cmd->getParameter(); }

ReturnValue_t TmStoreMessage::setReportSelectionDefinitionMessage(CommandMessage* cmd) {
  cmd->setCommand(REPORT_SELECTION_DEFINITION);
  return returnvalue::OK;
}

void TmStoreMessage::setSelectionDefinitionReportMessage(CommandMessage* cmd,
                                                         store_address_t storeId) {
  cmd->setCommand(SELECTION_DEFINITION_REPORT);
  cmd->setParameter2(storeId.raw);
}

ReturnValue_t TmStoreMessage::setReportStoreCatalogueMessage(CommandMessage* cmd) {
  cmd->setCommand(REPORT_STORE_CATALOGUE);
  return returnvalue::OK;
}

void TmStoreMessage::setStoreCatalogueReportMessage(CommandMessage* cmd, object_id_t objectId,
                                                    store_address_t storeId) {
  cmd->setCommand(STORE_CATALOGUE_REPORT);
  cmd->setParameter(objectId);
  cmd->setParameter2(storeId.raw);
}

object_id_t TmStoreMessage::getObjectId(CommandMessage* cmd) { return cmd->getParameter(); }

void TmStoreMessage::setDownlinkContentTimeMessage(CommandMessage* cmd, store_address_t storeId) {
  cmd->setCommand(DOWNLINK_STORE_CONTENT_TIME);
  cmd->setParameter2(storeId.raw);
}

uint32_t TmStoreMessage::getAddressLow(CommandMessage* cmd) { return cmd->getParameter(); }
uint32_t TmStoreMessage::getAddressHigh(CommandMessage* cmd) { return cmd->getParameter2(); }

void TmStoreMessage::setDeleteContentTimeMessage(CommandMessage* cmd, store_address_t storeId) {
  cmd->setCommand(DELETE_STORE_CONTENT_TIME);
  cmd->setParameter2(storeId.raw);
}

ReturnValue_t TmStoreMessage::setDeleteBlocksMessage(CommandMessage* cmd, uint32_t addressLow,
                                                     uint32_t addressHigh) {
  cmd->setCommand(DELETE_STORE_CONTENT_BLOCKS);
  cmd->setParameter(addressLow);
  cmd->setParameter2(addressHigh);
  return returnvalue::OK;
}
ReturnValue_t TmStoreMessage::setDownlinkBlocksMessage(CommandMessage* cmd, uint32_t addressLow,
                                                       uint32_t addressHigh) {
  cmd->setCommand(DOWNLINK_STORE_CONTENT_BLOCKS);
  cmd->setParameter(addressLow);
  cmd->setParameter2(addressHigh);
  return returnvalue::OK;
}
ReturnValue_t TmStoreMessage::setIndexRequestMessage(CommandMessage* cmd) {
  cmd->setCommand(REPORT_INDEX_REQUEST);
  return returnvalue::OK;
}

void TmStoreMessage::setIndexReportMessage(CommandMessage* cmd, store_address_t storeId) {
  cmd->setCommand(INDEX_REPORT);
  cmd->setParameter2(storeId.raw);
}
