#include "fsfw/subsystem/modes/ModeStore.h"

// todo: I think some parts are deprecated. If this is used,  the define
// USE_MODESTORE could be part of the new FSFWConfig.h file.
#if FSFW_USE_MODESTORE == 1

ModeStore::ModeStore(object_id_t objectId, uint32_t slots)
    : SystemObject(objectId), store(slots), emptySlot(store.front()) {
  mutex = MutexFactory::instance()->createMutex();
  ;
  OSAL::createMutex(objectId + 1, mutex);
  clear();
}

ModeStore::~ModeStore() { delete mutex; }

uint32_t ModeStore::getFreeSlots() {
  OSAL::lockMutex(mutex, OSAL::NO_TIMEOUT);
  uint32_t count = 0;
  ArrayList<ModeListEntry, uint32_t>::Iterator iter;
  for (iter = store.begin(); iter != store.end(); ++iter) {
    if (iter->getNext() == emptySlot) {
      ++count;
    }
  }
  OSAL::unlockMutex(mutex);
  return count;
}

ReturnValue_t ModeStore::storeArray(ArrayList<ModeListEntry>* sequence,
                                    ModeListEntry** storedFirstEntry) {
  if (sequence->size == 0) {
    return CANT_STORE_EMPTY;
  }
  OSAL::lockMutex(mutex, OSAL::NO_TIMEOUT);
  *storedFirstEntry = findEmptySlotNoLock(store.front());

  ModeListEntry* pointer = *storedFirstEntry;
  pointer->setNext(pointer);

  ArrayList<ModeListEntry>::Iterator iter;
  for (iter = sequence->begin(); iter != sequence->end(); ++iter) {
    // SHOULDDO: I need to check this in detail. What is the idea? Why does it not work?
    pointer = pointer->getNext()->value;
    if (pointer == NULL) {
      deleteListNoLock(*storedFirstEntry);
      OSAL::unlockMutex(mutex);
      return TOO_MANY_ELEMENTS;
    }
    pointer->value->value1 = iter->value1;
    pointer->value->value2 = iter->value2;
    pointer->value->value3 = iter->value3;
    pointer->setNext(findEmptySlotNoLock(pointer + 1));
  }
  pointer->setNext(NULL);
  OSAL::unlockMutex(mutex);
  return returnvalue::OK;
}

ReturnValue_t ModeStore::deleteList(ModeListEntry* sequence) {
  ReturnValue_t result = isValidEntry(sequence);
  if (result != returnvalue::OK) {
    return result;
  }
  OSAL::lockMutex(mutex, OSAL::NO_TIMEOUT);
  deleteListNoLock(sequence);
  OSAL::unlockMutex(mutex);
  return returnvalue::OK;
}

ReturnValue_t ModeStore::readList(ModeListEntry* sequence, ArrayList<ModeListEntry>* into) {
  ReturnValue_t result = isValidEntry(sequence);
  if (result != returnvalue::OK) {
    return result;
  }
  OSAL::lockMutex(mutex, OSAL::NO_TIMEOUT);
  result = into->insert(*sequence->value);
  while ((result == returnvalue::OK) && (sequence->getNext() != NULL)) {
    result = into->insert(*sequence->value);
    sequence = sequence->getNext()->value;
  }
  OSAL::unlockMutex(mutex);
  return result;
}

void ModeStore::clear() {
  OSAL::lockMutex(mutex, OSAL::NO_TIMEOUT);
  store.size = store.maxSize();
  ArrayList<ModeListEntry, uint32_t>::Iterator iter;
  for (iter = store.begin(); iter != store.end(); ++iter) {
    iter->setNext(emptySlot);
  }
  OSAL::unlockMutex(mutex);
}

ModeListEntry* ModeStore::findEmptySlotNoLock(ModeListEntry* startFrom) {
  ArrayList<ModeListEntry, uint32_t>::Iterator iter(startFrom);
  for (; iter != store.end(); ++iter) {
    if (iter.value->getNext() == emptySlot) {
      OSAL::unlockMutex(mutex);
      return iter.value;
    }
  }
  return NULL;
}

void ModeStore::deleteListNoLock(ModeListEntry* sequence) {
  ModeListEntry* next = sequence;
  while (next != NULL) {
    next = sequence->getNext()->value;
    sequence->setNext(emptySlot);
    sequence = next;
  }
}

ReturnValue_t ModeStore::isValidEntry(ModeListEntry* sequence) {
  if ((sequence < store.front()) || (sequence > store.back()) || sequence->getNext() == emptySlot) {
    return INVALID_ENTRY;
  }
  return returnvalue::OK;
}

#endif
