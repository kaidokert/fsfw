#ifndef FSFW_TMTCSERVICES_ACCEPTSTELEMETRYIF_H_
#define FSFW_TMTCSERVICES_ACCEPTSTELEMETRYIF_H_

#include "fsfw/ipc/MessageQueueSenderIF.h"
/**
 * @brief 	This interface is implemented by classes that are sinks for
 * 			Telemetry.
 * @details	Any object receiving telemetry shall implement this interface
 * 			and thus make the service id and the receiving message queue public.
 */
class AcceptsTelemetryIF {
 public:
  /**
   * @brief The virtual destructor as it is mandatory for C++ interfaces.
   */
  virtual ~AcceptsTelemetryIF() = default;

  [[nodiscard]] virtual const char* getName() const = 0;
  /**
   * @brief	This method returns the message queue id of the telemetry
   * 			receiving message queue.
   * @return	The telemetry reception message queue id.
   */
  virtual MessageQueueId_t getReportReceptionQueue(uint8_t virtualChannel) = 0;

  virtual MessageQueueId_t getReportReceptionQueue() { return getReportReceptionQueue(0); }
};

#endif /* FSFW_TMTCSERVICES_ACCEPTSTELEMETRYIF_H_ */
