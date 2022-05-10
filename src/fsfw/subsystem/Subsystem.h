#ifndef FSFW_SUBSYSTEM_SUBSYSTEM_H_
#define FSFW_SUBSYSTEM_SUBSYSTEM_H_

#include "../container/FixedArrayList.h"
#include "../container/FixedMap.h"
#include "../container/HybridIterator.h"
#include "../container/SinglyLinkedList.h"
#include "../serialize/SerialArrayListAdapter.h"
#include "SubsystemBase.h"
#include "fsfw/FSFW.h"
#include "modes/ModeDefinitions.h"

struct TableSequenceBase {
 public:
  TableSequenceBase(Mode_t mode, ArrayList<ModeListEntry> *table) : mode(mode), table(table){};
  Mode_t mode;
  ArrayList<ModeListEntry> *table;
  bool inStore = false;
  bool preInit = true;
};

struct TableEntry : public TableSequenceBase {
 public:
  TableEntry(Mode_t mode, ArrayList<ModeListEntry> *table) : TableSequenceBase(mode, table){};
};

struct SequenceEntry : public TableSequenceBase {
 public:
  SequenceEntry(Mode_t mode, ArrayList<ModeListEntry> *table, Mode_t fallbackMode)
      : TableSequenceBase(mode, table), fallbackMode(fallbackMode) {}

  Mode_t fallbackMode;
};

/**
 * @brief   TODO: documentation missing
 * @details
 */
class Subsystem : public SubsystemBase, public HasModeSequenceIF {
 public:
  static const uint8_t INTERFACE_ID = CLASS_ID::SUBSYSTEM;
  static const ReturnValue_t SEQUENCE_ALREADY_EXISTS = MAKE_RETURN_CODE(0x01);
  static const ReturnValue_t TABLE_ALREADY_EXISTS = MAKE_RETURN_CODE(0x02);
  static const ReturnValue_t TABLE_DOES_NOT_EXIST = MAKE_RETURN_CODE(0x03);
  static const ReturnValue_t TABLE_OR_SEQUENCE_LENGTH_INVALID = MAKE_RETURN_CODE(0x04);
  static const ReturnValue_t SEQUENCE_DOES_NOT_EXIST = MAKE_RETURN_CODE(0x05);
  static const ReturnValue_t TABLE_CONTAINS_INVALID_OBJECT_ID = MAKE_RETURN_CODE(0x06);
  static const ReturnValue_t FALLBACK_SEQUENCE_DOES_NOT_EXIST = MAKE_RETURN_CODE(0x07);
  static const ReturnValue_t NO_TARGET_TABLE = MAKE_RETURN_CODE(0x08);
  static const ReturnValue_t SEQUENCE_OR_TABLE_TOO_LONG = MAKE_RETURN_CODE(0x09);
  static const ReturnValue_t IS_FALLBACK_SEQUENCE = MAKE_RETURN_CODE(0x0B);
  static const ReturnValue_t ACCESS_DENIED = MAKE_RETURN_CODE(0x0C);
  static const ReturnValue_t TABLE_IN_USE = MAKE_RETURN_CODE(0x0E);

  static const ReturnValue_t TARGET_TABLE_NOT_REACHED = MAKE_RETURN_CODE(0xA1);
  static const ReturnValue_t TABLE_CHECK_FAILED = MAKE_RETURN_CODE(0xA2);

  /**
   * TODO: Doc for constructor
   * @param setObjectId
   * @param parent
   * @param maxNumberOfSequences
   * @param maxNumberOfTables
   */
  Subsystem(object_id_t setObjectId, object_id_t parent, uint32_t maxNumberOfSequences,
            uint32_t maxNumberOfTables);
  virtual ~Subsystem();

  ReturnValue_t addSequence(SequenceEntry sequence);
  ReturnValue_t addSequence(ArrayList<ModeListEntry> *sequence, Mode_t id, Mode_t fallbackSequence,
                            bool inStore = true, bool preInit = true);

  ReturnValue_t addTable(TableEntry table);
  ReturnValue_t addTable(ArrayList<ModeListEntry> *table, Mode_t id, bool inStore = true,
                         bool preInit = true);

  void setInitialMode(Mode_t mode, Submode_t submode = SUBMODE_NONE);

  virtual ReturnValue_t initialize() override;

  virtual ReturnValue_t checkObjectConnections() override;

  virtual MessageQueueId_t getSequenceCommandQueue() const override;

 protected:
  struct EntryPointer {
    bool islinked;
    union {
      ModeListEntry *firstLinkedElement;
      ArrayList<ModeListEntry> *array;
    };
  };

  struct SequenceInfo {
    Mode_t fallbackSequence;
    EntryPointer entries;
  };

  static const uint8_t MAX_NUMBER_OF_TABLES_OR_SEQUENCES = 70;

  static const uint8_t MAX_LENGTH_OF_TABLE_OR_SEQUENCE = 20;

  bool isInTransition;

  bool childrenChangedHealth;

  uint32_t uptimeStartTable = 0;

  HybridIterator<ModeListEntry> currentTargetTable;

  Mode_t targetMode = 0;

  Submode_t targetSubmode;

  Mode_t initialMode = 0;
  Submode_t initSubmode = SUBMODE_NONE;

  HybridIterator<ModeListEntry> currentSequenceIterator;

  FixedMap<Mode_t, EntryPointer> modeTables;

  FixedMap<Mode_t, SequenceInfo> modeSequences;

  StorageManagerIF *IPCStore = nullptr;

#if FSFW_USE_MODESTORE == 1
  ModeStoreIF *modeStore = nullptr;
#endif

  bool existsModeSequence(Mode_t id);

  HybridIterator<ModeListEntry> getSequence(Mode_t id);

  bool existsModeTable(Mode_t id);

  HybridIterator<ModeListEntry> getTable(Mode_t id);

  HybridIterator<ModeListEntry> getCurrentTable();

  /**
   * DO NOT USE ON NON EXISTING SEQUENCE
   *
   * @param a sequence
   * @return the fallback sequence's Id
   */
  Mode_t getFallbackSequence(Mode_t sequence);

  void replyToCommand(ReturnValue_t status, uint32_t parameter);

  ReturnValue_t deleteSequence(Mode_t id);

  ReturnValue_t deleteTable(Mode_t id);

  virtual void performChildOperation() override;

  virtual ReturnValue_t handleCommandMessage(CommandMessage *message) override;

  bool isFallbackSequence(Mode_t SequenceId);

  bool isTableUsed(Mode_t tableId);

  virtual ReturnValue_t checkModeCommand(Mode_t mode, Submode_t submode,
                                         uint32_t *msToReachTheMode) override;

  virtual void startTransition(Mode_t mode, Submode_t submode) override;

  void sendSerializablesAsCommandMessage(Command_t command, SerializeIF **elements, uint8_t count);

  void transitionFailed(ReturnValue_t failureCode, uint32_t parameter);

  void cantKeepMode();

  /**
   * @brief    Checks whether a sequence, identified by a mode.
   * @param sequence
   * @return
   */
  ReturnValue_t checkSequence(Mode_t sequence);

  /**
   * @brief   Checks whether a sequence, identified by a mode list iterator
   *          and a fallback sequence. Iterator needs to point to a valid
   *          sequence.
   * @param iter
   * @return
   */
  ReturnValue_t checkSequence(HybridIterator<ModeListEntry> iter, Mode_t fallbackSequence);
};

#endif /* FSFW_SUBSYSTEM_SUBSYSTEM_H_ */
