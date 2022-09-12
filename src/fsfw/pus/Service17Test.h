#ifndef FSFW_PUS_SERVICE17TEST_H_
#define FSFW_PUS_SERVICE17TEST_H_

#include "fsfw/objectmanager/SystemObject.h"
#include "fsfw/tmtcservices/PusServiceBase.h"
#include "fsfw/tmtcservices/TmStoreAndSendHelper.h"
#include "fsfw/tmtcservices/TmStoreHelper.h"

/**
 * @brief Test Service
 * Full Documentation: ECSS-E70-41A p.167
 *
 * The test service provides the capability to activate test functions
 * implemented on-board and to report the results of such tests.
 * Service capability:
 *   - TC[17,1]: Perform connection test
 *   - TM[17,2]: Send Connection Test Report
 *   - TC[17,128]: Perform connection test and trigger event
 *
 * @ingroup pus_services
 */
class Service17Test : public PusServiceBase {
 public:
  // Custom events which can be triggered
  static constexpr uint8_t SUBSYSTEM_ID = SUBSYSTEM_ID::PUS_SERVICE_17;
  static constexpr Event TEST = MAKE_EVENT(0, severity::INFO);

  enum Subservice : uint8_t {
    //! [EXPORT] : [COMMAND] Perform connection test
    CONNECTION_TEST = 1,
    //! [EXPORT] : [REPLY] Connection test reply
    CONNECTION_TEST_REPORT = 2,
    //!  [EXPORT] : [COMMAND] Trigger test reply and test event
    EVENT_TRIGGER_TEST = 128,
  };

  explicit Service17Test(PsbParams params);

  void setCustomTmStore(StorageManagerIF& tmStore);

  ~Service17Test() override;
  ReturnValue_t handleRequest(uint8_t subservice) override;
  ReturnValue_t performService() override;
  ReturnValue_t initialize() override;

 protected:
  TmStoreHelper storeHelper;
  TmSendHelper sendHelper;
  TmStoreAndSendWrapper tmHelper;
};

#endif /* FSFW_PUS_SERVICE17TEST_H_ */
