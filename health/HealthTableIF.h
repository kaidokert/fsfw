#ifndef FSFW_HEALTH_HEALTHTABLEIF_H_
#define FSFW_HEALTH_HEALTHTABLEIF_H_

#include "ManagesHealthIF.h"
#include "../objectmanager/ObjectManagerIF.h"
#include "../returnvalues/HasReturnvaluesIF.h"

class HealthTableIF: public ManagesHealthIF {
public:
	virtual ~HealthTableIF() {}

	virtual ReturnValue_t registerObject(object_id_t object,
			HasHealthIF::HealthState initilialState = HasHealthIF::HEALTHY) = 0;

	virtual size_t getPrintSize() = 0;
	virtual void printAll(uint8_t *pointer, size_t maxSize) = 0;

protected:
	virtual ReturnValue_t iterate(
	        std::pair<object_id_t,HasHealthIF::HealthState> *value,
	        bool reset = false) = 0;
};

#endif /* FRAMEWORK_HEALTH_HEALTHTABLEIF_H_ */
