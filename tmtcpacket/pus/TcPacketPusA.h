#ifndef FSFW_TMTCPACKET_PUS_TCPACKETPUSA_H_
#define FSFW_TMTCPACKET_PUS_TCPACKETPUSA_H_

#include "ccsds_header.h"
#include "TcPacketBase.h"

#include <cstdint>

/**
 * This struct defines a byte-wise structured PUS TC A Data Field Header.
 * Any optional fields in the header must be added or removed here.
 * Currently, the Source Id field is present with one byte.
 * @ingroup tmtcpackets
 */
struct PUSTcDataFieldHeader {
    uint8_t version_type_ack;
    uint8_t service_type;
    uint8_t service_subtype;
    uint8_t source_id;
};

/**
 * This struct defines the data structure of a PUS Telecommand A packet when
 * accessed via a pointer.
 * @ingroup tmtcpackets
 */
struct TcPacketPointer {
    CCSDSPrimaryHeader primary;
    PUSTcDataFieldHeader dataField;
    uint8_t appData;
};


class TcPacketPusA: public TcPacketBase {
public:
private:
};


#endif /* FSFW_TMTCPACKET_PUS_TCPACKETPUSA_H_ */
