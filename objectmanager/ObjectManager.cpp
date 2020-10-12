#include "ObjectManager.h"
#include "../serviceinterface/ServiceInterfaceStream.h"
#include <iomanip>
#include <cstdlib>

ObjectManager::ObjectManager( void (*setProducer)() ):
		produceObjects(setProducer) {
	//There's nothing special to do in the constructor.
}


ObjectManager::~ObjectManager() {
	for (auto const& iter : objectList) {
		delete iter.second;
	}
}

ReturnValue_t ObjectManager::insert( object_id_t id, SystemObjectIF* object) {
	auto returnPair = objectList.emplace(id, object);
	if (returnPair.second) {
	    // sif::debug << "ObjectManager::insert: Object " << std::hex
	    //            << (int)id << std::dec << " inserted." << std::endl;
		return this->RETURN_OK;
	} else {
		sif::error << "ObjectManager::insert: Object id " << std::hex
		           << (int)id << std::dec << " is already in use!" << std::endl;
		sif::error << "Terminating program." << std::endl;
		//This is very severe and difficult to handle in other places.
		std::exit(INSERTION_FAILED);
	}
}

ReturnValue_t ObjectManager::remove( object_id_t id ) {
	if ( this->getSystemObject(id) != NULL ) {
		this->objectList.erase( id );
		//sif::debug << "ObjectManager::removeObject: Object " << std::hex
		//           << (int)id << std::dec << " removed." << std::endl;
		return RETURN_OK;
	} else {
		sif::error << "ObjectManager::removeObject: Requested object "
		        << std::hex << (int)id << std::dec << " not found." << std::endl;
		return NOT_FOUND;
	}
}



SystemObjectIF* ObjectManager::getSystemObject( object_id_t id ) {
	auto listIter = this->objectList.find( id );
	if (listIter == this->objectList.end() ) {
		return nullptr;
	} else {
		return listIter->second;
	}
}

ObjectManager::ObjectManager() : produceObjects(nullptr) {

}

void ObjectManager::initialize() {
	if(produceObjects == nullptr) {
		sif::error << "ObjectManager::initialize: Passed produceObjects "
				"functions is nullptr!" << std::endl;
		return;
	}
	this->produceObjects();
	ReturnValue_t result = RETURN_FAILED;
	uint32_t errorCount = 0;
	for (auto const& it : objectList) {
		result = it.second->initialize();
		if ( result != RETURN_OK ) {
			object_id_t var = it.first;
			sif::error << "ObjectManager::initialize: Object 0x" << std::hex <<
					std::setw(8) << std::setfill('0')<< var << " failed to "
					"initialize with code 0x" << result << std::dec <<
					std::setfill(' ') << std::endl;
			errorCount++;
		}
	}
	if (errorCount > 0) {
		sif::error << "ObjectManager::ObjectManager: Counted " << errorCount
		           << " failed initializations." << std::endl;
	}
	//Init was successful. Now check successful interconnections.
	errorCount = 0;
	for (auto const& it : objectList) {
		result = it.second->checkObjectConnections();
		if ( result != RETURN_OK ) {
			sif::error << "ObjectManager::ObjectManager: Object " << std::hex <<
					(int) it.first  << " connection check failed with code 0x"
					<< result << std::dec << std::endl;
			errorCount++;
		}
	}
	if (errorCount > 0) {
		sif::error << "ObjectManager::ObjectManager: Counted " << errorCount
		           << " failed connection checks." << std::endl;
	}
}

void ObjectManager::printList() {
	sif::debug << "ObjectManager: Object List contains:" << std::endl;
	for (auto const& it : objectList) {
		sif::debug << std::hex << it.first << " | " << it.second << std::endl;
	}
}
