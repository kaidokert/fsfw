#ifndef FSFW_OBJECTMANAGER_OBJECTMANAGERIF_H_
#define FSFW_OBJECTMANAGER_OBJECTMANAGERIF_H_

#include "frameworkObjects.h"
#include "SystemObjectIF.h"
#include "../returnvalues/HasReturnvaluesIF.h"
#include "../serviceinterface/ServiceInterfaceStream.h"

/**
 * @brief	This class provides an interface to the global object manager.
 * @details	This manager handles a list of available objects with system-wide
 * 			relevance, such as device handlers, and TM/TC services. They can be
 * 			inserted, removed and retrieved from the list. On getting the
 * 			object, the call checks if the object implements the requested
 * 			interface.
 * @author	Bastian Baetz
 * @ingroup system_objects
 */
class ObjectManagerIF : public HasReturnvaluesIF {
public:
	static constexpr uint8_t INTERFACE_ID = CLASS_ID::OBJECT_MANAGER_IF;
	static constexpr ReturnValue_t INSERTION_FAILED = MAKE_RETURN_CODE( 1 );
	static constexpr ReturnValue_t NOT_FOUND = MAKE_RETURN_CODE( 2 );
	static constexpr ReturnValue_t CHILD_INIT_FAILED = MAKE_RETURN_CODE( 3 ); //!< Can be used if the initialization of a SystemObject failed.
	static constexpr ReturnValue_t INTERNAL_ERR_REPORTER_UNINIT = MAKE_RETURN_CODE( 4 );

protected:
	/**
	 * @brief	This method is used to hide the template-based get call from
	 * 			a specific implementation.
	 * @details	So, an implementation only has to implement this call.
	 * @param id	The object id of the requested object.
	 * @return	The method returns a pointer to an object implementing (at
	 * 			least) the SystemObjectIF, or NULL.
	 */
	virtual  SystemObjectIF* getSystemObject( object_id_t id ) = 0;
public:
	/**
	 * @brief	This is the empty virtual destructor as requested by C++ interfaces.
	 */
	virtual ~ObjectManagerIF( void ) {};
	/**
	 * @brief	With this call, new objects are inserted to the list.
	 * @details	The implementation shall return an error code in case the
	 * 			object can't be added (e.g. the id is already in use).
	 * @param id		The new id to be added to the list.
	 * @param object	A pointer to the object to be added.
	 * @return	@li INSERTION_FAILED in case the object could not be inserted.
	 * 			@li RETURN_OK in case the object was successfully inserted
	 */
	virtual ReturnValue_t insert( object_id_t id, SystemObjectIF* object ) = 0;
	/**
	 * @brief	With the get call, interfaces of an object can be retrieved in
	 * 			a type-safe manner.
	 * @details With the template-based call, the object list is searched with the
	 * 			getSystemObject method and afterwards it is checked, if the object
	 * 			implements the requested interface (with a dynamic_cast).
	 * @param id	The object id of the requested object.
	 * @return	The method returns a pointer to an object implementing the
	 * 			requested interface, or NULL.
	 */
	template <typename T> T* get( object_id_t id );
	/**
	 * @brief	With this call, an object is removed from the list.
	 * @param id	The object id of the object to be removed.
	 * @return	\li NOT_FOUND in case the object was not found
	 * 			\li RETURN_OK in case the object was successfully removed
	 */
	virtual ReturnValue_t remove( object_id_t id ) = 0;
	virtual void initialize() = 0;
	/**
	 * @brief	This is a debug function, that prints the current content of the
	 * 			object list.
	 */
	virtual void printList() = 0;
};


/**
 * @brief	This is the forward declaration of the global objectManager instance.
 */
// SHOULDDO: maybe put this in the glob namespace to explicitely mark it global?
extern ObjectManagerIF *objectManager;

/*Documentation can be found in the class method declaration above.*/
template <typename T>
T* ObjectManagerIF::get( object_id_t id ) {
	if(objectManager == nullptr) {
		sif::error << "ObjectManagerIF: Global object manager has not "
				"been initialized yet!" << std::endl;
	}
	SystemObjectIF* temp = this->getSystemObject(id);
	return dynamic_cast<T*>(temp);
}

#endif /* OBJECTMANAGERIF_H_ */
