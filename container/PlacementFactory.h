/*
 * PlacementFactory.h
 *
 *  Created on: 10.03.2015
 *      Author: baetz
 */

#ifndef FRAMEWORK_CONTAINER_PLACEMENTFACTORY_H_
#define FRAMEWORK_CONTAINER_PLACEMENTFACTORY_H_

#include <framework/storagemanager/StorageManagerIF.h>
#include <utility>

class PlacementFactory {
public:
	PlacementFactory(StorageManagerIF* backend) :
			dataBackend(backend) {
	}
	template<typename T, typename ... Args>
	T* generate(Args&&... args) {
		store_address_t tempId;
		uint8_t* pData = NULL;
		ReturnValue_t result = dataBackend->getFreeElement(&tempId, sizeof(T),
				&pData);
		if (result != HasReturnvaluesIF::RETURN_OK) {
			return NULL;
		}
		T* temp = new (pData) T(std::forward<Args>(args)...);
		return temp;
	}
	template<typename T>
	ReturnValue_t destroy(T* thisElement) {
		//TODO: Shouldn't we call the destructor here first, in case something was allocated by the object (shouldn't do that, however).
		uint8_t* pointer = (uint8_t*) (thisElement);
		return dataBackend->deleteData(pointer, sizeof(T));
	}
private:
	StorageManagerIF* dataBackend;
};

#endif /* FRAMEWORK_CONTAINER_PLACEMENTFACTORY_H_ */
