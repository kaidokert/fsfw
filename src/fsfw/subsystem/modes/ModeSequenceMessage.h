#ifndef FSFW_SUBSYSTEM_MODES_MODESEQUENCEMESSAGE_H_
#define FSFW_SUBSYSTEM_MODES_MODESEQUENCEMESSAGE_H_

#include "ModeDefinitions.h"
#include "fsfw/ipc/CommandMessage.h"
#include "fsfw/storagemanager/StorageManagerIF.h"

class ModeSequenceMessage {
 public:
  static const uint8_t MESSAGE_ID = messagetypes::MODE_SEQUENCE;

  static const Command_t ADD_SEQUENCE = MAKE_COMMAND_ID(0x01);
  static const Command_t ADD_TABLE = MAKE_COMMAND_ID(0x02);
  static const Command_t DELETE_SEQUENCE = MAKE_COMMAND_ID(0x03);
  static const Command_t DELETE_TABLE = MAKE_COMMAND_ID(0x04);
  static const Command_t READ_SEQUENCE = MAKE_COMMAND_ID(0x05);
  static const Command_t READ_TABLE = MAKE_COMMAND_ID(0x06);
  static const Command_t LIST_SEQUENCES = MAKE_COMMAND_ID(0x07);
  static const Command_t LIST_TABLES = MAKE_COMMAND_ID(0x08);
  static const Command_t SEQUENCE_LIST = MAKE_COMMAND_ID(0x09);
  static const Command_t TABLE_LIST = MAKE_COMMAND_ID(0x0A);
  static const Command_t TABLE = MAKE_COMMAND_ID(0x0B);
  static const Command_t SEQUENCE = MAKE_COMMAND_ID(0x0C);
  static const Command_t READ_FREE_SEQUENCE_SLOTS = MAKE_COMMAND_ID(0x0D);
  static const Command_t FREE_SEQUENCE_SLOTS = MAKE_COMMAND_ID(0x0E);
  static const Command_t READ_FREE_TABLE_SLOTS = MAKE_COMMAND_ID(0x0F);
  static const Command_t FREE_TABLE_SLOTS = MAKE_COMMAND_ID(0x10);

  static void setModeSequenceMessage(CommandMessage *message, Command_t command,
                                     Mode_t sequenceOrTable, store_address_t storeAddress);
  static void setModeSequenceMessage(CommandMessage *message, Command_t command,
                                     Mode_t sequenceOrTable);
  static void setModeSequenceMessage(CommandMessage *message, Command_t command,
                                     store_address_t storeAddress);

  static store_address_t getStoreAddress(const CommandMessage *message);
  static Mode_t getSequenceId(const CommandMessage *message);
  static Mode_t getTableId(const CommandMessage *message);
  static uint32_t getNumber(const CommandMessage *message);

  static void clear(CommandMessage *message);

 private:
  ModeSequenceMessage();
};

#endif /* FSFW_SUBSYSTEM_MODES_MODESEQUENCEMESSAGE_H_ */
