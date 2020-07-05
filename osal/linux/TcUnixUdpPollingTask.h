#ifndef FRAMEWORK_OSAL_LINUX_TCSOCKETPOLLINGTASK_H_
#define FRAMEWORK_OSAL_LINUX_TCSOCKETPOLLINGTASK_H_
#include <framework/objectmanager/SystemObject.h>
#include <framework/tasks/ExecutableObjectIF.h>

/**
 * @brief 	This class can be used to implement the polling of a Unix socket,
 * 			using UDP for now.
 * @details
 * The task will be blocked while the specified number of bytes has not been
 * received, so TC reception is handled inside a separate task.
 * This class caches the IP address of the sender. It is assumed there
 * is only one sender for now.
 */
class TcSocketPollingTask: public SystemObject,
		public ExecutableObjectIF {
public:
	TcSocketPollingTask(object_id_t objectId, object_id_t tmtcUnixUdpBridge);
	virtual~ TcSocketPollingTask();

	virtual ReturnValue_t performOperation(uint8_t opCode) override;
	virtual ReturnValue_t initialize() override;
private:
};



#endif /* FRAMEWORK_OSAL_LINUX_TCSOCKETPOLLINGTASK_H_ */
