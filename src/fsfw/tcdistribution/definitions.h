#ifndef FSFW_TMTCPACKET_DEFINITIONS_H
#define FSFW_TMTCPACKET_DEFINITIONS_H

#include <cstdint>

#include "fsfw/returnvalues/FwClassIds.h"
#include "fsfw/returnvalues/returnvalue.h"

namespace tcdistrib {
static const uint8_t INTERFACE_ID = CLASS_ID::PACKET_CHECK;
static constexpr ReturnValue_t INVALID_CCSDS_VERSION = MAKE_RETURN_CODE(0);
static constexpr ReturnValue_t INVALID_APID = MAKE_RETURN_CODE(1);
static constexpr ReturnValue_t INVALID_PACKET_TYPE = MAKE_RETURN_CODE(2);
static constexpr ReturnValue_t INVALID_SEC_HEADER_FIELD = MAKE_RETURN_CODE(3);
static constexpr ReturnValue_t INCORRECT_PRIMARY_HEADER = MAKE_RETURN_CODE(4);

static constexpr ReturnValue_t INCOMPLETE_PACKET = MAKE_RETURN_CODE(7);
static constexpr ReturnValue_t INVALID_PUS_VERSION = MAKE_RETURN_CODE(8);
static constexpr ReturnValue_t INCORRECT_CHECKSUM = MAKE_RETURN_CODE(9);
static constexpr ReturnValue_t ILLEGAL_PACKET_SUBTYPE = MAKE_RETURN_CODE(10);
static constexpr ReturnValue_t INCORRECT_SECONDARY_HEADER = MAKE_RETURN_CODE(11);

};      // namespace tcdistrib
#endif  // FSFW_TMTCPACKET_DEFINITIONS_H
