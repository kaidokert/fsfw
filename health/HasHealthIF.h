#ifndef FSFW_HEALTH_HASHEALTHIF_H_
#define FSFW_HEALTH_HASHEALTHIF_H_

#include "../events/Event.h"
#include "../returnvalues/HasReturnvaluesIF.h"
#include "../ipc/MessageQueueSenderIF.h"

class HasHealthIF {
public:

	enum HealthState: uint8_t {
		HEALTHY = 1,
		FAULTY = 0,
		EXTERNAL_CONTROL = 2,
		NEEDS_RECOVERY = 3,
		PERMANENT_FAULTY = 4
	};

	static const uint8_t INTERFACE_ID = CLASS_ID::HAS_HEALTH_IF;
	static const ReturnValue_t OBJECT_NOT_HEALTHY = MAKE_RETURN_CODE(1);
	static const ReturnValue_t INVALID_HEALTH_STATE = MAKE_RETURN_CODE(2);

	static const uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::SYSTEM_MANAGER_1;
	static const Event HEALTH_INFO = MAKE_EVENT(6, SEVERITY::INFO);
	static const Event CHILD_CHANGED_HEALTH = MAKE_EVENT(7, SEVERITY::INFO);
	static const Event CHILD_PROBLEMS = MAKE_EVENT(8, SEVERITY::LOW);
	static const Event OVERWRITING_HEALTH = MAKE_EVENT(9, SEVERITY::LOW); //!< Assembly overwrites health information of children to keep satellite alive.
	static const Event TRYING_RECOVERY = MAKE_EVENT(10, SEVERITY::MEDIUM); //!< Someone starts a recovery of a component (typically power-cycle). No parameters.
	static const Event RECOVERY_STEP = MAKE_EVENT(11, SEVERITY::MEDIUM); //!< Recovery is ongoing. Comes twice during recovery. P1: 0 for the first, 1 for the second event. P2: 0
	static const Event RECOVERY_DONE = MAKE_EVENT(12, SEVERITY::MEDIUM); //!< Recovery was completed. Not necessarily successful. No parameters.

	virtual ~HasHealthIF() {
	}

	virtual MessageQueueId_t getCommandQueue() const = 0;

	/**
	 * @brief   Set the Health State
	 * The parent will be informed, if the Health changes
	 * @param health
	 */
	virtual ReturnValue_t setHealth(HealthState health) = 0;

	/**
	 * @brief   Get Health State
	 * @return  Health State of the object
	 */
	virtual HasHealthIF::HealthState getHealth() = 0;
};

#endif /* FSFW_HEALTH_HASHEALTHIF_H_ */
