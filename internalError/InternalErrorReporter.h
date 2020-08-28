#ifndef INTERNALERRORREPORTER_H_
#define INTERNALERRORREPORTER_H_

#include "InternalErrorReporterIF.h"

#include "../tasks/ExecutableObjectIF.h"
#include "../objectmanager/SystemObject.h"
#include "../ipc/MutexIF.h"

class InternalErrorReporter: public SystemObject,
		public ExecutableObjectIF,
		public InternalErrorReporterIF {
public:
	InternalErrorReporter(object_id_t setObjectId, uint32_t queuePoolId,
			uint32_t tmPoolId, uint32_t storePoolId);
	virtual ~InternalErrorReporter();

	virtual ReturnValue_t performOperation(uint8_t opCode);

	virtual void queueMessageNotSent();

	virtual void lostTm();

	virtual void storeFull();
protected:
	MutexIF* mutex;

	uint32_t queuePoolId;
	uint32_t tmPoolId;
	uint32_t storePoolId;

	uint32_t queueHits;
	uint32_t tmHits;
	uint32_t storeHits;

	uint32_t getAndResetQueueHits();
	uint32_t getQueueHits();
	void incrementQueueHits();

	uint32_t getAndResetTmHits();
	uint32_t getTmHits();
	void incrementTmHits();

	uint32_t getAndResetStoreHits();
	uint32_t getStoreHits();
	void incrementStoreHits();

};

#endif /* INTERNALERRORREPORTER_H_ */
