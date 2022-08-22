#ifndef FSFW_OSAL_COMMON_TCP_TMTC_SERVER_H_
#define FSFW_OSAL_COMMON_TCP_TMTC_SERVER_H_

#include "TcpIpBase.h"
#include "fsfw/container/SimpleRingBuffer.h"
#include "fsfw/ipc/MessageQueueIF.h"
#include "fsfw/ipc/messageQueueDefinitions.h"
#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/objectmanager/frameworkObjects.h"
#include "fsfw/osal/common/tcpipHelpers.h"
#include "fsfw/platform.h"
#include "fsfw/storagemanager/StorageManagerIF.h"
#include "fsfw/tasks/ExecutableObjectIF.h"

#ifdef PLATFORM_UNIX
#include <sys/socket.h>
#endif

#include <string>
#include <utility>
#include <vector>

class TcpTmTcBridge;
class SpacePacketParser;

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
class TcpTmTcServer : public SystemObject, public TcpIpBase, public ExecutableObjectIF {
 public:
  enum class ReceptionModes { SPACE_PACKETS };

  struct TcpConfig {
   public:
    explicit TcpConfig(std::string tcpPort) : tcpPort(std::move(tcpPort)) {}

    /**
     * Passed to the recv call
     */
    int tcpFlags = 0;
    int tcpBacklog = 3;

    /**
     * If no telecommands packets are being received and no telemetry is being sent,
     * the TCP server will delay periodically by this amount to decrease the CPU load
     */
    uint32_t tcpLoopDelay = DEFAULT_LOOP_DELAY_MS;
    /**
     * Passed to the send call
     */
    int tcpTmFlags = 0;

    const std::string tcpPort;
  };

  static const std::string DEFAULT_SERVER_PORT;

  static constexpr size_t ETHERNET_MTU_SIZE = 1500;
  static constexpr size_t RING_BUFFER_SIZE = ETHERNET_MTU_SIZE * 3;
  static constexpr uint32_t DEFAULT_LOOP_DELAY_MS = 200;

  /**
   * TCP Server Constructor
   * @param objectId              Object ID of the TCP Server
   * @param tmtcTcpBridge         Object ID of the TCP TMTC Bridge object
   * @param receptionBufferSize   This will be the size of the reception buffer. Default buffer
   *                              size will be the Ethernet MTU size
   * @param customTcpServerPort   The user can specify another port than the default (7301) here.
   */
  TcpTmTcServer(object_id_t objectId, object_id_t tmtcTcpBridge,
                size_t receptionBufferSize = RING_BUFFER_SIZE,
                size_t ringBufferSize = RING_BUFFER_SIZE,
                std::string customTcpServerPort = DEFAULT_SERVER_PORT,
                ReceptionModes receptionMode = ReceptionModes::SPACE_PACKETS);
  ~TcpTmTcServer() override;

  void enableWiretapping(bool enable);

  /**
   * Get a handle to the TCP configuration struct, which can be used to configure TCP
   * properties
   * @return
   */
  TcpConfig& getTcpConfigStruct();
  void setSpacePacketParsingOptions(std::vector<uint16_t> validPacketIds);

  ReturnValue_t initialize() override;
  [[noreturn]] ReturnValue_t performOperation(uint8_t opCode) override;
  ReturnValue_t initializeAfterTaskCreation() override;

  [[nodiscard]] const std::string& getTcpPort() const;

 protected:
  StorageManagerIF* tcStore = nullptr;
  StorageManagerIF* tmStore = nullptr;

 private:
  static constexpr ReturnValue_t CONN_BROKEN = returnvalue::makeCode(1, 0);
  //! TMTC bridge is cached.
  object_id_t tmtcBridgeId = objects::NO_OBJECT;
  TcpTmTcBridge* tmtcBridge = nullptr;
  bool wiretappingEnabled = false;

  ReceptionModes receptionMode;
  TcpConfig tcpConfig;
  struct sockaddr tcpAddress = {};
  socket_t listenerTcpSocket = 0;

  MessageQueueId_t targetTcDestination = MessageQueueIF::NO_QUEUE;

  std::vector<uint8_t> receptionBuffer;
  SimpleRingBuffer ringBuffer;
  std::vector<uint16_t> validPacketIds;
  SpacePacketParser* spacePacketParser = nullptr;
  uint8_t lastRingBufferSize = 0;

  virtual void handleServerOperation(socket_t& connSocket);
  ReturnValue_t handleTcReception(uint8_t* spacePacket, size_t packetSize);
  ReturnValue_t handleTmSending(socket_t connSocket, bool& tmSent);
  ReturnValue_t handleTcRingBufferData(size_t availableReadData);
  void handleSocketError(ConstStorageAccessor& accessor);
#if defined PLATFORM_WIN
  void setSocketNonBlocking(socket_t& connSocket);
#endif
};

#endif /* FSFW_OSAL_COMMON_TCP_TMTC_SERVER_H_ */
