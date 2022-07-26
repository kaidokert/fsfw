#ifndef FSFW_TMTCSERVICES_TMSTOREANDSENDHELPER_H
#define FSFW_TMTCSERVICES_TMSTOREANDSENDHELPER_H

#include "TmSendHelper.h"
#include "TmStoreHelper.h"

class StoreAndSendWrapper {
 public:
  StoreAndSendWrapper(TmStoreHelper& storeHelper, TmSendHelper& sendHelper)
      : storeHelper(storeHelper), sendHelper(sendHelper) {}
  ReturnValue_t storeAndSendTmPacket(TmStoreHelper& storeHelper, TmSendHelper& sendHelper,
                                     bool delOnFailure = true);
  TmStoreHelper& storeHelper;
  TmSendHelper& sendHelper;
  bool delOnFailure = true;
};

#endif  // FSFW_TMTCSERVICES_TMSTOREANDSENDHELPER_H
