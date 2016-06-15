/*
 * SetAddressListAdapter.h
 *
 *  Created on: 24.04.2014
 *      Author: baetz
 */

#ifndef SETADDRESSLISTADAPTER_H_
#define SETADDRESSLISTADAPTER_H_
#include <framework/container/SinglyLinkedList.h>
#include <framework/memory/HasMemoryIF.h>

template <typename T>
class SetAddressListAdapter {
public:
	static ReturnValue_t setAddresses(const LinkedElement<T>* element, uint32_t startFrom) {
		ReturnValue_t result = HasReturnvaluesIF::RETURN_OK;
		while ((result == HasReturnvaluesIF::RETURN_OK) && (element != NULL)) {
			result = element->value->setAddress(&startFrom);
			element = element->getNext();
			startFrom++;
		}
		return result;
	}
};



#endif /* SETADDRESSLISTADAPTER_H_ */
