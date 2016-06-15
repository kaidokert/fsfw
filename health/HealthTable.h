#ifndef HEALTHTABLE_H_
#define HEALTHTABLE_H_

#include <framework/health/HealthTableIF.h>
#include <framework/objectmanager/SystemObject.h>
#include <framework/osal/OSAL.h>
#include <map>

typedef std::map<object_id_t, HasHealthIF::HealthState> HealthMap;

class HealthTable: public HealthTableIF, public SystemObject {
public:
	HealthTable(object_id_t objectid);
	virtual ~HealthTable();

	virtual ReturnValue_t registerObject(object_id_t object,
			HasHealthIF::HealthState initilialState = HasHealthIF::HEALTHY);

	virtual bool hasHealth(object_id_t object);
	virtual void setHealth(object_id_t object, HasHealthIF::HealthState newState);
	virtual HasHealthIF::HealthState getHealth(object_id_t);

	virtual uint32_t getPrintSize();
	virtual void printAll(uint8_t *pointer, uint32_t maxSize);

protected:
	MutexId_t* mutex;
	HealthMap healthMap;

	HealthMap::iterator mapIterator;

	virtual ReturnValue_t iterate(std::pair<object_id_t,HasHealthIF::HealthState> *value, bool reset = false);
};

#endif /* HEALTHTABLE_H_ */
