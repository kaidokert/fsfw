#include <framework/objectmanager/ObjectManager.h>
#include <framework/serviceinterface/ServiceInterfaceStream.h>
#include <cstdlib>

ObjectManager::ObjectManager( void (*setProducer)() ) : produceObjects(setProducer) {
	//There's nothing special to do in the constructor.
}


ObjectManager::~ObjectManager() {
	std::map<object_id_t, SystemObjectIF*>::iterator it;
	for (it = this->objectList.begin(); it != this->objectList.end(); it++) {
		delete it->second;
	}
}

ReturnValue_t ObjectManager::insert( object_id_t id, SystemObjectIF* object) {
	bool insert_return = this->objectList.insert( std::pair< object_id_t, SystemObjectIF* >( id, object ) ).second;
	if (insert_return == true) {
//		debug << "ObjectManager::insert: Object " << std::hex << (int)id << std::dec << " inserted." << std::endl;
		return this->RETURN_OK;
	} else {
		error << "ObjectManager::insert: Object id " << std::hex << (int)id << std::dec << " is already in use!" << std::endl;
		exit(0); //This is very severe and difficult to handle in other places.
		return this->INSERTION_FAILED;
	}
}

ReturnValue_t ObjectManager::remove( object_id_t id ) {
	if ( this->getSystemObject(id) != NULL ) {
		this->objectList.erase( id );
		debug << "ObjectManager::removeObject: Object " << std::hex << (int)id << std::dec << " removed." << std::endl;
		return RETURN_OK;
	} else {
		error << "ObjectManager::removeObject: Requested object "<< std::hex << (int)id << std::dec << " not found." << std::endl;
		return NOT_FOUND;
	}
}



SystemObjectIF* ObjectManager::getSystemObject( object_id_t id ) {
	std::map<object_id_t, SystemObjectIF*>::iterator it = this->objectList.find( id );
	if (it == this->objectList.end() ) {
		//Changed for testing different method.
//		SystemObjectIF* object = this->produceObjects( id );
//		return object;
		return NULL;
	} else {
		return it->second;
	}
}

ObjectManager::ObjectManager( ) : produceObjects(NULL) {

}

void ObjectManager::initialize() {
	this->produceObjects();
	ReturnValue_t return_value = RETURN_FAILED;
	uint32_t error_count = 0;
	for (std::map<object_id_t, SystemObjectIF*>::iterator it = this->objectList.begin(); it != objectList.end(); it++ ) {
		return_value = it->second->initialize();
		if ( return_value != RETURN_OK ) {
			object_id_t var = it->first;
			error << "Object " << std::hex <<  (int) var << " failed to initialize with code 0x" << return_value << std::dec << std::endl;
			error_count++;
		}
	}
	if (error_count > 0) {
		error << "ObjectManager::ObjectManager: Counted " << error_count << " failed initializations." << std::endl;
	}
	//Init was successful. Now check successful interconnections.
	error_count = 0;
	for (std::map<object_id_t, SystemObjectIF*>::iterator it = this->objectList.begin(); it != objectList.end(); it++ ) {
		return_value = it->second->checkObjectConnections();
		if ( return_value != RETURN_OK ) {
			error << "Object " << std::hex <<  (int) it->first << " connection check failed with code 0x" << return_value << std::dec << std::endl;
			error_count++;
		}
	}
	if (error_count > 0) {
		error << "ObjectManager::ObjectManager: Counted " << error_count << " failed connection checks." << std::endl;
	}

}

void ObjectManager::printList() {
	std::map<object_id_t, SystemObjectIF*>::iterator it;
	debug << "ObjectManager: Object List contains:" << std::endl;
	for (it = this->objectList.begin(); it != this->objectList.end(); it++) {
		debug << std::hex << it->first << " | " << it->second << std::endl;
	}
}
