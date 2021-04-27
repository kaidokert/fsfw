#ifndef FSFW_UNITTEST_TESTS_MOCKS_HKRECEIVERMOCK_H_
#define FSFW_UNITTEST_TESTS_MOCKS_HKRECEIVERMOCK_H_

#include <fsfw/housekeeping/AcceptsHkPacketsIF.h>
#include <fsfw/objectmanager/SystemObject.h>

class HkReceiverMock: public SystemObject, public AcceptsHkPacketsIF {
public:
	HkReceiverMock(object_id_t objectId): SystemObject(objectId) {

	}

	MessageQueueId_t getHkQueue() const {
		return MessageQueueIF::NO_QUEUE;
	}
};



#endif /* FSFW_UNITTEST_TESTS_MOCKS_HKRECEIVERMOCK_H_ */
