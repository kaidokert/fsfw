#ifndef HEALTHTABLEIF_H_
#define HEALTHTABLEIF_H_

#include <framework/health/ManagesHealthIF.h>
#include <framework/objectmanager/ObjectManagerIF.h>
#include <framework/returnvalues/HasReturnvaluesIF.h>
#include <map>


class HealthTableIF: public ManagesHealthIF {
	friend class HealthCommandingService;
public:
	virtual ~HealthTableIF() {
	}

	virtual ReturnValue_t registerObject(object_id_t object,
			HasHealthIF::HealthState initilialState = HasHealthIF::HEALTHY) = 0;

	virtual uint32_t getPrintSize() = 0;
	virtual void printAll(uint8_t *pointer, size_t maxSize) = 0;

protected:
	virtual ReturnValue_t iterate(std::pair<object_id_t,HasHealthIF::HealthState> *value, bool reset = false) = 0;
};

#endif /* HEALTHTABLEIF_H_ */
