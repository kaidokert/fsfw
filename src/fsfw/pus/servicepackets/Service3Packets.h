#ifndef FSFW_PUS_SERVICEPACKETS_SERVICE3PACKETS_H_
#define FSFW_PUS_SERVICEPACKETS_SERVICE3PACKETS_H_

#include <fsfw/housekeeping/HousekeepingMessage.h>

#include <cstdint>

/**
 * @brief Subservices 25 and 26: TM packets
 * @ingroup spacepackets
 */
class HkPacket {  //!< [EXPORT] : [SUBSERVICE] 25, 26
 public:
  sid_t sid;              //!< [EXPORT] : [COMMENT] Structure ID (SID) of housekeeping data.
  const uint8_t* hkData;  //!< [EXPORT] : [MAXSIZE] Deduced size
  size_t hkSize;          //!< [EXPORT] : [IGNORE]

  HkPacket(sid_t sid, const uint8_t* data, size_t size) : sid(sid), hkData(data), hkSize(size) {}
};

#endif /* FSFW_PUS_SERVICEPACKETS_SERVICE3PACKETS_H_ */
