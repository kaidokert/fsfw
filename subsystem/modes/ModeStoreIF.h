#ifndef MODESTOREIF_H_
#define MODESTOREIF_H_

#ifdef USE_MODESTORE

#include <framework/container/ArrayList.h>
#include <framework/container/SinglyLinkedList.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <framework/subsystem/modes/ModeDefinitions.h>

class ModeStoreIF {
public:
	static const uint8_t INTERFACE_ID = CLASS_ID::MODE_STORE_IF;
	static const ReturnValue_t INVALID_ENTRY = MAKE_RETURN_CODE(0x02);
	static const ReturnValue_t TOO_MANY_ELEMENTS = MAKE_RETURN_CODE(0x03);
	static const ReturnValue_t CANT_STORE_EMPTY = MAKE_RETURN_CODE(0x04);

	virtual ~ModeStoreIF() {

	}

	virtual ReturnValue_t storeArray(ArrayList<ModeListEntry> *sequence,
			ModeListEntry **storedFirstEntry) = 0;

	virtual ReturnValue_t deleteList(
			ModeListEntry *sequence) = 0;

	virtual ReturnValue_t readList(
			ModeListEntry *sequence,
			ArrayList<ModeListEntry> *into) = 0;

	virtual uint32_t getFreeSlots() = 0;
};

#endif

#endif /* MODESTOREIF_H_ */
