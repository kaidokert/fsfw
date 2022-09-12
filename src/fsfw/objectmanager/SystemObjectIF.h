#ifndef FSFW_OBJECTMANAGER_SYSTEMOBJECTIF_H_
#define FSFW_OBJECTMANAGER_SYSTEMOBJECTIF_H_

#include <cstdint>

#include "../events/EventReportingProxyIF.h"
#include "../returnvalues/returnvalue.h"
/**
 * @defgroup system_objects Software System Object Management
 * The classes to create System Objects and classes to manage these are
 * contained in this group. System Objects are software elements that can be
 * controlled externally. They all have a unique object identifier.
 */

/**
 * This is the typedef for object identifiers.
 * @ingroup system_objects
 */
using object_id_t = uint32_t;

/**
 * This interface allows a class to be included in the object manager
 * list.
 * It does not provide any method definitions, still it is required to
 * perform a type check with dynamic_cast.
 * @author  Bastian Baetz
 * @ingroup system_objects
 */
class SystemObjectIF : public EventReportingProxyIF {
 public:
  /**
   * This is a simple getter to return the object identifier.
   * @return	Returns the object id of this object.
   */
  virtual object_id_t getObjectId() const = 0;
  /**
   * The empty virtual destructor as required for C++ interfaces.
   */
  virtual ~SystemObjectIF() {}
  /**
   * @brief   Initializes the object.
   * There are initialization steps which can also be done in the constructor.
   * However, there is no clean way to get a returnvalue from a constructor.
   * Furthermore some components require other system object to be created
   * which might not have been built yet.
   * Therefore, a two-step initialization resolves this problem and prevents
   * circular dependencies of not-fully initialized objects on start up.
   * @return	- @c returnvalue::OK in case the initialization was successful
   * 		- @c returnvalue::FAILED otherwise
   */
  virtual ReturnValue_t initialize() = 0;
  /**
   * @brief   Checks if all object-object interconnections are satisfying
   *          for operation.
   * Some objects need certain other objects (or a certain number), to be
   * registered as children. These checks can be done in this method.
   * @return	- @c returnvalue::OK in case the check was successful
   * 			- @c any other code otherwise
   */
  virtual ReturnValue_t checkObjectConnections() = 0;
};

#endif /* #ifndef FSFW_OBJECTMANAGER_SYSTEMOBJECTIF_H_ */
