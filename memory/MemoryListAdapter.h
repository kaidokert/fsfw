/*
 * MemoryListAdapter.h
 *
 *  Created on: 21.03.2014
 *      Author: baetz
 */

#ifndef MEMORYLISTADAPTER_H_
#define MEMORYLISTADAPTER_H_

#include <framework/container/SinglyLinkedList.h>
#include <framework/memory/HasMemoryIF.h>

template<typename T>
class MemoryListAdapter : public SinglyLinkedList<T>, public HasMemoryIF {
public:
	MemoryListAdapter(typename LinkedElement<T>::Iterator start) : SinglyLinkedList<T>(start) {
	}
	MemoryListAdapter() : SinglyLinkedList<T>() {
	}

	ReturnValue_t handleMemoryLoad(uint32_t address, const uint8_t* data, uint32_t size, uint8_t** dataPointer) {
		return handleMemoryLoad(SinglyLinkedList<T>::start, address, data, size, dataPointer);
	}

	static ReturnValue_t handleMemoryLoad(LinkedElement<T>* element, uint32_t address, const uint8_t* data, uint32_t size, uint8_t** dataPointer) {
		ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
		while (!HasMemoryIF::memAccessWasSuccessful(result) && (element != NULL)) {
			result = element->value->handleMemoryLoad(address, data, size, dataPointer);
			element = element->getNext();
		}
		return result;
	}

	ReturnValue_t handleMemoryDump(uint32_t address, uint32_t size, uint8_t** dataPointer, uint8_t* dumpTarget ) {
		return handleMemoryDump(SinglyLinkedList<T>::start, address, size, dataPointer, dumpTarget);
	}
	static ReturnValue_t handleMemoryDump(LinkedElement<T>* element, uint32_t address, uint32_t size, uint8_t** dataPointer, uint8_t* dumpTarget ) {
		ReturnValue_t result = HasReturnvaluesIF::RETURN_FAILED;
		while (!HasMemoryIF::memAccessWasSuccessful(result) && (element != NULL)) {
			result = element->value->handleMemoryDump(address, size, dataPointer, dumpTarget);
			element = element->getNext();
		}
		return result;
	}
};


#endif /* MEMORYLISTADAPTER_H_ */
