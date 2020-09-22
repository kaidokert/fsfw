#ifndef FSFW_OBJECTMANAGER_OBJECTMANAGER_H_
#define FSFW_OBJECTMANAGER_OBJECTMANAGER_H_

#include "ObjectManagerIF.h"
#include "SystemObjectIF.h"
#include <map>

/**
 * @brief	This class implements a global object manager.
 * @details	This manager handles a list of available objects with system-wide
 * 			relevance, such as device handlers, and TM/TC services. Objects can
 * 			be inserted, removed and retrieved from the list. In addition, the
 * 			class holds a so-called factory, that creates and inserts new
 * 			objects if they are not already in the list. This feature automates
 * 			most of the system initialization.
 * 			As the system is static after initialization, no new objects are
 * 			created or inserted into the list after startup.
 * @ingroup system_objects
 * @author	Bastian Baetz
 */
class ObjectManager : public ObjectManagerIF {
private:
	//comparison?
	/**
	 * @brief	This is the map of all initialized objects in the manager.
	 * @details	Objects in the List must inherit the SystemObjectIF.
	 */
	std::map<object_id_t, SystemObjectIF*> objectList;
protected:
	SystemObjectIF* getSystemObject( object_id_t id );
	/**
	 * @brief	This attribute is initialized with the factory function
	 * 			that creates new objects.
	 * @details	The function is called if an object was requested with
	 * 			getSystemObject, but not found in objectList.
	 * @param 	The id of the object to be created.
	 * @return	Returns a pointer to the newly created object or NULL.
	 */
	void (*produceObjects)();
public:
	/**
	 * @brief	Apart from setting the producer function, nothing special
	 * 			happens in the constructor.
	 * @param setProducer	A pointer to a factory function.
	 */
	ObjectManager( void (*produce)() );
	ObjectManager();
	/**
	 *	@brief	In the class's destructor, all objects in the list are deleted.
	 */
	// SHOULDDO: If, for some reason, deleting an ObjectManager instance is
	// required, check if this works.
	virtual ~ObjectManager( void );
	ReturnValue_t insert( object_id_t id, SystemObjectIF* object );
	ReturnValue_t remove( object_id_t id );
	void initialize();
	void printList();
};



#endif /* FSFW_OBJECTMANAGER_OBJECTMANAGER_H_ */
