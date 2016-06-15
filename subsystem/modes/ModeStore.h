#ifndef MODESTORE_H_
#define MODESTORE_H_

#include <framework/container/ArrayList.h>
#include <framework/container/SinglyLinkedList.h>
#include <framework/objectmanager/SystemObject.h>
#include <framework/subsystem/modes/ModeStoreIF.h>

class ModeStore: public ModeStoreIF, public SystemObject {
public:
	ModeStore(object_id_t objectId, uint32_t slots);
	virtual ~ModeStore();

	virtual ReturnValue_t storeArray(ArrayList<ModeListEntry> *sequence,
			ModeListEntry **storedFirstEntry);

	virtual ReturnValue_t deleteList(
			ModeListEntry *sequence);

	virtual ReturnValue_t readList(
			ModeListEntry *sequence,
			ArrayList<ModeListEntry> *into);

	virtual uint32_t getFreeSlots();

private:
	MutexId_t* mutex;
	ArrayList<ModeListEntry, uint32_t> store;
	ModeListEntry *emptySlot;

	void clear();
	ModeListEntry* findEmptySlotNoLock(
			ModeListEntry* startFrom);
	void deleteListNoLock(
			ModeListEntry *sequence);

	ReturnValue_t isValidEntry(ModeListEntry *sequence);
};

#endif /* MODESTORE_H_ */

