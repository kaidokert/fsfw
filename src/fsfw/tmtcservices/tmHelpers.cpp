#include "tmHelpers.h"

ReturnValue_t telemetry::storeAndSendTmPacket(TmStoreHelper &storeHelper, TmSendHelper &sendHelper,
                                              bool delOnFailure) {
  ReturnValue_t result = storeHelper.addPacketToStore();
  if (result != HasReturnvaluesIF::RETURN_OK) {
    return result;
  }
  result = sendHelper.sendPacket(storeHelper.getCurrentAddr());
  if (result != HasReturnvaluesIF::RETURN_OK) {
    if (delOnFailure) {
      storeHelper.deletePacket();
    }
  }
  return result;
}
