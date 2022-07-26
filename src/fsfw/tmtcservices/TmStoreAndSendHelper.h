#ifndef FSFW_TMTCSERVICES_TMSTOREANDSENDHELPER_H
#define FSFW_TMTCSERVICES_TMSTOREANDSENDHELPER_H

#include "TmSendHelper.h"
#include "TmStoreHelper.h"

/**
 * Wrapper class intended to help with PUS TM handling. This wrapper class also caches the current
 * packet count and the service ID
 */
class TmStoreAndSendWrapper {
 public:
  TmStoreAndSendWrapper(uint8_t defaultService, TmStoreHelper& storeHelper,
                        TmSendHelper& sendHelper);
  ReturnValue_t storeAndSendTmPacket();

  ReturnValue_t sendTmPacket(uint8_t subservice);
  /**
   * @brief   Send TM data from pointer to data.
   *          If a header is supplied it is added before data
   * @param subservice Number of subservice
   * @param sourceData Custom source data
   * @param sourceDataLen Lenght of data in the Packet
   */
  ReturnValue_t sendTmPacket(uint8_t subservice, const uint8_t* sourceData, size_t sourceDataLen);

  /**
   * @brief   To send TM packets of objects that still need to be serialized
   *          and consist of an object ID with appended data.
   * @param subservice Number of subservice
   * @param objectId ObjectId is placed before data
   * @param data Data to append to the packet
   * @param dataLen Length of Data
   */
  ReturnValue_t sendTmPacket(uint8_t subservice, object_id_t objectId, const uint8_t* data,
                             size_t dataLen);

  /**
   * @brief   To send packets which are contained inside a class implementing
   *          SerializeIF.
   * @param subservice Number of subservice
   * @param content This is a pointer to the serialized packet
   * @param header Serialize IF header which will be placed before content
   */
  ReturnValue_t sendTmPacket(uint8_t subservice, SerializeIF& sourceData);

  bool incrementSendCounter = true;
  TmStoreHelper& storeHelper;
  TmSendHelper& sendHelper;
  bool delOnFailure = true;
  uint8_t defaultService = 0;
  uint16_t sendCounter = 0;
};

#endif  // FSFW_TMTCSERVICES_TMSTOREANDSENDHELPER_H
