#include "ObjectManager.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

#if CPP_OSTREAM_ENABLED == 1
#include <iomanip>
#endif
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
#if CPP_OSTREAM_ENABLED == 1
	    // sif::debug << "ObjectManager::insert: Object " << std::hex
	    //            << (int)id << std::dec << " inserted." << std::endl;
#endif
		return this->RETURN_OK;
	} else {
#if CPP_OSTREAM_ENABLED == 1
		sif::error << "ObjectManager::insert: Object id " << std::hex
		           << static_cast<uint32_t> id << std::dec
				   << " is already in use!" << std::endl;
		sif::error << "Terminating program." << std::endl;
#endif
		//This is very severe and difficult to handle in other places.
		std::exit(INSERTION_FAILED);
	}
}

ReturnValue_t ObjectManager::remove( object_id_t id ) {
	if ( this->getSystemObject(id) != NULL ) {
		this->objectList.erase( id );
#if CPP_OSTREAM_ENABLED == 1
		//sif::debug << "ObjectManager::removeObject: Object " << std::hex
		//           << (int)id << std::dec << " removed." << std::endl;
#endif
		return RETURN_OK;
	} else {
#if CPP_OSTREAM_ENABLED == 1
		sif::error << "ObjectManager::removeObject: Requested object "
		        << std::hex << (int)id << std::dec << " not found." << std::endl;
#endif
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
#if CPP_OSTREAM_ENABLED == 1
		sif::error << "ObjectManager::initialize: Passed produceObjects "
				"functions is nullptr!" << std::endl;
#endif
		return;
	}
	this->produceObjects();
	ReturnValue_t result = RETURN_FAILED;
	uint32_t errorCount = 0;
	for (auto const& it : objectList) {
		result = it.second->initialize();
		if ( result != RETURN_OK ) {
#if CPP_OSTREAM_ENABLED == 1
			object_id_t var = it.first;
			sif::error << "ObjectManager::initialize: Object 0x" << std::hex <<
					std::setw(8) << std::setfill('0')<< var << " failed to "
					"initialize with code 0x" << result << std::dec <<
					std::setfill(' ') << std::endl;
#endif
			errorCount++;
		}
	}
	if (errorCount > 0) {
#if CPP_OSTREAM_ENABLED == 1
		sif::error << "ObjectManager::ObjectManager: Counted " << errorCount
		           << " failed initializations." << std::endl;
#endif
	}
	//Init was successful. Now check successful interconnections.
	errorCount = 0;
	for (auto const& it : objectList) {
		result = it.second->checkObjectConnections();
		if ( result != RETURN_OK ) {
#if CPP_OSTREAM_ENABLED == 1
			sif::error << "ObjectManager::ObjectManager: Object " << std::hex <<
					(int) it.first  << " connection check failed with code 0x"
					<< result << std::dec << std::endl;
#endif
			errorCount++;
		}
	}
	if (errorCount > 0) {
#if CPP_OSTREAM_ENABLED == 1
		sif::error << "ObjectManager::ObjectManager: Counted " << errorCount
		           << " failed connection checks." << std::endl;
#endif
	}
}

void ObjectManager::printList() {
#if CPP_OSTREAM_ENABLED == 1
	sif::debug << "ObjectManager: Object List contains:" << std::endl;
	for (auto const& it : objectList) {
		sif::debug << std::hex << it.first << " | " << it.second << std::endl;

	}
#endif
}
