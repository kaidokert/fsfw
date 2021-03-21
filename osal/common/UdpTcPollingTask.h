#ifndef FSFW_OSAL_WINDOWS_TCSOCKETPOLLINGTASK_H_
#define FSFW_OSAL_WINDOWS_TCSOCKETPOLLINGTASK_H_

#include "UdpTmTcBridge.h"
#include "../../objectmanager/SystemObject.h"
#include "../../tasks/ExecutableObjectIF.h"
#include "../../storagemanager/StorageManagerIF.h"

#include <vector>

/**
 * @brief 	This class can be used to implement the polling of a Unix socket,
 * 			using UDP for now.
 * @details
 * The task will be blocked while the specified number of bytes has not been
 * received, so TC reception is handled inside a separate task.
 * This class caches the IP address of the sender. It is assumed there
 * is only one sender for now.
 */
class UdpTcPollingTask:
        public TcpIpBase,
        public SystemObject,
		public ExecutableObjectIF {
	friend class TmTcWinUdpBridge;
public:
	static constexpr size_t DEFAULT_MAX_RECV_SIZE = 1500;
	//! 0.5  default milliseconds timeout for now.
	static constexpr timeval DEFAULT_TIMEOUT = {0, 500};

	UdpTcPollingTask(object_id_t objectId, object_id_t tmtcUnixUdpBridge,
			size_t maxRecvSize = 0, double timeoutSeconds = -1);
	virtual~ UdpTcPollingTask();

	/**
	 * Turn on optional timeout for UDP polling. In the default mode,
	 * the receive function will block until a packet is received.
	 * @param timeoutSeconds
	 */
	void setTimeout(double timeoutSeconds);

	virtual ReturnValue_t performOperation(uint8_t opCode) override;
	virtual ReturnValue_t initialize() override;
	virtual ReturnValue_t initializeAfterTaskCreation() override;

protected:
	StorageManagerIF* tcStore = nullptr;

private:
	//! TMTC bridge is cached.
	object_id_t tmtcBridgeId = objects::NO_OBJECT;
	UdpTmTcBridge* tmtcBridge = nullptr;
	MessageQueueId_t targetTcDestination = MessageQueueIF::NO_QUEUE;

	//! See: https://docs.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-recvfrom
	int receptionFlags = 0;

	std::vector<uint8_t> receptionBuffer;

	size_t frameSize = 0;
	timeval receptionTimeout;

	ReturnValue_t handleSuccessfullTcRead(size_t bytesRead);
};

#endif /* FRAMEWORK_OSAL_LINUX_TCSOCKETPOLLINGTASK_H_ */
