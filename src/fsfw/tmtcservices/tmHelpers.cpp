#include "tmHelpers.h"

ReturnValue_t telemetry::storeAndSendTmPacket(TmStoreHelper &storeHelper,
                                              TmSendHelper &sendHelper) {
  storeHelper.addPacketToStore();
  ReturnValue_t result = sendHelper.sendPacket(storeHelper.getCurrentAddr());
  if (result != HasReturnvaluesIF::RETURN_OK) {
    storeHelper.deletePacket();
  }
  return result;
}
