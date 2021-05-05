#ifndef FSFW_OSAL_WINDOWS_TCWINTCPSERVER_H_
#define FSFW_OSAL_WINDOWS_TCWINTCPSERVER_H_

#include "TcpIpBase.h"
#include "../../ipc/messageQueueDefinitions.h"
#include "../../ipc/MessageQueueIF.h"
#include "../../objectmanager/frameworkObjects.h"
#include "../../objectmanager/SystemObject.h"
#include "../../storagemanager/StorageManagerIF.h"
#include "../../tasks/ExecutableObjectIF.h"

#ifdef __unix__
#include <sys/socket.h>
#endif

#include <string>
#include <vector>

class TcpTmTcBridge;
//class SharedRingBuffer;


/**
 * @brief   Windows TCP server used to receive telecommands on a Windows Host
 * @details
 * Based on: https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code
 */
class TcpTmTcServer:
        public SystemObject,
        public TcpIpBase,
        public ExecutableObjectIF {
public:
    /* The ports chosen here should not be used by any other process. */
    static const std::string DEFAULT_TCP_SERVER_PORT;
    static const std::string DEFAULT_TCP_CLIENT_PORT;
    static constexpr  size_t ETHERNET_MTU_SIZE = 1500;

    TcpTmTcServer(object_id_t objectId, object_id_t tmtcTcpBridge /*, SharedRingBuffer* tcpRingBuffer*/,
            size_t receptionBufferSize = ETHERNET_MTU_SIZE,
            std::string customTcpServerPort = "");
    virtual~ TcpTmTcServer();

    ReturnValue_t initialize() override;
    ReturnValue_t performOperation(uint8_t opCode) override;
    ReturnValue_t initializeAfterTaskCreation() override;

protected:
    StorageManagerIF* tcStore = nullptr;

private:
    //! TMTC bridge is cached.
    object_id_t tmtcBridgeId = objects::NO_OBJECT;
    TcpTmTcBridge* tmtcBridge = nullptr;

    std::string tcpPort;
    int tcpFlags = 0;
    socket_t listenerTcpSocket = 0;
    struct sockaddr tcpAddress;
    MessageQueueId_t targetTcDestination = MessageQueueIF::NO_QUEUE;
    int tcpAddrLen = sizeof(tcpAddress);
    int currentBacklog = 3;

    std::vector<uint8_t> receptionBuffer;
    //SharedRingBuffer* tcpRingBuffer;
    int tcpSockOpt = 0;

    void handleServerOperation(socket_t connSocket);
    ReturnValue_t handleTcReception(size_t bytesRecvd);
};

#endif /* FSFW_OSAL_WINDOWS_TCWINTCPSERVER_H_ */
