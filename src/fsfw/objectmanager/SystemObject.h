#ifndef FSFW_OBJECTMANAGER_SYSTEMOBJECT_H_
#define FSFW_OBJECTMANAGER_SYSTEMOBJECT_H_

#include "SystemObjectIF.h"
#include "fsfw/events/Event.h"
#include "fsfw/events/EventReportingProxyIF.h"
#include "fsfw/timemanager/Clock.h"

/**
 * @brief	This class automates insertion into the ObjectManager and
 * 			management of the object id.
 * @details	This class is more a base class, which shall be inherited by any
 * 			class that is announced to ObjectManager. It automatically includes
 * 			itself (and therefore the inheriting class) in the object manager's
 * 			list.
 * @author	Ulrich Mohr
 * @ingroup system_objects
 */
class SystemObject : public SystemObjectIF {
 private:
  /**
   * @brief	This is the id the class instant got assigned.
   */
  const object_id_t objectId;
  const bool registered;

 public:
  /**
   * Helper function to send Event Messages to the Event Manager
   * @param event
   * @param parameter1
   * @param parameter2
   */
  virtual void triggerEvent(Event event, uint32_t parameter1 = 0, uint32_t parameter2 = 0);

  /**
   * @brief	The class's constructor.
   * @details	In the constructor, the object id is set and the class is
   * 			inserted in the object manager.
   * @param setObjectId	The id the object shall have.
   * @param doRegister	Determines if the object is registered in
   * 						the global object manager.
   */
  SystemObject(object_id_t setObjectId, bool doRegister = true);
  /**
   * @brief	On destruction, the object removes itself from the list.
   */
  virtual ~SystemObject();
  object_id_t getObjectId() const override;
  virtual ReturnValue_t initialize() override;
  virtual ReturnValue_t checkObjectConnections() override;

  virtual void forwardEvent(Event event, uint32_t parameter1 = 0,
                            uint32_t parameter2 = 0) const override;
};

#endif /* FSFW_OBJECTMANAGER_SYSTEMOBJECT_H_ */
