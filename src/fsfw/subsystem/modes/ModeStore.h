#ifndef FSFW_SUBSYSTEM_MODES_MODESTORE_H_
#define FSFW_SUBSYSTEM_MODES_MODESTORE_H_

#include <FSFWConfig.h>

#if FSFW_USE_MODESTORE == 1

#include "../../container/ArrayList.h"
#include "../../container/SinglyLinkedList.h"
#include "../../objectmanager/SystemObject.h"
#include "ModeStoreIF.h"

class ModeStore : public ModeStoreIF, public SystemObject {
 public:
  ModeStore(object_id_t objectId, uint32_t slots);
  virtual ~ModeStore();

  virtual ReturnValue_t storeArray(ArrayList<ModeListEntry> *sequence,
                                   ModeListEntry **storedFirstEntry);

  virtual ReturnValue_t deleteList(ModeListEntry *sequence);

  virtual ReturnValue_t readList(ModeListEntry *sequence, ArrayList<ModeListEntry> *into);

  virtual uint32_t getFreeSlots();

 private:
  MutexId_t *mutex;
  ArrayList<ModeListEntry, uint32_t> store;
  ModeListEntry *emptySlot;

  void clear();
  ModeListEntry *findEmptySlotNoLock(ModeListEntry *startFrom);
  void deleteListNoLock(ModeListEntry *sequence);

  ReturnValue_t isValidEntry(ModeListEntry *sequence);
};

#endif

#endif /* FSFW_SUBSYSTEM_MODES_MODESTORE_H_ */
