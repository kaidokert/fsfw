#ifndef FSFW_PUS_SERVICE5EVENTREPORTING_H_
#define FSFW_PUS_SERVICE5EVENTREPORTING_H_

#include "fsfw/events/EventMessage.h"
#include "fsfw/tmtcservices/PusServiceBase.h"
#include "fsfw/tmtcservices/TmStoreAndSendHelper.h"

/**
 * @brief Report on-board events like information or errors
 * @details
 * Full Documentation: ECSS-E70-41A p.79
 * Implements the PusServiceBase template class.
 * Documentation: Dissertation Baetz p.135,136
 *
 * This service provides for the reporting to the service user of information of
 * operational significance.
 *   1. reporting of failures or anomalies detected on-board;
 *   2. reporting of autonomous on-board actions;
 *   3. reporting of normal progress of operations and activities, e.g.
 *   detection of events which are not anomalous (such as payload events),
 *   reaching of predefined steps in an operation. Some reports can combine
 *   more than one of these events.
 *
 * Minimum capabilities of this service:
 *
 * - TM[5,1]: Normal/Progress Report
 * - TM[5,2]: Error/Anomaly Report - Low Severity
 * - TM[5,3]: Error/Anomaly Report - Medium Severity
 * - TM[5,4]: Error/Anomaly Report - High Severity
 *
 * Events can be translated by using translator files located in
 * /config/objects/ and /config/events/. Description to events can be added by
 * adding a comment behind the event definition with [//!<] as leading string
 *
 * Additional capabilities of this service:
 *
 *  - TC[5,5]: Enable Event Report Generation (Req. 6)
 *  - TC[5,6]: Disable Event Report Generation (Req. 5)
 * @author 	R. Mueller
 * @ingroup pus_services
 */
class Service5EventReporting : public PusServiceBase {
 public:
  Service5EventReporting(PsbParams params, size_t maxNumberReportsPerCycle = 10,
                         uint32_t messageQueueDepth = 10);
  ~Service5EventReporting() override;

  /***
   * Check for events and generate event reports if required.
   * @return
   */
  ReturnValue_t performService() override;

  /***
   * Turn event generation on or off.
   * @return
   */
  ReturnValue_t handleRequest(uint8_t subservice) override;

  /**
   * The default PusServiceBase initialize has been overridden but is still
   * executed. Registers this service as a listener for events at the
   * EventManager.
   * @return
   */
  ReturnValue_t initialize() override;

  enum Subservice : uint8_t {
    NORMAL_REPORT = 1,        //!< [EXPORT] : [REPLY] Generate normal report
    ERROR_LOW_SEVERITY = 2,   //!< [EXPORT] : [REPLY] Generate error report with low severity
    ERROR_MED_SEVERITY = 3,   //!< [EXPORT] : [REPLY] Generate error report with medium severity
    ERROR_HIGH_SEVERITY = 4,  //!< [EXPORT] : [REPLY] Generate error report with high severity
    ENABLE = 5,               //!< [EXPORT] : [COMMAND] Enable report generation
    DISABLE = 6               //!< [EXPORT] : [COMMAND] Disable report generation
  };

 private:
  MessageQueueIF* eventQueue = nullptr;
  bool enableEventReport = true;
  TmSendHelper sendHelper;
  TmStoreHelper storeHelper;
  TmStoreAndSendWrapper tmHelper;
  const uint8_t maxNumberReportsPerCycle;

  ReturnValue_t generateEventReport(EventMessage message);
};

#endif /* FSFW_PUS_SERVICE5EVENTREPORTING_H_ */
