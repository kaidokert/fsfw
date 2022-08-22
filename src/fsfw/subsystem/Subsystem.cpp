#include "fsfw/subsystem/Subsystem.h"

#include <string>

#include "fsfw/health/HealthMessage.h"
#include "fsfw/objectmanager/ObjectManager.h"
#include "fsfw/serialize/SerialArrayListAdapter.h"
#include "fsfw/serialize/SerialFixedArrayListAdapter.h"
#include "fsfw/serialize/SerialLinkedListAdapter.h"
#include "fsfw/serialize/SerializeElement.h"

Subsystem::Subsystem(object_id_t setObjectId, object_id_t parent, uint32_t maxNumberOfSequences,
                     uint32_t maxNumberOfTables)
    : SubsystemBase(setObjectId, parent, 0),
      isInTransition(false),
      childrenChangedHealth(false),
      currentTargetTable(),
      targetSubmode(SUBMODE_NONE),
      currentSequenceIterator(),
      modeTables(maxNumberOfTables),
      modeSequences(maxNumberOfSequences) {}

Subsystem::~Subsystem() {}

ReturnValue_t Subsystem::checkSequence(HybridIterator<ModeListEntry> iter,
                                       Mode_t fallbackSequence) {
  // only check for existence, checking the fallback would lead to a (possibly infinite) recursion.
  // the fallback sequence will be checked when it is needed.
  if (!existsModeSequence(fallbackSequence)) {
    return FALLBACK_SEQUENCE_DOES_NOT_EXIST;
  }

  if (iter.value == nullptr) {
    return NO_TARGET_TABLE;
  }

  for (; iter.value != nullptr; ++iter) {
    if (!existsModeTable(iter->getTableId())) {
      return TABLE_DOES_NOT_EXIST;
    } else {
      ReturnValue_t result = checkTable(getTable(iter->getTableId()));
      if (result != returnvalue::OK) {
        return result;
      }
    }
  }
  return returnvalue::OK;
}

ReturnValue_t Subsystem::checkSequence(Mode_t sequence) {
  if (!existsModeSequence(sequence)) {
    return SEQUENCE_DOES_NOT_EXIST;
  }
  HybridIterator<ModeListEntry> iter = getSequence(sequence);
  return checkSequence(iter, getFallbackSequence(sequence));
}

bool Subsystem::existsModeSequence(Mode_t id) {
  return modeSequences.exists(id) == returnvalue::OK;
}

bool Subsystem::existsModeTable(Mode_t id) { return modeTables.exists(id) == returnvalue::OK; }

HybridIterator<ModeListEntry> Subsystem::getCurrentTable() {
  return getTable(currentSequenceIterator->getTableId());
}

void Subsystem::performChildOperation() {
  if (isInTransition) {
    if (commandsOutstanding <= 0) {  // all children of the current table were commanded and replied
      if (currentSequenceIterator.value == nullptr) {  // we're through with this sequence
        if (checkStateAgainstTable(currentTargetTable, targetSubmode) == returnvalue::OK) {
          setMode(targetMode, targetSubmode);
          isInTransition = false;
          return;
        } else {
          Mode_t tableId = 0;
          auto seq = getSequence(targetMode);
          if (seq.value != nullptr) {
            tableId = seq->getTableId();
          }
          transitionFailed(TARGET_TABLE_NOT_REACHED, tableId);
          return;
        }
      }
      if (currentSequenceIterator->checkSuccess()) {
        if (checkStateAgainstTable(getCurrentTable(), targetSubmode) != returnvalue::OK) {
          transitionFailed(TABLE_CHECK_FAILED, currentSequenceIterator->getTableId());
          return;
        }
      }
      if (currentSequenceIterator->getWaitSeconds() != 0) {
        if (uptimeStartTable == 0) {
          Clock::getUptime(&uptimeStartTable);
          return;
        } else {
          uint32_t uptimeNow;
          Clock::getUptime(&uptimeNow);
          if ((uptimeNow - uptimeStartTable) < (currentSequenceIterator->getWaitSeconds() * 1000)) {
            return;
          }
        }
      }
      uptimeStartTable = 0;
      // next Table, but only if there is one
      if ((++currentSequenceIterator).value != NULL) {  // we're through with this sequence
        executeTable(getCurrentTable(), targetSubmode);
      }
    }
  } else {
    if (childrenChangedHealth) {
      triggerEvent(CHILD_CHANGED_HEALTH, 0, 0);
      childrenChangedHealth = false;
      startTransition(mode, submode);
    } else if (childrenChangedMode) {
      if (checkStateAgainstTable(currentTargetTable, submode) != returnvalue::OK) {
        triggerEvent(CANT_KEEP_MODE, mode, submode);
        cantKeepMode();
      }
    }
  }
}

HybridIterator<ModeListEntry> Subsystem::getSequence(Mode_t id) {
  SequenceInfo *sequenceInfo = modeSequences.findValue(id);
  if (sequenceInfo->entries.islinked) {
    return HybridIterator<ModeListEntry>(sequenceInfo->entries.firstLinkedElement);
  } else {
    return HybridIterator<ModeListEntry>(sequenceInfo->entries.array->front(),
                                         sequenceInfo->entries.array->back());
  }
}

HybridIterator<ModeListEntry> Subsystem::getTable(Mode_t id) {
  EntryPointer *entry = modeTables.findValue(id);
  if (entry->islinked) {
    return HybridIterator<ModeListEntry>(entry->firstLinkedElement);
  } else {
    return HybridIterator<ModeListEntry>(entry->array->front(), entry->array->back());
  }
}

ReturnValue_t Subsystem::handleCommandMessage(CommandMessage *message) {
  switch (message->getCommand()) {
    case HealthMessage::HEALTH_INFO: {
      HealthState health = HealthMessage::getHealth(message);
      if (health != EXTERNAL_CONTROL) {
        // Ignore external control, as it has an effect only if the mode changes,
        // which is communicated with an additional mode info event.
        childrenChangedHealth = true;
      }
    } break;
    case ModeSequenceMessage::ADD_SEQUENCE: {
      FixedArrayList<ModeListEntry, MAX_LENGTH_OF_TABLE_OR_SEQUENCE> sequence;
      const uint8_t *pointer;
      size_t sizeRead;
      ReturnValue_t result =
          IPCStore->getData(ModeSequenceMessage::getStoreAddress(message), &pointer, &sizeRead);
      if (result == returnvalue::OK) {
        Mode_t fallbackId;
        size_t size = sizeRead;
        result = SerializeAdapter::deSerialize(&fallbackId, &pointer, &size,
                                               SerializeIF::Endianness::BIG);
        if (result == returnvalue::OK) {
          result = SerialArrayListAdapter<ModeListEntry>::deSerialize(&sequence, &pointer, &size,
                                                                      SerializeIF::Endianness::BIG);
          if (result == returnvalue::OK) {
            result =
                addSequence(&sequence, ModeSequenceMessage::getSequenceId(message), fallbackId);
          }
        }
        IPCStore->deleteData(ModeSequenceMessage::getStoreAddress(message));
      }
      replyToCommand(result, 0);
    } break;
    case ModeSequenceMessage::ADD_TABLE: {
      FixedArrayList<ModeListEntry, MAX_LENGTH_OF_TABLE_OR_SEQUENCE> table;
      const uint8_t *pointer;
      size_t sizeRead;
      ReturnValue_t result =
          IPCStore->getData(ModeSequenceMessage::getStoreAddress(message), &pointer, &sizeRead);
      if (result == returnvalue::OK) {
        size_t size = sizeRead;
        result = SerialArrayListAdapter<ModeListEntry>::deSerialize(&table, &pointer, &size,
                                                                    SerializeIF::Endianness::BIG);
        if (result == returnvalue::OK) {
          result = addTable(&table, ModeSequenceMessage::getSequenceId(message));
        }
        IPCStore->deleteData(ModeSequenceMessage::getStoreAddress(message));
      }
      replyToCommand(result, 0);

    } break;
    case ModeSequenceMessage::DELETE_SEQUENCE: {
      if (isInTransition) {
        replyToCommand(IN_TRANSITION, 0);
        break;
      }
      ReturnValue_t result = deleteSequence(ModeSequenceMessage::getSequenceId(message));
      replyToCommand(result, 0);
    } break;
    case ModeSequenceMessage::DELETE_TABLE: {
      if (isInTransition) {
        replyToCommand(IN_TRANSITION, 0);
        break;
      }
      ReturnValue_t result = deleteTable(ModeSequenceMessage::getTableId(message));
      replyToCommand(result, 0);
    } break;
    case ModeSequenceMessage::LIST_SEQUENCES: {
      SerialFixedArrayListAdapter<Mode_t, MAX_NUMBER_OF_TABLES_OR_SEQUENCES> sequences;
      FixedMap<Mode_t, SequenceInfo>::Iterator iter;
      for (iter = modeSequences.begin(); iter != modeSequences.end(); ++iter) {
        sequences.insert(iter.value->first);
      }
      SerializeIF *pointer = &sequences;
      sendSerializablesAsCommandMessage(ModeSequenceMessage::SEQUENCE_LIST, &pointer, 1);
    } break;
    case ModeSequenceMessage::LIST_TABLES: {
      SerialFixedArrayListAdapter<Mode_t, MAX_NUMBER_OF_TABLES_OR_SEQUENCES> tables;
      FixedMap<Mode_t, EntryPointer>::Iterator iter;
      for (iter = modeTables.begin(); iter != modeTables.end(); ++iter) {
        tables.insert(iter.value->first);
      }
      SerializeIF *pointer = &tables;
      sendSerializablesAsCommandMessage(ModeSequenceMessage::TABLE_LIST, &pointer, 1);
    } break;
    case ModeSequenceMessage::READ_SEQUENCE: {
      ReturnValue_t result;
      Mode_t sequence = ModeSequenceMessage::getSequenceId(message);
      SequenceInfo *sequenceInfo = NULL;
      result = modeSequences.find(sequence, &sequenceInfo);
      if (result != returnvalue::OK) {
        replyToCommand(result, 0);
      }

      SerializeIF *elements[3];
      SerializeElement<Mode_t> sequenceId(sequence);
      SerializeElement<Mode_t> fallbackSequenceId(getFallbackSequence(sequence));

      elements[0] = &sequenceId;
      elements[1] = &fallbackSequenceId;

      if (sequenceInfo->entries.islinked) {
        SerialLinkedListAdapter<ModeListEntry> list(sequenceInfo->entries.firstLinkedElement, true);
        elements[2] = &list;
        sendSerializablesAsCommandMessage(ModeSequenceMessage::SEQUENCE, elements, 3);
      } else {
        SerialArrayListAdapter<ModeListEntry> serializableArray(sequenceInfo->entries.array);

        elements[2] = &serializableArray;
        sendSerializablesAsCommandMessage(ModeSequenceMessage::SEQUENCE, elements, 3);
      }
    } break;
    case ModeSequenceMessage::READ_TABLE: {
      ReturnValue_t result;
      Mode_t table = ModeSequenceMessage::getSequenceId(message);
      EntryPointer *entry = nullptr;
      result = modeTables.find(table, &entry);
      if (result != returnvalue::OK or entry == nullptr) {
        replyToCommand(result, 0);
        if (entry == nullptr) {
          return result;
        }
      }

      SerializeIF *elements[2];
      SerializeElement<Mode_t> tableId(table);

      elements[0] = &tableId;

      if (entry->islinked) {
        SerialLinkedListAdapter<ModeListEntry> list(entry->firstLinkedElement, true);
        elements[1] = &list;
        sendSerializablesAsCommandMessage(ModeSequenceMessage::TABLE, elements, 2);
      } else {
        SerialArrayListAdapter<ModeListEntry> serializableArray(entry->array);
        elements[1] = &serializableArray;
        sendSerializablesAsCommandMessage(ModeSequenceMessage::TABLE, elements, 2);
      }
    } break;
    case ModeSequenceMessage::READ_FREE_SEQUENCE_SLOTS: {
      uint32_t freeSlots = modeSequences.maxSize() - modeSequences.size();
      CommandMessage reply;
      ModeSequenceMessage::setModeSequenceMessage(&reply, ModeSequenceMessage::FREE_SEQUENCE_SLOTS,
                                                  freeSlots);
      commandQueue->reply(&reply);
    } break;
    case ModeSequenceMessage::READ_FREE_TABLE_SLOTS: {
      uint32_t free = modeTables.maxSize() - modeTables.size();
      CommandMessage reply;
      ModeSequenceMessage::setModeSequenceMessage(&reply, ModeSequenceMessage::FREE_TABLE_SLOTS,
                                                  free);
      commandQueue->reply(&reply);
    } break;
    default:
      return returnvalue::FAILED;
  }
  return returnvalue::OK;
}

void Subsystem::replyToCommand(ReturnValue_t status, uint32_t parameter) {
  if (status == returnvalue::OK) {
    CommandMessage reply(CommandMessage::REPLY_COMMAND_OK, 0, 0);
    commandQueue->reply(&reply);
  } else {
    CommandMessage reply(CommandMessage::REPLY_REJECTED, status, 0);
    commandQueue->reply(&reply);
  }
}

ReturnValue_t Subsystem::addSequence(SequenceEntry sequence) {
  return addSequence(sequence.table, sequence.mode, sequence.fallbackMode, sequence.inStore,
                     sequence.preInit);
}

ReturnValue_t Subsystem::addSequence(ArrayList<ModeListEntry> *sequence, Mode_t id,
                                     Mode_t fallbackSequence, bool inStore, bool preInit) {
  ReturnValue_t result;

  // Before initialize() is called, tables must not be checked as the
  // children are not added yet.
  // Sequences added before are checked by initialize()
  if (!preInit) {
    result = checkSequence(HybridIterator<ModeListEntry>(sequence->front(), sequence->back()),
                           fallbackSequence);
    if (result != returnvalue::OK) {
      return result;
    }
  }

  SequenceInfo info;

  info.fallbackSequence = fallbackSequence;

  info.entries.islinked = inStore;
  info.entries.array = sequence;

  result = modeSequences.insert(id, info);

  if (result != returnvalue::OK) {
    return result;
  }

  if (inStore) {
#if FSFW_USE_MODESTORE == 1
    result = modeStore->storeArray(sequence, &(modeSequences.find(id)->entries.firstLinkedElement));
    if (result != returnvalue::OK) {
      modeSequences.erase(id);
    }
#else
    modeSequences.erase(id);
    return returnvalue::FAILED;
#endif
  }

  return result;
}

ReturnValue_t Subsystem::addTable(TableEntry table) {
  return addTable(table.table, table.mode, table.inStore, table.preInit);
}

ReturnValue_t Subsystem::addTable(ArrayList<ModeListEntry> *table, Mode_t id, bool inStore,
                                  bool preInit) {
  ReturnValue_t result;

  // Before initialize() is called, tables must not be checked as the children
  // are not added yet. Tables added before are checked by initialize()
  if (!preInit) {
    result = checkTable(HybridIterator<ModeListEntry>(table->front(), table->back()));
    if (result != returnvalue::OK) {
      return result;
    }
  }

  EntryPointer pointer;

  pointer.islinked = inStore;
  pointer.array = table;

  result = modeTables.insert(id, pointer);

  if (result != returnvalue::OK) {
    return result;
  }

  if (inStore) {
#if FSFW_USE_MODESTORE == 1
    result = modeStore->storeArray(table, &(modeTables.find(id)->firstLinkedElement));
    if (result != returnvalue::OK) {
      modeTables.erase(id);
    }
#else
    modeTables.erase(id);
    return returnvalue::FAILED;
#endif
  }
  return result;
}

ReturnValue_t Subsystem::deleteSequence(Mode_t id) {
  if (isFallbackSequence(id)) {
    return IS_FALLBACK_SEQUENCE;
  }

  SequenceInfo *sequenceInfo;
  ReturnValue_t result;
  result = modeSequences.find(id, &sequenceInfo);
  if (result != returnvalue::OK) {
    return result;
  }
  if (!sequenceInfo->entries.islinked) {
    return ACCESS_DENIED;
  }

#if FSFW_USE_MODESTORE == 1
  modeStore->deleteList(sequenceInfo->entries.firstLinkedElement);
#endif
  modeSequences.erase(id);
  return returnvalue::OK;
}

ReturnValue_t Subsystem::deleteTable(Mode_t id) {
  if (isTableUsed(id)) {
    return TABLE_IN_USE;
  }

  EntryPointer *pointer;
  ReturnValue_t result;
  result = modeTables.find(id, &pointer);
  if (result != returnvalue::OK) {
    return result;
  }
  if (!pointer->islinked) {
    return ACCESS_DENIED;
  }

#if FSFW_USE_MODESTORE == 1
  modeStore->deleteList(pointer->firstLinkedElement);
#endif
  modeSequences.erase(id);
  return returnvalue::OK;
}

ReturnValue_t Subsystem::initialize() {
  ReturnValue_t result = SubsystemBase::initialize();

  if (result != returnvalue::OK) {
    return result;
  }

  IPCStore = ObjectManager::instance()->get<StorageManagerIF>(objects::IPC_STORE);
  if (IPCStore == NULL) {
    return returnvalue::FAILED;
  }

#if FSFW_USE_MODESTORE == 1
  modeStore = ObjectManager::instance()->get<ModeStoreIF>(objects::MODE_STORE);

  if (modeStore == nullptr) {
    return returnvalue::FAILED;
  }
#endif

  if ((modeSequences.maxSize() > MAX_NUMBER_OF_TABLES_OR_SEQUENCES) ||
      (modeTables.maxSize() > MAX_NUMBER_OF_TABLES_OR_SEQUENCES)) {
    return TABLE_OR_SEQUENCE_LENGTH_INVALID;
  }

  mode = initialMode;
  submode = initSubmode;

  return returnvalue::OK;
}

MessageQueueId_t Subsystem::getSequenceCommandQueue() const {
  return SubsystemBase::getCommandQueue();
}

ReturnValue_t Subsystem::checkModeCommand(Mode_t mode, Submode_t submode,
                                          uint32_t *msToReachTheMode) {
  // Need to accept all submodes to be able to inherit submodes
  //	if (submode != SUBMODE_NONE) {
  //		return INVALID_SUBMODE;
  //	}

  if (isInTransition && (mode != getFallbackSequence(targetMode))) {
    return HasModesIF::IN_TRANSITION;
  } else {
    return checkSequence(mode);
  }
}

void Subsystem::startTransition(Mode_t sequence, Submode_t submode) {
  if (modeHelper.isForced()) {
    triggerEvent(FORCING_MODE, sequence, submode);
  } else {
    triggerEvent(CHANGING_MODE, sequence, submode);
  }
  targetMode = sequence;
  targetSubmode = submode;
  isInTransition = true;
  commandsOutstanding = 0;
  currentSequenceIterator = getSequence(sequence);

  currentTargetTable = getTable(currentSequenceIterator->getTableId());

  ++currentSequenceIterator;

  if (currentSequenceIterator.value != NULL) {
    executeTable(getCurrentTable(), targetSubmode);
  }
}

Mode_t Subsystem::getFallbackSequence(Mode_t sequence) {
  for (FixedMap<Mode_t, SequenceInfo>::Iterator iter = modeSequences.begin();
       iter != modeSequences.end(); ++iter) {
    if (iter.value->first == sequence) {
      return iter->second.fallbackSequence;
    }
  }
  return -1;
}

bool Subsystem::isFallbackSequence(Mode_t SequenceId) {
  for (FixedMap<Mode_t, SequenceInfo>::Iterator iter = modeSequences.begin();
       iter != modeSequences.end(); iter++) {
    if (iter->second.fallbackSequence == SequenceId) {
      return true;
    }
  }
  return false;
}

bool Subsystem::isTableUsed(Mode_t tableId) {
  for (FixedMap<Mode_t, SequenceInfo>::Iterator sequence = modeSequences.begin();
       sequence != modeSequences.end(); sequence++) {
    HybridIterator<ModeListEntry> sequenceIterator = getSequence(sequence.value->first);
    while (sequenceIterator.value != NULL) {
      if (sequenceIterator->getTableId() == tableId) {
        return true;
      }
      ++sequenceIterator;
    }
  }
  return false;
}

void Subsystem::transitionFailed(ReturnValue_t failureCode, uint32_t parameter) {
  triggerEvent(MODE_TRANSITION_FAILED, failureCode, parameter);
  if (mode == targetMode) {
    // already tried going back to the current mode
    // go into fallback mode, also set current mode to fallback mode,
    // so we come here at the next fail
    modeHelper.setForced(true);
    ReturnValue_t result;
    if ((result = checkSequence(getFallbackSequence(mode))) != returnvalue::OK) {
      triggerEvent(FALLBACK_FAILED, result, getFallbackSequence(mode));
      // keep still and allow arbitrary mode commands to recover
      isInTransition = false;
      return;
    }
    mode = getFallbackSequence(mode);
    startTransition(mode, submode);
  } else {
    // try to go back to the current mode
    startTransition(mode, submode);
  }
}

void Subsystem::sendSerializablesAsCommandMessage(Command_t command, SerializeIF **elements,
                                                  uint8_t count) {
  ReturnValue_t result;
  size_t maxSize = 0;
  for (uint8_t i = 0; i < count; i++) {
    maxSize += elements[i]->getSerializedSize();
  }
  uint8_t *storeBuffer;
  store_address_t address;
  size_t size = 0;

  result = IPCStore->getFreeElement(&address, maxSize, &storeBuffer);
  if (result != returnvalue::OK) {
    replyToCommand(result, 0);
    return;
  }
  for (uint8_t i = 0; i < count; i++) {
    elements[i]->serialize(&storeBuffer, &size, maxSize, SerializeIF::Endianness::BIG);
  }
  CommandMessage reply;
  ModeSequenceMessage::setModeSequenceMessage(&reply, command, address);
  if (commandQueue->reply(&reply) != returnvalue::OK) {
    IPCStore->deleteData(address);
  }
}

ReturnValue_t Subsystem::checkObjectConnections() {
  ReturnValue_t result = returnvalue::OK;
  for (FixedMap<Mode_t, SequenceInfo>::Iterator iter = modeSequences.begin();
       iter != modeSequences.end(); iter++) {
    result = checkSequence(iter.value->first);
    if (result != returnvalue::OK) {
      return result;
    }
  }
  return returnvalue::OK;
}

void Subsystem::setInitialMode(Mode_t mode, Submode_t submode) {
  this->initialMode = mode;
  this->initSubmode = submode;
}

void Subsystem::cantKeepMode() {
  ReturnValue_t result;
  if ((result = checkSequence(getFallbackSequence(mode))) != returnvalue::OK) {
    triggerEvent(FALLBACK_FAILED, result, getFallbackSequence(mode));
    return;
  }

  modeHelper.setForced(true);

  // already set the mode, so that we do not try to go back in our old mode
  // when the transition fails
  mode = getFallbackSequence(mode);
  // SHOULDDO: We should store submodes for fallback sequence as well,
  // otherwise we should get rid of submodes completely.
  startTransition(mode, SUBMODE_NONE);
}
