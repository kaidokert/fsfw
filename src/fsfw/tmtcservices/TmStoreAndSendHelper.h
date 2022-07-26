#ifndef FSFW_TMTCSERVICES_TMSTOREANDSENDHELPER_H
#define FSFW_TMTCSERVICES_TMSTOREANDSENDHELPER_H

#include "TmSendHelper.h"
#include "TmStoreHelper.h"
#include "tmHelpers.h"

/**
 * Wrapper class intended to help with PUS TM handling. This wrapper class also caches the current
 * packet count and the service ID
 */
class TmStoreAndSendWrapper {
 public:
  TmStoreAndSendWrapper(uint8_t defaultService, TmStoreHelper& storeHelper,
                        TmSendHelper& sendHelper);

  /**
   * Prepares a TM packet with the given parameters. It will also set the default service.
   * @param subservice
   * @return
   */
  ReturnValue_t prepareTmPacket(uint8_t subservice);
  /**
   * Prepares a TM packet with the given parameters. It will also set the default service.
   * @param subservice Number of subservice
   * @param sourceData Custom source data
   * @param sourceDataLen Lenght of data in the Packet
   */
  ReturnValue_t prepareTmPacket(uint8_t subservice, const uint8_t* sourceData,
                                size_t sourceDataLen);

  /**
   * Prepares a TM packet with the given parameters. It will also set the default service.
   * @param subservice Number of subservice
   * @param objectId ObjectId is placed before data
   * @param data Data to append to the packet
   * @param dataLen Length of Data
   */
  ReturnValue_t prepareTmPacket(uint8_t subservice,
                                telemetry::DataWithObjectIdPrefix& dataWithObjectId);

  /**
   * Prepares a TM packet with the given parameters. It will also set the default service.
   * @param subservice Number of subservice
   * @param content This is a pointer to the serialized packet
   * @param header Serialize IF header which will be placed before content
   */
  ReturnValue_t prepareTmPacket(uint8_t subservice, SerializeIF& sourceData);

  ReturnValue_t storeAndSendTmPacket();

  bool incrementSendCounter = true;
  TmStoreHelper& storeHelper;
  TmSendHelper& sendHelper;
  bool delOnFailure = true;
  uint8_t defaultService = 0;
  uint16_t sendCounter = 0;
};

#endif  // FSFW_TMTCSERVICES_TMSTOREANDSENDHELPER_H
