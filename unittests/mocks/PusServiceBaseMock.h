#ifndef FSFW_TESTS_PUSSERVICEBASEMOCK_H
#define FSFW_TESTS_PUSSERVICEBASEMOCK_H

#include <queue>

#include "fsfw/tmtcservices/PusServiceBase.h"

class PsbMock : public PusServiceBase {
 public:
  explicit PsbMock(PsbParams params);
  unsigned int handleRequestCallCnt = 0;
  std::queue<uint8_t> subserviceQueue;
  unsigned int performServiceCallCnt = 0;

  static void setStaticPusDistributor(object_id_t pusDistributor);
  static object_id_t getStaticPusDistributor();
  static void setStaticTmDest(object_id_t tmDest);
  static object_id_t getStaticTmDest();

  PsbParams& getParams();

  std::pair<bool, ReturnValue_t> handleReqFailPair;
  std::pair<bool, ReturnValue_t> performServiceFailPair;
  ReturnValue_t handleRequest(uint8_t subservice) override;
  ReturnValue_t performService() override;

  void makeNextHandleReqCallFail(ReturnValue_t retval);
  bool getAndPopNextSubservice(uint8_t& subservice);
  void reset();
};

#endif  // FSFW_TESTS_PUSSERVICEBASEMOCK_H
