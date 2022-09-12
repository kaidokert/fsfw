#ifndef INTERNALERRORREPORTERIF_H_
#define INTERNALERRORREPORTERIF_H_

/**
 * @brief   Interface which is used to report internal errors like full message queues or stores.
 * @details
 * This interface smust be used for the InteralErrorReporter object.
 * It should be used to indicate that there was a Problem with Queues or Stores.
 *
 * It can be used to report missing Telemetry which could not be sent due to a internal problem.
 *
 */
class InternalErrorReporterIF {
 public:
  virtual ~InternalErrorReporterIF() = default;
  /**
   * @brief Function to be called if a message queue could not be sent.
   * @details OSAL Implementations should call this function to indicate that
   * a message was lost.
   *
   *  Implementations are required to be Thread safe
   */
  virtual void queueMessageNotSent() = 0;
  /**
   * @brief Function to be called if Telemetry could not be sent
   * @details Implementations must be Thread safe
   */
  virtual void lostTm() = 0;
  /**
   * @brief Function to be called if a onboard storage is full
   * @details Implementations must be Thread safe
   */
  virtual void storeFull() = 0;
};

#endif /* INTERNALERRORREPORTERIF_H_ */
