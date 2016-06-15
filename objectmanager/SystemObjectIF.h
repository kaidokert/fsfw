/**
 * @file	SystemObjectIF.h
 * @brief	This file contains the definition of the SystemObjectIF interface.
 * @date	18.09.2012
 * @author	Bastian Baetz
 */

#ifndef SYSTEMOBJECTIF_H_
#define SYSTEMOBJECTIF_H_

#include <framework/events/EventReportingProxyIF.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <stdint.h>
/**
 * \defgroup system_objects Software System Object Management
 * The classes to create System Objects and classes to manage these are contained in this group.
 * System Objects are software elements that can be controlled externally. They all have a unique
 * object identifier.
 */

/**
 * This is the typedef for object identifiers.
 * \ingroup system_objects
 */
typedef uint32_t object_id_t;

/**
 * This interface allows a class to be included in the object manager
 * list.
 * It does not provide any method definitions, still it is required to
 * perform a type check with dynamic_cast.
 * \ingroup system_objects
 */
class SystemObjectIF : public EventReportingProxyIF {
public:
	/**
	 * This is a simple getter to return the object identifier.
	 * @return	Returns the object id of this object.
	 */
	virtual const object_id_t getObjectId() const = 0;
	/**
	 * The empty virtual destructor as required for C++ interfaces.
	 */
	virtual ~SystemObjectIF() {
	}
	/**
	 * Initializes all inter-object dependencies.
	 * This is necessary to avoid circular dependencies of not-fully
	 * initialized objects on start up.
	 * @return	- \c RETURN_OK in case the initialization was successful
	 * 			- \c RETURN_FAILED otherwise
	 */
	virtual ReturnValue_t initialize() = 0;
	/**
	 * Checks, if all object-object interconnections are satisfying for operation.
	 * Some objects need certain other objects (or a certain number), to be registered as children.
	 * These checks can be done in this method.
	 * @return	- \c RETURN_OK in case the check was successful
	 * 			- \c any other code otherwise
	 */
	virtual ReturnValue_t checkObjectConnections() = 0;
};

#endif /* SYSTEMOBJECTIF_H_ */
