#ifndef FSFW_OSAL_COMMON_TCP_TMTC_SERVER_H_
#define FSFW_OSAL_COMMON_TCP_TMTC_SERVER_H_

#include "TcpIpBase.h"

#include "../../platform.h"
#include "../../ipc/messageQueueDefinitions.h"
#include "../../ipc/MessageQueueIF.h"
#include "../../objectmanager/frameworkObjects.h"
#include "../../objectmanager/SystemObject.h"
#include "../../storagemanager/StorageManagerIF.h"
#include "../../tasks/ExecutableObjectIF.h"

#ifdef PLATFORM_UNIX
#include <sys/socket.h>
#endif

#include <string>
#include <vector>

class TcpTmTcBridge;

/**
 * @brief   TCP server implementation
 * @details
 * This server will run for the whole program lifetime and will take care of serving client
 * requests on a specified TCP server port. This server was written in a generic way and
 * can be used on Unix and on Windows systems.
 *
 * If a connection is accepted, the server will read all telecommands sent by a client and then
 * send all telemetry currently found in the TMTC bridge FIFO.
 *
 * Reading telemetry without sending telecommands is possible by connecting, shutting down the
 * send operation immediately and then reading the telemetry. It is therefore recommended to
 * connect to the server regularly, even if no telecommands need to be sent.
 *
 * The server will listen to a specific port on all addresses (0.0.0.0).
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

    /**
     * TCP Server Constructor
     * @param objectId              Object ID of the TCP Server
     * @param tmtcTcpBridge         Object ID of the TCP TMTC Bridge object
     * @param receptionBufferSize   This will be the size of the reception buffer. Default buffer
     *                              size will be the Ethernet MTU size
     * @param customTcpServerPort   The user can specify another port than the default (7301) here.
     */
    TcpTmTcServer(object_id_t objectId, object_id_t tmtcTcpBridge,
            size_t receptionBufferSize = ETHERNET_MTU_SIZE + 1,
            std::string customTcpServerPort = "");
    virtual~ TcpTmTcServer();

    void setTcpBacklog(uint8_t tcpBacklog);

    ReturnValue_t initialize() override;
    ReturnValue_t performOperation(uint8_t opCode) override;
    ReturnValue_t initializeAfterTaskCreation() override;

protected:
    StorageManagerIF* tcStore = nullptr;
    StorageManagerIF* tmStore = nullptr;
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
    int tcpBacklog = 3;

    std::vector<uint8_t> receptionBuffer;
    int tcpSockOpt = 0;
    int tcpTmFlags = 0;

    void handleServerOperation(socket_t connSocket);
    ReturnValue_t handleTcReception(size_t bytesRecvd);
    ReturnValue_t handleTmSending(socket_t connSocket);
};

#endif /* FSFW_OSAL_COMMON_TCP_TMTC_SERVER_H_ */
