#ifndef FSFW_SRC_FSFW_TMTCPACKET_PUS_TM_DEFINITIONS_H_
#define FSFW_SRC_FSFW_TMTCPACKET_PUS_TM_DEFINITIONS_H_

#include <cstdint>
#include <utility>

#include "fsfw/serialize/SerializeIF.h"

namespace ecss {

using PusChecksumT = uint16_t;

//! Version numbers according to ECSS-E-ST-70-41C p.439
enum PusVersion : uint8_t { PUS_A = 1, PUS_C = 2 };

/**
 * This struct defines the data structure of a Space Packet when accessed
 * via a pointer.
 * @ingroup tmtcpackets
 */
struct PusPointers {
  const uint8_t* spHeaderStart;
  const uint8_t* secHeaderStart;
  const uint8_t* userDataStart;
  const uint8_t* crcStart;
};

}  // namespace ecss

#endif /* FSFW_SRC_FSFW_TMTCPACKET_PUS_TM_DEFINITIONS_H_ */
